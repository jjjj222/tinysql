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
#include "wrapper.h"
#include "tiny_util.h"

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
string dump_field_type_str(const FIELD_TYPE& type)
{
    if (type == INT) {
        return "INT";
    }

    assert(type == STR20);
    return "STR20";
}

string dump_tiny_type_str(const DataType& type)
{
    switch (type) {
        case TINY_INT:
            return "INT";
        case TINY_STR20:
            return "STR20";
        case TINY_UNKNOWN:
            return "UNKNOWN";
    }

    return "UNKNOWN";
}

DataType field_to_tiny_type(const FIELD_TYPE& type)
{
    if (type == INT) {
        return TINY_INT;
    }

    assert(type == STR20);
    return TINY_STR20;
}

FIELD_TYPE tiny_to_field_type(const DataType& type)
{
    if (type == TINY_INT) {
        return INT;
    } else {
        assert(type == TINY_STR20);
        return STR20;
    }
}

//------------------------------------------------------------------------------
//   TinySchema
//------------------------------------------------------------------------------

TinySchema::TinySchema(const vector<pair<string, DataType>>& attribute_type_list)
: _schema(NULL)
{
    vector<string> field_names;
    vector<FIELD_TYPE> field_types;
    for (const auto& name_type : attribute_type_list) {
        //dump_normal(name_type);
        field_names.push_back(name_type.first);
        field_types.push_back(tiny_to_field_type(name_type.second));
    }

    _schema = new Schema(field_names, field_types);
}

TinySchema::TinySchema(const Schema& s)
: _schema(NULL)
{
    _schema = new Schema(s);
}

TinySchema::TinySchema(const TinySchema& rhs)
: _schema(NULL)
{
    assign(rhs);    
}

TinySchema::~TinySchema()
{
    assert(_schema != NULL);

    delete _schema;
}

DataType TinySchema::get_data_type(const string& field_name) const
{
    vector<pair<string, FIELD_TYPE>> name_type_list = get_name_type_list();

    for (const auto& name_type : name_type_list) {
        const string& name = name_type.first;
        if (name == field_name)
            return field_to_tiny_type(name_type.second);
    }
    return TINY_UNKNOWN;
}

vector<string> TinySchema::get_attr_list() const
{
    return _schema->getFieldNames();
}

vector<pair<string, FIELD_TYPE>> TinySchema::get_name_type_list() const
{
    vector<string> names = _schema->getFieldNames();
    vector<FIELD_TYPE> types = _schema->getFieldTypes();
    assert(names.size() == types.size());

    //string tmp;
    //tmp += "(";
    vector<pair<string, FIELD_TYPE>> tmp;
    for (size_t i = 0; i < names.size(); ++i) {
        tmp.push_back(make_pair(names[i], types[i]));
    }
    //    if (i != 0) {
    //        tmp += ", ";
    //    }
    //    tmp += names[i];
    //    tmp += " ";
    //    tmp += dump_field_type_str(types[i]);
    //}
    //tmp += ")";


    return tmp;
}

size_t TinySchema::size() const
{
    return (size_t)_schema->getNumOfFields();
}

size_t TinySchema::tuple_per_block() const
{
    return (size_t)_schema->getTuplesPerBlock();
}

//bool TinySchema::is_field_name_exist(const string& column) const
//{
//    vector<string> attr_list = get_attr_list();
//    size_t count = 0;
//    for (const auto& attr : attr_list) {
//        pair<string, string> table_name = get_column_name_value(attr);
//        
//    }
//    return false;
//}
bool TinySchema::is_field_name_exist(const string& name) const
{
    vector<string> attr_list = get_attr_list();
    bool res = is_contain(attr_list, name);
    return res;
}

//size_t TinySchema::count_field_name(const string& table, const string& column) const
//{
//    vector<string> attr_list = get_attr_list();
//    for (const auto& attr : attr_list) {
//        pair<string, string> table_name = get_column_name_value(attr);
//        //dump_normal(table_name);        
//    }
//    
//    
//    //bool res = is_contain(attr_list, table_name);
//    //return res;
//    return 0;
//}

void TinySchema::assign(const TinySchema& rhs)
{
    delete_not_null(_schema);
    _schema = new Schema( *(rhs._schema) );
}

bool TinySchema::is_equal_to(const TinySchema& rhs) const
{
    return *_schema == *(rhs._schema);
}

void TinySchema::dump() const
{
    cout << dump_str() << endl;
    //cout << schema << endl;
    
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
}

string TinySchema::dump_str() const
{
    vector<string> names = _schema->getFieldNames();
    vector<FIELD_TYPE> types = _schema->getFieldTypes();
    assert(names.size() == types.size());

    string tmp;
    tmp += "(";
    for (size_t i = 0; i < names.size(); ++i) {
        if (i != 0) {
            tmp += ", ";
        }
        tmp += names[i];
        tmp += " ";
        tmp += dump_field_type_str(types[i]);
    }
    tmp += ")";

    return tmp;
}

//------------------------------------------------------------------------------
//   TinyTuple
//------------------------------------------------------------------------------
TinyTuple::TinyTuple(const Tuple& tuple)
: _tuple(NULL)
{
    _tuple = new Tuple(tuple);
}

TinyTuple::~TinyTuple()
{
    assert(_tuple != NULL);
    delete _tuple;
}

bool TinyTuple::set_value(const string& name, const string& raw_value)
{
    assert(raw_value != "NULL"); // NULL has been removed from spec

    bool res = false;
    if (raw_value[0] == '\"') {
        //res = set_str_value(name, raw_value.substr(1, raw_value.size() - 2));
        res = set_str_value(name, get_literal_value(raw_value));
    } else {
        res = set_int_value(name, str_to<int>(raw_value));
    }

    return res;
}

bool TinyTuple::set_str_value(const string& name, const string& value)
{
    bool res = _tuple->setField(name, value);
    return res;
}

bool TinyTuple::set_int_value(const string& name, int value)
{
    bool res = _tuple->setField(name, value);
    return res;
}

void TinyTuple::init()
{
    vector<pair<string, FIELD_TYPE>> name_type_list = get_tiny_schema().get_name_type_list();

    for (const auto& name_type : name_type_list) {
        const string& name = name_type.first;
        const FIELD_TYPE& type = name_type.second;
        bool res = false;
        if (type == INT) {
            res = _tuple->setField(name, 0);
        } else {
            assert(type == STR20);
            res = _tuple->setField(name, "");
        }
        assert(res);
  //union Field getField(int offset) const; // returns INT_MIN if out of bound
  //union Field getField(string field_name) const; // returns INT_MIN if the name is not found
  //bool setField(string field_name, int i); // returns false if the type is wrong or the name is not found

    }
}

TinySchema TinyTuple::get_tiny_schema() const
{
    return TinySchema(_tuple->getSchema());
}

DataValue TinyTuple::get_value(const string& column_name) const
{
    //pair<string, string> get_
    ColumnName cn(column_name);

    vector<pair<string, FIELD_TYPE>> name_type_list = get_tiny_schema().get_name_type_list();

    for (const auto& name_type : name_type_list) {
        const auto& name = name_type.first;
        const auto& type = name_type.second;

        if (name == cn.get_column() || name == cn.get_column_name()) {
            Field value = _tuple->getField(name);
            if (type == INT) {
                return DataValue(value.integer);
            } else {
                assert(type == STR20);

                return DataValue(*(value.str));
            }
        }
    }

    return DataValue();
}

//string TinyTuple::get_value_str(const string& name) const
//{
//    //DataType data_type = get_tiny_schema().get_data_type(name);
//    //Field value = _tuple->getField(name);
//
//    //if (type == TINY_INT) {
//    //    return jjjj222::dump_str(value.integer);
//    //} else {
//    //    assert(type == TINY_STR20);
//    //    return *(value.str);
//    //}
//
//    return "<ERROR>";
//}

vector<string> TinyTuple::get_attr_list() const
{
    return get_tiny_schema().get_attr_list();
}

vector<string> TinyTuple::str_list() const
{
    vector<pair<string, FIELD_TYPE>> name_type_list = get_tiny_schema().get_name_type_list();
    vector<string> str_list;
    for (size_t i = 0; i < name_type_list.size(); ++i) {
        const auto& name = name_type_list[i].first;
        const auto& type = name_type_list[i].second;
        Field value = _tuple->getField(name);
        if (type == INT) {
            str_list.push_back(jjjj222::dump_str(value.integer));
        } else {
            assert(type == STR20);
            str_list.push_back( *(value.str) );
        }
    }

    return str_list;
}

string TinyTuple::dump_str() const
{
    vector<pair<string, FIELD_TYPE>> name_type_list = get_tiny_schema().get_name_type_list();

    string tmp = "(";
    for (size_t i = 0; i < name_type_list.size(); ++i) {
        if (i != 0) {
            tmp += ", ";
        }
    //for (const auto& name_type : name_type_list) {
        const auto& name = name_type_list[i].first;
        const auto& type = name_type_list[i].second;
        Field value = _tuple->getField(name);
        if (type == INT) {
            //cout << name << ": " << value.integer << endl;
            tmp += name;    
            tmp += " ";
            tmp += jjjj222::dump_str(value.integer);
        } else {
            assert(type == STR20);

            tmp += name;    
            tmp += " ";
            tmp += *(value.str);
            //cout << name << ": \"" << *(value.str) << "\"" << endl;
        }
    }

    tmp += ")";
    return tmp;
}

void TinyTuple::dump() const
{
    vector<pair<string, FIELD_TYPE>> name_type_list = get_tiny_schema().get_name_type_list();
    //cout << *_tuple << endl;
    //dump_pretty(tmp);
    //get_tiny_schema().dump();

    for (const auto& name_type : name_type_list) {
        const auto& name = name_type.first;
        const auto& type = name_type.second;
        Field value = _tuple->getField(name);
        if (type == INT) {
            cout << name << ": " << value.integer << endl;
        } else {
            assert(type == STR20);
            cout << name << ": \"" << *(value.str) << "\"" << endl;
        }
    }
  //union Field getField(int offset) const; // returns INT_MIN if out of bound
}

//------------------------------------------------------------------------------
//   TinyRelation
//------------------------------------------------------------------------------

TinyRelation::TinyRelation(Relation* r)
: _relation(r)
//, _size(0)
{
    ;
}

void TinyRelation::push_back(const TinyTuple& tuple)
{
    //dump_pretty(tuple);
    size_t mem_index = 0;
    Block* block = HwMgr::ins()->get_mem_block(mem_index);
    if (next_is_new_block()) {
        block->clear();
        block->appendTuple(tuple);
        _relation->setBlock(_relation->getNumOfBlocks(), mem_index);
    } else {
        _relation->getBlock(_relation->getNumOfBlocks()-1, mem_index);
        block->appendTuple(tuple); // append the tuple
        _relation->setBlock(_relation->getNumOfBlocks()-1, mem_index);
    }
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

}

void TinyRelation::add_space(size_t block_idx, size_t tuple_idx)
{
    size_t space_idx = block_idx * tuple_per_block() + tuple_idx; 
    add_space(space_idx);
}

void TinyRelation::add_space(size_t space_idx)
{
    _space.push_back(space_idx);
    for (size_t i = _space.size() - 1; i > 0; --i) {
        assert(_space[i] != _space[i-1]);
        if (_space[i] < _space[i-1]) {
            swap(_space[i], _space[i-1]);
        }
    }
}

void TinyRelation::refresh_block_num()
{
    size_t total_size = size() + _space.size();
    for (size_t i = _space.size(); i-- > 0;) {
        if (_space[i] == total_size - 1) {
            total_size--;
            _space.pop_back();
        }
    }

    size_t new_block_size = total_size / tuple_per_block();

    if (total_size % tuple_per_block() != 0)
        new_block_size++;

    reduce_blocks_to(new_block_size);
}

bool TinyRelation::load_block_to_mem(size_t block_index, size_t mem_index) const
{
    return _relation->getBlock(block_index, mem_index);
}

bool TinyRelation::save_block_to_disk(size_t block_index, size_t mem_index) const
{
    return _relation->setBlock(block_index, mem_index);
}

bool TinyRelation::reduce_blocks_to(size_t block_num) const
{
    //dump_normal(block_num);
    return _relation->deleteBlocks(block_num);
}

TinyTuple TinyRelation::create_tuple() const
{
    TinyTuple t = _relation->createTuple();
    t.init();
    return t;
}

string TinyRelation::get_name() const
{
    return _relation->getRelationName();
}

TinySchema TinyRelation::get_tiny_schema() const
{
    return TinySchema(_relation->getSchema());
}

size_t TinyRelation::size() const
{
    return (size_t)_relation->getNumOfTuples();
}

size_t TinyRelation::get_num_of_attribute() const
{
    return get_tiny_schema().size();
}

vector<string> TinyRelation::get_attr_list() const
{
    return get_tiny_schema().get_attr_list();
}

size_t TinyRelation::get_num_of_block() const
{
    return (size_t)_relation->getNumOfBlocks();
}

size_t TinyRelation::tuple_per_block() const
{
    return get_tiny_schema().tuple_per_block();
}

bool TinyRelation::next_is_new_block() const
{
    return (size() + _space.size()) % tuple_per_block() == 0; 
}

void TinyRelation::dump() const
{
    cout << "name: " << get_name() << endl;
    cout << "schema: " << get_tiny_schema().dump_str() << endl;
    cout << "size: " << size() << endl; 
    cout << "num of block: " << get_num_of_block() << endl; 
    cout << "tuple per block: " << tuple_per_block() << endl; 
    cout << "_space" << jjjj222::dump_str(_space) << endl;

    size_t mem_index = 0;

    size_t num_of_block = get_num_of_block();
    for (size_t i = 0; i < num_of_block; ++i) {
        _relation->getBlock(i, mem_index);
        Block* block = HwMgr::ins()->get_mem_block(mem_index);
        vector<Tuple> tuples = block->getTuples();
        for (const auto& tuple : tuples) {
            dump_normal(TinyTuple(tuple));
        }
    }
    //DrawTable t(2);
    //t.add_row("name", get_name());
    //t.add_row("schema", get_tiny_schema().dump_str());
    //t.draw();
}

string TinyRelation::dump_str() const
{
    string tmp = get_name();
    tmp += " ";
    //tmp += dump_str(get_tiny_schema());
    tmp += get_tiny_schema().dump_str();
    //cout << "The table has name " << relation.getRelationName() << endl;
    //dump_normal(relation.getRelationName());
    //cout << "The table has schema:" << endl;
    //cout << relation.getSchema() << endl;
    //cout << "The table currently have " << relation.getNumOfBlocks() << " blocks" << endl;
    //cout << "The table currently have " << relation.getNumOfTuples() << " tuples" << endl << endl;
    //cout << relation << endl;
    return tmp;
}
