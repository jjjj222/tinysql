#include <iostream>
#include <iterator>
#include <cassert>

using namespace std;

#include "debug.h"
#include "util.h"
#include "obj_util.h"

using namespace jjjj222;

#include "Block.h"
#include "Config.h"
#include "Disk.h"
#include "Field.h"
#include "MainMemory.h"
#include "Relation.h"
#include "Schema.h"
#include "SchemaManager.h"
#include "Tuple.h"

#include "tiny_util.h"
#include "query.h"
#include "dbMgr.h"
#include "parser.h"
#include "wrapper.h"

//------------------------------------------------------------------------------
//   SqlParser
//------------------------------------------------------------------------------
SqlParser::~SqlParser()
{
    parser_reset();
}

tree_node_t* SqlParser::parse_string(const string& sql_string) const
{
    parser_reset();
    return parse_sql_string(sql_string.c_str());
}

bool SqlParser::is_error() const
{
    return parser_get_error() != 0;
}

void SqlParser::dump() const
{
    dump_query_list();
}

//------------------------------------------------------------------------------
//   HwMgr
//------------------------------------------------------------------------------
HwMgr* HwMgr::_ins = NULL;

HwMgr::HwMgr() 
: _disk_io(0)
, _disk_time(0)
, _mem(NULL)
, _disk(NULL)
, _schema_mgr(NULL)
{
    init();
}

HwMgr::~HwMgr()
{
    delete_not_null(_mem);
    delete_not_null(_disk);
    delete_not_null(_schema_mgr);
    delete_all(_relations);
}

HwMgr* HwMgr::ins()
{
    if (_ins == NULL)
        _ins = new HwMgr();

    return _ins;
}

void HwMgr::init()
{
    _mem = new MainMemory();
    _disk = new Disk();
    _schema_mgr = new SchemaManager(_mem, _disk);

    _disk->resetDiskIOs();
    _disk->resetDiskTimer();
}

TinyRelation* HwMgr::get_tiny_relation(const string& name) const
{
    TinyRelation* res = NULL;
    for (const auto& r : _relations) {
        if (name == r->get_name()) {
            res = r;
            break;
        }
    }
    
    assert(res == NULL || res->get_relation() == get_relation(name));

    return res;
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------

TinyRelation* HwMgr::create_relation(const string& name, const TinySchema& schema)
{
    assert(!name.empty());
    //assert(!is_table_exist(name));
    assert(get_tiny_relation(name) == NULL);

    Relation* relation_ptr = _schema_mgr->createRelation(name, schema);
    assert(relation_ptr != NULL);

    TinyRelation* tiny_relation = new TinyRelation(relation_ptr);
    add_into(_relations, tiny_relation);

    return tiny_relation;
}

TinyBlock HwMgr::get_mem_block(size_t i)
{
    return _mem->getBlock(i);
}

size_t HwMgr::get_mem_size() const
{
    return NUM_OF_BLOCKS_IN_MEMORY;
}

size_t HwMgr::get_block_size() const
{
    return FIELDS_PER_BLOCK;
}

MemRange HwMgr::get_mem_range() const
{
    return MemRange(0, get_mem_size());
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
TinyRelation* HwMgr::create_tmp_relation(TinyRelation* r1, TinyRelation* r2) // TODO: const?
{
    assert(r1 != NULL);
    assert(r2 != NULL);

    vector<pair<string, DataType>> attr_type_list;
    add_into(attr_type_list, r1->get_attr_type_list_with_name());
    add_into(attr_type_list, r2->get_attr_type_list_with_name());
    //if (r1->is_with_prefix()) {
    //    add_into(attr_type_list, r1->get_attr_type_list());
    //} else {
    //    add_into(attr_type_list, r1->get_attr_type_list_with_name());
    //}

    //if (r2->is_with_prefix()) {
    //    add_into(attr_type_list, r2->get_attr_type_list());
    //} else {
    //    add_into(attr_type_list, r2->get_attr_type_list_with_name());
    //}

    string name;
    name += r1->get_name();
    name += " + ";
    name += r2->get_name();

    TinyRelation* tiny_relation = create_relation(name, attr_type_list);
    return tiny_relation;
}

bool HwMgr::create_table(const string& name, const vector<pair<string, DataType>>& attribute_type_list)
{
    //if (is_table_exist(name)) {
    if (get_tiny_relation(name) != NULL) {
        error_msg("Table '" + name + "' already exists");
        return false;
    }

    create_relation(name, attribute_type_list);

    return true;
}

bool HwMgr::drop_table(const string& name)
{
    const TinyRelation* to_delete = NULL;
    size_t pos = 0;
    for (size_t i = 0; i < _relations.size(); ++i) {
        const TinyRelation* r_ptr = _relations[i];
        if (r_ptr->get_name() == name) {
            to_delete = r_ptr;
            pos = i;
        }
    }

    if (to_delete == NULL) {
        assert(!_schema_mgr->relationExists(name));

        error_msg("Unknown table '" + name + "'");
        return false;
    }

    assert(to_delete->get_relation() == get_relation(name));

    delete_pos(_relations, pos);

    bool res = _schema_mgr->deleteRelation(name);
    assert(res == true);

    return res;
}

bool HwMgr::insert_into(const string& name, const vector<pair<string, string>>& data)
{
    TinyRelation* relation = get_tiny_relation(name);
    if (relation == NULL) {
        error_msg_table_not_exist(name);
        return false;
    }
    
    TinyTuple tuple = relation->create_tuple();
    
    for (const auto& name_value : data) { 
        const auto& name = name_value.first;
        const auto& value = name_value.second;
        if (!tuple.set_raw_value(name, value)) {
            return false;
        }
    }

    relation->push_back(tuple);
    return true;
}

bool HwMgr::delete_from(const string& name, tree_node_t* where_node)
{
    TinyRelation* relation = get_tiny_relation(name);
    if (relation == NULL) {
        error_msg_table_not_exist(name);
        return false;
    }

    ConditionMgr cond_mgr(where_node, relation);
    if (cond_mgr.is_error())
        return false;

    size_t mem_index = 0;
    size_t num_of_block = relation->get_num_of_block();
    //size_t tuple_count = 0;
    for (size_t i = 0; i < num_of_block; ++i) {
        size_t disk_index = i;
        relation->load_block_to_mem(disk_index, mem_index);
        TinyBlock tiny_block = HwMgr::ins()->get_mem_block(mem_index);
        //Block* block = HwMgr::ins()->get_mem_block(mem_index);
        //TinyBlock tiny_block(block);

        //vector<Tuple> tuples = block->getTuples();
        vector<Tuple> tuples = tiny_block.get_tuples();
        bool is_delete = false;
        for (size_t j = 0; j < tuples.size(); ++j) {
            const Tuple& tuple = tuples[j];

            if (!tuple.isNull()) {
                //tuple_count++;
                if (cond_mgr.is_tuple_match(tuple)) {
                    is_delete = true;
                    //block->nullTuple(j);
                    tiny_block.null_tuple(j);
                    relation->add_space(i, j);
                }
            }
        }

        if (is_delete) {
            relation->save_block_to_disk(disk_index, mem_index);
        }
    }

    relation->refresh_block_num();

    return true;
}

//bool HwMgr::is_table_exist(const string& name) const
//{
//    for (const auto& r : _relations) {
//        if (r->get_name() == name) {
//            return true;
//        }
//    }
//    return false;
//}

Relation* HwMgr::get_relation(const string& name) const
{
    return _schema_mgr->getRelation(name);
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
size_t HwMgr::get_elapse_io()
{
    size_t current_io = _disk->getDiskIOs();
    size_t res = current_io - _disk_io;
    _disk_io = current_io;
    return res;
}

double HwMgr::get_elapse_time()
{
    double current_time = _disk->getDiskTimer();
    double res = current_time - _disk_time;
    _disk_time = current_time;
    return res;
}

void HwMgr::print_tables()
{
    DrawTable table(1, DrawTable::MYSQL_TABLE);
    //table.set_align_right(i);
    //vector<DataType> type_list = get_type_list();
    //for (size_t i = 0; i < type_list.size(); ++i) {
    //    if (type_list[i] == TINY_INT) {
    //        table.set_align_right(i);
    //    }
    //}

    vector<string> header;
    header.push_back("Tables_in_tinysql");
    table.set_header(header);

    for (const auto& relation_ptr : _relations) {
        vector<string> row;
        row.push_back(relation_ptr->get_name());
        table.add_row(row);
    }
    
    if (table.size() == 0) {
        cout << "Empty set";
    } else {
        table.draw();
        cout << table.size() << " ";
        cout << ((table.size() == 1) ? "row" : "rows");
        cout << " in set";
    }

    //cout << " (" << get_elapse_io() << " disk I/O)"<< endl;
    cout << " (" << get_elapse_io() << " disk I/O), ";
    cout << get_elapse_time() << " ms)"<< endl;
}

void HwMgr::print_time() const
{
    cout << "Calculated elapse time = " << _disk->getDiskTimer() << " ms" << endl;
    cout << "Calculated Disk I/Os = " << _disk->getDiskIOs() << endl;
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
void HwMgr::dump()
{
    dump_relations();
    dump_memory();
}

void HwMgr::dump_memory() const
{
    DrawTable table(get_block_size() + 1);

    vector<string> header;
    header.push_back("\\");
    for (size_t i = 0; i < get_block_size(); ++i) {
        header.push_back("(" + tiny_dump_str(i) + ")");
    }
    table.set_header(header);

    for (size_t i = 0; i < get_mem_size(); ++i) {

        TinyBlock b = _mem->getBlock(i);
        vector<string> row;
        row.push_back("(" + tiny_dump_str(i) + ")");
        add_into(row, b.dump_str_list());
        table.add_row(row);
    }
    table.draw();
}

void HwMgr::dump_relations() const
{
    dump_print(_relations);
}

void HwMgr::dump_relation(const string& name) const
{
    TinyRelation* relation = get_tiny_relation(name);
    if (relation == NULL) {
        error_msg_table_not_exist(name);
        return;
    }

    relation->dump();
}

void HwMgr::dump_io()
{
    cout << "io: " << get_elapse_io() << endl;
        //size_t get_elapse_io();
}

