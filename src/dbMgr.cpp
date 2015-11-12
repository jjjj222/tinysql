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

#include "dbMgr.h"
#include "parser.h"
#include "wrapper.h"


//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
//FIELD_TYPE tiny_to_field_type(const DataType type)
//{
//    if (type == TINY_INT) {
//        return INT;
//    } else {
//        assert(type == TINY_STR20);
//        return STR20;
//    }
//}

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
    for (const auto& r : _relations) {
        if (name == r->get_name())
            return r;
    }
    return NULL;
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------

Relation* HwMgr::create_relation(const string& name, const Schema& schema)
{
    //cout << "Creating table " << name << endl;  
    Relation* relation_ptr = _schema_mgr->createRelation(name,schema);

    // Print the information about the Relation
    return relation_ptr;
}

Tuple HwMgr::create_tuple(const Relation& relation)
{
    return relation.createTuple();
}

Block* HwMgr::get_mem_block(size_t i)
{
    return _mem->getBlock(i);
}

vector<Tuple> HwMgr::get_block_tuple(const Block& block) const
{
    return block.getTuples();
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
bool HwMgr::create_table(const string& name, const vector<pair<string, DataType>>& attribute_type_list)
{
    for (const auto& r : _relations) {
        if (r->get_name() == name) {
            error_msg("Table '" + name + "' already exists");
            return false;
        }
    }

    TinySchema schema_wrapper(attribute_type_list);

    //string relation_name="ExampleTable1";
    //Relation* relation = create_relation(name, schema);
    Relation* relation = create_relation(name, schema_wrapper);
    TinyRelation* tiny_relation = new TinyRelation(relation);
    add_into(_relations, tiny_relation);
    //dump_relation(*relation);

    return true;
}

bool HwMgr::insert_into(const string& name, const vector<pair<string, string>>& data)
{
    //dump_pretty(name);
    //dump_pretty(data);
    TinyRelation* relation = get_tiny_relation(name);
    if (relation == NULL) {
        error_msg_table_not_exist(name);
        return false;
    }
    
    TinyTuple tuple = relation->create_tuple();
    
    //tuple.dump();

    for (const auto& name_value : data) { 
        const auto& name = name_value.first;
        const auto& value = name_value.second;
        if (!tuple.set_value(name, value)) {
            return false;
        }
    }

    //dump_pretty(tuple);

    relation->push_back(tuple);
    //relation->push_back(tuple);
    //Block* block = HwMgr::ins()->get_mem_block(0);
    //if (relation->next_is_new_block()) {
    //    block->clear();
    //    block->appendTuple(tuple);
    //} else {

    //}
    //relation_ptr->setBlock(relation_ptr->getNumOfBlocks(),memory_block_index);
//void appendTupleToRelation(Relation* relation_ptr, MainMemory& mem, int memory_block_index, Tuple& tuple) {
//  Block* block_ptr;
//  if (relation_ptr->getNumOfBlocks()==0) {
//    cout << "The relation is empty" << endl;
//    cout << "Get the handle to the memory block " << memory_block_index << " and clear it" << endl;
//    block_ptr=mem.getBlock(memory_block_index);
//    block_ptr->clear(); //clear the block
//    block_ptr->appendTuple(tuple); // append the tuple
//    cout << "Write to the first block of the relation" << endl;
//    relation_ptr->setBlock(relation_ptr->getNumOfBlocks(),memory_block_index);
//  } else {
//    cout << "Read the last block of the relation into memory block 5:" << endl;
//    relation_ptr->getBlock(relation_ptr->getNumOfBlocks()-1,memory_block_index);
//    block_ptr=mem.getBlock(memory_block_index);
//
//    if (block_ptr->isFull()) {
//      cout << "(The block is full: Clear the memory block and append the tuple)" << endl;
//      block_ptr->clear(); //clear the block
//      block_ptr->appendTuple(tuple); // append the tuple
//      cout << "Write to a new block at the end of the relation" << endl;
//      relation_ptr->setBlock(relation_ptr->getNumOfBlocks(),memory_block_index); //write back to the relation
//    } else {
//      cout << "(The block is not full: Append it directly)" << endl;
//      block_ptr->appendTuple(tuple); // append the tuple
//      cout << "Write to the last block of the relation" << endl;
//      relation_ptr->setBlock(relation_ptr->getNumOfBlocks()-1,memory_block_index); //write back to the relation
//    }
//  }  
//}

    return true;
}
//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
void HwMgr::dump()
{
    dump_memory();

    vector<string> field_names;
    vector<FIELD_TYPE> field_types;
    field_names.push_back("f1");
    field_names.push_back("f2");
    field_names.push_back("f3");
    field_names.push_back("f4");
    field_types.push_back(STR20);
    //field_types.push_back(STR20);
    field_types.push_back(INT);
    field_types.push_back(INT);
    field_types.push_back(STR20);
    Schema schema(field_names,field_types);
    //dump_schema(schema);

    string relation_name="ExampleTable1";
    Relation* relation = create_relation(relation_name, schema);
    //dump_relation(*relation);

    Tuple tuple = create_tuple(*relation);
    tuple.setField(0,"v11");
    tuple.setField(1,21);
    tuple.setField(2,31);
    tuple.setField(3,"v41");
    // Another way of setting the tuples
    //tuple.setField("f1","v11");
    //tuple.setField("f2",21);
    //tuple.setField("f3",31);
    //tuple.setField("f4","v41");
    //dump_tuple(tuple);

    // Set up a block in the memory
    //cout << "Clear the memory block 0" << endl;
    //Block* block_ptr = _mem->getBlock(0); //access to memory block 0
    Block* block_ptr = get_mem_block(0); 
    block_ptr->clear(); //clear the block

    // A block stores at most 2 tuples in this case
    // -----------first tuple-----------
    //cout << "Set the tuple at offset 0 of the memory block 0" << endl;
    block_ptr->setTuple(0,tuple); // You can also use appendTuple()
    //dump_block(*block_ptr);
    //dump_memory();
    //dump_relation(*relation);
}

void HwMgr::dump_memory() const
{
    //cout << NUM_OF_BLOCKS_IN_MEMORY << endl;
    cout << *_mem << endl;

    //for (int i = 0; i < NUM_OF_BLOCKS_IN_MEMORY; ++i) {
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
