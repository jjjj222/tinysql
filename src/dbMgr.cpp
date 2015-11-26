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
: _mem(NULL)
, _disk(NULL)
, _schema_mgr(NULL)
{
    init();
}

HwMgr::~HwMgr()
{
    //if (_mem != NULL) delete _mem;
    //if (_disk != NULL) delete _disk;
    //if (_schema_mgr != NULL) delete _schema_mgr;
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
        }
    }
    
    assert(res == NULL || res->get_relation() == get_relation(name));

    return res;
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------

//Relation* HwMgr::create_relation(const string& name, const Schema& schema)
//TinyRelation* HwMgr::create_relation(const string& name, const Schema& schema)
TinyRelation* HwMgr::create_relation(const string& name, const TinySchema& schema)
{
    assert(!name.empty());
    assert(!is_table_exist(name));

    Relation* relation_ptr = _schema_mgr->createRelation(name, schema);
    assert(relation_ptr != NULL);
    TinyRelation* tiny_relation = new TinyRelation(relation_ptr);
    add_into(_relations, tiny_relation);

    //return relation_ptr;
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

//vector<Tuple> HwMgr::get_block_tuple(const Block& block) const
//{
//    return block.getTuples();
//}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
TinyRelation* HwMgr::create_tmp_relation(TinyRelation* r1, TinyRelation* r2)
{
    assert(r1 != NULL);
    assert(r2 != NULL);

    vector<pair<string, DataType>> attr_type_list;
    bool with_prefix = false;
    if (is_contain(r1->get_attr_list(), r2->get_attr_list())) {
        add_into(attr_type_list, r1->get_attr_type_list_with_name());
        add_into(attr_type_list, r2->get_attr_type_list_with_name());
        with_prefix = true;
    } else {
        add_into(attr_type_list, r1->get_attr_type_list());
        add_into(attr_type_list, r2->get_attr_type_list());
    }


    string name;
    name += r1->get_name();
    name += " ";
    name += r2->get_name();
    //dump_pretty(attr_type_list);

    TinySchema schema_wrapper(attr_type_list);

    //string relation_name="ExampleTable1";
    //Relation* relation = create_relation(name, schema);
    TinyRelation* tiny_relation = create_relation(name, schema_wrapper);
    //Relation* relation = create_relation(name, schema_wrapper);
    //TinyRelation* tiny_relation = new TinyRelation(relation);
    if (with_prefix) {
        tiny_relation->set_with_prefix();
    }

    //add_into(_relations, tiny_relation);
    return tiny_relation;
}

//TinyRelation* HwMgr::create_tmp_relation(const string& name, TinyRelation* r)
//{
//    assert(r != NULL);
//
//    //vector<pair<string, DataType>> attr_type_list = r->get_attr_list();
//    //vector<pair<string, DataType>> attr_type_list = r->get_attr_list();
//
//    //TinySchema schema_wrapper(attr_type_list);
//    TinySchema schema_wrapper(r->get_attr_type_list());
//
//    TinyRelation* tiny_relation = create_relation(name, schema_wrapper);
//    //Relation* relation = create_relation(name, schema_wrapper);
//    //TinyRelation* tiny_relation = new TinyRelation(relation);
//
//    //add_into(_relations, tiny_relation);
//    return tiny_relation;
//}

bool HwMgr::create_table(const string& name, const vector<pair<string, DataType>>& attribute_type_list)
{
    if (is_table_exist(name)) {
        error_msg("Table '" + name + "' already exists");
        return false;
    }
    //for (const auto& r : _relations) {
    //    if (r->get_name() == name) {
    //        error_msg("Table '" + name + "' already exists");
    //        return false;
    //    }
    //}


    //string relation_name="ExampleTable1";
    //Relation* relation = create_relation(name, schema);
    //Relation* relation = create_relation(name, schema_wrapper);
    //TinyRelation* tiny_relation = new TinyRelation(relation);
    //add_into(_relations, tiny_relation);
    //dump_relation(*relation);
    //TinySchema schema_wrapper(attribute_type_list);
    //create_relation(name, schema_wrapper);
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
    //delete to_delete;
    //for (size_t i = pos+1; i < _relations.size(); ++i) {
    //    _relations[i-1] = _relations[i];
    //}
    //_relations.resize(_relations.size() - 1);

    bool res = _schema_mgr->deleteRelation(name);
    //bool res = delete_relation(name); 
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
    //cond_mgr.dump();

    size_t mem_index = 0;
    size_t num_of_block = relation->get_num_of_block();
    size_t tuple_count = 0;
    //size_t shift_idx = 0;
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
                tuple_count++;
                if (cond_mgr.is_tuple_match(tuple)) {
                    is_delete = true;
                    //block->nullTuple(j);
                    tiny_block.null_tuple(j);
                    relation->add_space(i, j);
                }
            }
        }

        if (is_delete) {
            //if (tiny_block.empty()) {
            //    shift_idx++;
            //    //dump_normal(shift_idx);
            //} else {
                //relation->save_block_to_disk(disk_index - shift_idx, mem_index);
                relation->save_block_to_disk(disk_index, mem_index);
            //}
        }
    }

    relation->refresh_block_num();

    return true;
}

//bool HwMgr::select_from(
//    const vector<string>& table_list, 
//    const vector<string>& attr_list,
//    tree_node_t* where_tree,
//    bool is_distinct
//)
//{
//    assert(!table_list.empty());
//
//    if (table_list.size() == 1) {
//        return select_from_single_table(table_list[0], attr_list, where_tree, is_distinct);
//    }
//    cout << "TODO: table_list.size() > 1" << endl;
//    //TinyRelation* relation = get_tiny_relation(name);
//    //if (relation == NULL) {
//    //    error_msg_table_not_exist(name);
//    //    return false;
//    //}
//
//    //dump_pretty(table_list);
//    //dump_pretty(attr_list);
//
//    return true;
//}
//
//bool HwMgr::select_from_single_table(
//    const string& table_name, 
//    const vector<string>& attr_list,
//    tree_node_t* where_tree,
//    bool is_distinct
//)
//{
//    TinyRelation* relation = get_tiny_relation(table_name);
//    if (relation == NULL) {
//        error_msg_table_not_exist(table_name);
//        return false;
//    }
//
//    if (where_tree != NULL) {
//        cout << "TODO: where != NULL" << endl;
//    }
//
//    if (!attr_list.empty()) {
//        cout << "TODO: !attr_list.empty()" << endl;
//    }
//
//    DrawTable table(relation->get_num_of_attribute(), DrawTable::MYSQL_TABLE);
//
//    size_t mem_index = 0;
//    size_t num_of_block = relation->get_num_of_block();
//    for (size_t i = 0; i < num_of_block; ++i) {
//        relation->load_block_to_mem(i, mem_index);
//        //_relation->getBlock(i, mem_index);
//        Block* block = HwMgr::ins()->get_mem_block(mem_index);
//        vector<Tuple> tuples = block->getTuples();
//        for (const auto& tuple : tuples) {
//            //dump_normal(TinyTuple(tuple));
//            table.add_row(TinyTuple(tuple).str_list());
//        }
//    }
//
//    table.set_header(relation->get_attr_list());
//    table.draw();
//    //cout << table.size() << " "
//
//    return true;
//}
bool HwMgr::is_table_exist(const string& name) const
{
    for (const auto& r : _relations) {
        if (r->get_name() == name) {
            return true;
        }
    }
    return false;
}

Relation* HwMgr::get_relation(const string& name) const
{
    return _schema_mgr->getRelation(name);
}

//bool HwMgr::delete_relation(const string& name) const
//{
//    return _schema_mgr->deleteRelation(name);
//}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
bool HwMgr::assert_relations() const
{
    //bool relationExists(string relation_name) const; //returns true if the relation exists
    
    //// returns a pointer to the newly allocated relation; the relation name must not exist already
    //Relation* createRelation(string relation_name,const Schema& schema);
    //Relation* getRelation(string relation_name); //returns NULL if the relation is not found
    //bool deleteRelation(string relation_name); //returns false if the relation is not found
    //for(const auto* tiny_relation_ptr : _relations) {

    //}
    return true;
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
void HwMgr::dump()
{
}

void HwMgr::dump_memory() const
{
    //cout << NUM_OF_BLOCKS_IN_MEMORY << endl;
    //cout << *_mem << endl;

    DrawTable table(get_block_size() + 1);

    vector<string> header;
    header.push_back("\\");
    for (size_t i = 0; i < get_block_size(); ++i) {
        header.push_back("(" + tiny_dump_str(i) + ")");
    }
    table.set_header(header);

    for (size_t i = 0; i < get_mem_size(); ++i) {

        TinyBlock b = _mem->getBlock(i);
        //dump_normal(b.dump_str_list());
        vector<string> row;
        row.push_back("(" + tiny_dump_str(i) + ")");
        add_into(row, b.dump_str_list());
        table.add_row(row);
        //cout << "QQ" << endl;
    //    b.dump();
    }
    table.draw();
    //    cout << "block " << i << ":" << endl;
    //    Block* b = _mem->getBlock(i);
    //    vector<Tuple> ts = b->getTuples();
    //    for (auto t : ts) {
    //        Schema s = t.getSchema();
    //        vector<FIELD_TYPE> fts = s.getFieldTypes();
    //        for (auto ft : fts) {
    //            if (ft == INT) {
    //                cout << "INT" << endl;
    //            } else {
    //                cout << "STR20" << endl;
    //            }
    //        }
    //    } 
    //}
}

void HwMgr::dump_relations() const
{
    //cout << * _schema_mgr;
    //dump_pretty(_relations);
    dump_print(_relations);
}
//void HwMgr::dump_schema(const Schema& schema) const
//{
//    cout << schema << endl;
//
//  //vector<string> getFieldNames() const; //returns the field names in defined order
//  //vector<enum FIELD_TYPE> getFieldTypes() const; //returns field types in defined order
//    //vector<string> field_names;
//    //vector<enum FIELD_TYPE> field_types;
//    //vector<FIELD_TYPE> field_types;
//
//    //cout << "The schema has " << schema.getNumOfFields() << " fields" << endl;
//    //cout << "The schema allows " << schema.getTuplesPerBlock() << " tuples per block" << endl;
//    //cout << "The schema has field names: " << endl;
//    //vector<string> field_names=schema.getFieldNames();
//    //copy(field_names.begin(),field_names.end(),ostream_iterator<string,char>(cout," "));
//    //cout << endl;
//    //cout << "The schema has field types: " << endl;
//    //vector<FIELD_TYPE> field_types=schema.getFieldTypes();
//    //for (int i=0;i<schema.getNumOfFields();i++) {
//    //  cout << (field_types[i]==0?"INT":"STR20") << "\t";
//    //}
//    //cout << endl;  
//    //cout << "The first field is of name " << schema.getFieldName(0) << endl;
//    //cout << "The second field is of type " << (schema.getFieldType(1)==0?"INT":"STR20") << endl;
//    //cout << "The field f3 is of type " << (schema.getFieldType("f3")==0?"INT":"STR20") << endl;
//    //cout << "The field f4 is at offset " << schema.getFieldOffset("f4") << endl << endl;
//}

//void HwMgr::dump_relation(const Relation& relation) const
void HwMgr::dump_relation(const string& name) const
{
    TinyRelation* relation = get_tiny_relation(name);
    if (relation == NULL) {
        error_msg_table_not_exist(name);
        return;
    }

    relation->dump();
    //dump_impl(relation);
    //dump_pretty(relation);
    //cout << "The table has name " << relation.getRelationName() << endl;
    //dump_normal(relation.getRelationName());
    //cout << "The table has schema:" << endl;
    //cout << relation.getSchema() << endl;
    //cout << "The table currently have " << relation.getNumOfBlocks() << " blocks" << endl;
    //cout << "The table currently have " << relation.getNumOfTuples() << " tuples" << endl << endl;
    //cout << relation << endl;
}

void HwMgr::dump_tuple(const Tuple& tuple) const
{
    cout << "Created a tuple " << tuple << " through the relation" << endl;
    cout << "The tuple is invalid? " << (tuple.isNull()?"TRUE":"FALSE") << endl;
    Schema tuple_schema = tuple.getSchema();
    cout << "The tuple has schema" << endl;
    cout << tuple_schema << endl;
    cout << "A block can allow at most " << tuple.getTuplesPerBlock() << " such tuples" << endl;
    
    cout << "The tuple has fields: " << endl;
    for (int i=0; i<tuple.getNumOfFields(); i++) {
      if (tuple_schema.getFieldType(i)==INT)
        cout << tuple.getField(i).integer << "\t";
      else
        cout << *(tuple.getField(i).str) << "\t";
    }
    cout << endl;

    cout << "The tuple has fields: " << endl;
    cout << *(tuple.getField("f1").str) << "\t";
    cout << tuple.getField("f2").integer << "\t";
    cout << tuple.getField("f3").integer << "\t";
    cout << *(tuple.getField("f4").str) << "\t";
    cout << endl << endl;
}

void HwMgr::dump_block(const Block& block) const
{
    //cout << "Now the memory block 0 contains:" << endl;
    cout << block << endl;

    //cout << "The block is full? " << (block.isFull()==1?"true":"false") << endl;
    //cout << "The block currently has " << block.getNumTuples() << " tuples" << endl;
    //cout << "The tuple at offset 0 of the block is:" << endl;
    //cout << block.getTuple(0) << endl << endl;
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
void error_msg_table_not_exist(const string& name)
{
    error_msg_not_exist("Table", name);
}
