#include <iostream>
#include <algorithm>
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
string dump_field_type_str(const FIELD_TYPE&);
string dump_tiny_type_str(const DataType&);
DataType field_to_data_type(const FIELD_TYPE&);
FIELD_TYPE tiny_to_field_type(const DataType&);

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

DataType field_to_data_type(const FIELD_TYPE& type)
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

TinySchema::TinySchema(TinySchema&& rhs)
: _schema(NULL)
{
    //cout << "move TinySchema" << endl;
    swap(_schema, rhs._schema);
    //delete_not_null(_schema);
    //_schema = rhs._schema;
    //rhs._schema = NULL;
}

TinySchema::~TinySchema()
{
    //assert(_schema != NULL);

    delete_not_null(_schema);
    //delete _schema;
}

DataType TinySchema::get_data_type(const string& field_name) const
{
    vector<pair<string, FIELD_TYPE>> name_type_list = get_name_type_list();

    for (const auto& name_type : name_type_list) {
        const string& name = name_type.first;
        if (name == field_name)
            return field_to_data_type(name_type.second);
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

    vector<pair<string, FIELD_TYPE>> tmp;
    for (size_t i = 0; i < names.size(); ++i) {
        tmp.push_back(make_pair(names[i], types[i]));
    }

    return tmp;
}

vector<pair<string, DataType>> TinySchema::get_attr_type_list() const
{
    vector<string> names = _schema->getFieldNames();
    vector<FIELD_TYPE> types = _schema->getFieldTypes();
    assert(names.size() == types.size());

    vector<pair<string, DataType>> res;
    for (size_t i = 0; i < names.size(); ++i) {
        res.push_back(make_pair(names[i], field_to_data_type(types[i])));
    }

    return res;
}

vector<DataType> TinySchema::get_type_list() const
{
    vector<FIELD_TYPE> types = _schema->getFieldTypes();
    vector<DataType> tmp;
    for (const auto& type : types) {
        tmp.push_back(field_to_data_type(type));
    }
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
//TinyTuple::TinyTuple(TinyRelation* r)
//: _tuple(NULL)
//{
//    _tuple = new Tuple(r->create_null_tuple());
//}

TinyTuple::TinyTuple(const Tuple& tuple)
: _tuple(NULL)
{
    _tuple = new Tuple(tuple);
}

TinyTuple::TinyTuple(const TinyTuple& rhs)
: _tuple(NULL)
{
    assign(rhs);
}

TinyTuple::TinyTuple(TinyTuple&& rhs)
: _tuple(NULL)
{
    //cout << "move TinyTuple" << endl;
    swap(_tuple, rhs._tuple);
    //delete_not_null(_tuple);
    //_tuple = rhs._tuple;
    //rhs._tuple == NULL;
}

TinyTuple::~TinyTuple()
{
    //assert(_tuple != NULL);
    delete_not_null(_tuple);
    //delete _tuple;
}

bool TinyTuple::set_raw_value(const string& name, const string& raw_value)
{
    //assert(raw_value != "NULL"); // NULL has been removed from spec
    if (raw_value == "NULL") {
        //error_msg
        error_msg("do not support 'NULL' now");
        return false;
    }

    DataType data_type = get_data_type(name);

    bool res = false;
    if (raw_value[0] == '\"') {
        //res = set_str_value(name, raw_value.substr(1, raw_value.size() - 2));
        res = set_str_value(name, get_literal_value(raw_value));
    } else {
        if (data_type != TINY_INT) {
            error_msg("\'" + name + "\' should be INT");
            return false;
        }
        res = set_int_value(name, str_to<int>(raw_value));
    }

    return res;
}

void TinyTuple::set_value(const string& name, const DataValue& data_value)
{
    if (data_value.get_type() == TINY_INT) {
        set_int_value(name, data_value.get_int());
    } else {
        assert(data_value.get_type() == TINY_STR20);
        set_str_value(name, data_value.get_str());
    }
}

void TinyTuple::set_value(size_t i, const DataValue& type_value)
{
    const DataType& type = type_value.get_type(); 
    if (type == TINY_INT) {
        _tuple->setField(i, type_value.get_int());
    } else {
        assert(type == TINY_STR20);

        _tuple->setField(i, type_value.get_str());
    }
}

void TinyTuple::set_value(const TinyTuple& t1, const TinyTuple& t2)
{
    assert(size() == t1.size() + t2.size()); 
    assert(!t1.is_null());
    assert(!t2.is_null());
    
    vector<DataValue> tmp;
    add_into(tmp, t1.get_value_list());
    add_into(tmp, t2.get_value_list());
    //dump_normal(v1);
     
    for (size_t i = 0; i < tmp.size(); ++i) {
        set_value(i, tmp[i]);
    }
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

void TinyTuple::set_null()
{
    _tuple->null();
}

TinySchema TinyTuple::get_tiny_schema() const
{
    return TinySchema(_tuple->getSchema());
}

size_t TinyTuple::size() const
{
    return get_tiny_schema().size();
}

DataType TinyTuple::get_data_type(const string& name) const
{
    return get_tiny_schema().get_data_type(name);
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

vector<DataValue> TinyTuple::get_value_list() const
{
    vector<DataType> type_list = get_tiny_schema().get_type_list();

    vector<DataValue> res;
    for (size_t i = 0; i < type_list.size(); ++i) {
        Field value = _tuple->getField(i);
        if (type_list[i] == TINY_INT) {

            res.push_back(DataValue(value.integer)); 
        } else {
            assert(type_list[i] == TINY_STR20);
    
            res.push_back(DataValue(*(value.str))); 
        }
    }

    return res;
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

bool TinyTuple::is_null() const
{   
    assert(_tuple != NULL);
    
    return _tuple->isNull();
}

void TinyTuple::assign(const TinyTuple& rhs)
{
    delete_not_null(_tuple);
    _tuple = new Tuple(rhs);
}

bool TinyTuple::is_less_than(const TinyTuple& rhs) const
{
    bool res = get_value_list() < rhs.get_value_list();;
    return res;
}

bool TinyTuple::is_equal_to(const TinyTuple& rhs) const
{
    bool res = get_value_list() == rhs.get_value_list();;
    return res;
}

bool TinyTuple::is_less_than_by_attr(const TinyTuple& rhs, const vector<string>& attr_list) const
{
    for (const string& attr : attr_list) {
        DataValue value = get_value(attr);
        DataValue rhs_value = rhs.get_value(attr);

        if (value < rhs_value) {
            return true;
        } else if (value > rhs_value) {
            return false;
        }
    }

    return is_less_than(rhs);
}

string TinyTuple::dump_str() const
{
    if (is_null())
        return "<DELETE>";

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
    cout << dump_str() << endl;
    //vector<pair<string, FIELD_TYPE>> name_type_list = get_tiny_schema().get_name_type_list();
    ////cout << *_tuple << endl;
    ////dump_pretty(tmp);
    ////get_tiny_schema().dump();

    //for (const auto& name_type : name_type_list) {
    //    const auto& name = name_type.first;
    //    const auto& type = name_type.second;
    //    Field value = _tuple->getField(name);
    //    if (type == INT) {
    //        cout << name << ": " << value.integer << endl;
    //    } else {
    //        assert(type == STR20);
    //        cout << name << ": \"" << *(value.str) << "\"" << endl;
    //    }
    //}
  //union Field getField(int offset) const; // returns INT_MIN if out of bound
}

vector<string> TinyTuple::dump_str_list() const
{
    return str_list();
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
TinyBlock::TinyBlock(Block* block)
: _block(block)
{
    ;
}

vector<Tuple> TinyBlock::get_tuples() const
{
    return _block->getTuples();
}

vector<TinyTuple> TinyBlock::get_tiny_tuples() const
{
    vector<TinyTuple> tmp;

    vector<Tuple> tuples = get_tuples();
    for (const auto& tuple : tuples) {
        tmp.push_back(tuple);
    }

    return tmp;
}

TinyTuple TinyBlock::get_tiny_tuple(size_t i) const
{
//    //assert(
    return _block->getTuple(i);
}

size_t TinyBlock::size()
{
    return get_tuples().size();
}

void TinyBlock::push_back(const TinyTuple& tuple)
{
    _block->appendTuple(tuple);
}

void TinyBlock::set_tuple(size_t i, const TinyTuple& tuple)
{
    _block->setTuple(i, tuple);
}

void TinyBlock::remove_null_tuple()
{
    vector<Tuple> tuples = get_tuples();
    _block->clear();
    for (const auto& tuple : tuples) {
        if (!tuple.isNull()) {
            push_back(tuple);
            //dump_normal(TinyTuple(tuple));
        }
    }
}

void TinyBlock::null_tuple(size_t i)
{
    _block->nullTuple(i);
}

void TinyBlock::clear()
{
    _block->clear();
}

bool TinyBlock::empty() const
{
    vector<Tuple> tuples = get_tuples();
    for (const auto& tuple : tuples) {
        if (!tuple.isNull())
            return false;
    }
    return true;
}

void TinyBlock::dump() const
{
    vector<TinyTuple> tuples = get_tiny_tuples();
    for (const auto& tuple : tuples) {
        tuple.dump();
    }
}

string TinyBlock::dump_str() const
{
    return "TODO";
}

vector<string> TinyBlock::dump_str_list() const
{
    size_t count = 0;
    vector<TinyTuple> tuples = get_tiny_tuples();

    vector<string> res;
    for (const auto& tuple : tuples) {
        if (tuple.is_null()) {
            size_t num = tuple.size();
            count += num;
            for (size_t i = 0; i < num; ++i) {
                res.push_back(""); 
            }
        } else {
            const vector<string> str_list = tuple.dump_str_list();
            count += str_list.size();
            add_into(res, str_list); 
        }
        //tuple.dump();
    }

    for (size_t i = count; i < HwMgr::ins()->get_block_size(); ++i) {
        res.push_back("");
    }
    return res;
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
//MemIter::MemIter(TinyRelation* r, size_t pos)
MemIter::MemIter(const TinyRelation* r, size_t pos)
: _relation(r)
, _pos(pos)
{
    ;
}

//void MemIter::clear_block()
//{
//    size_t block_idx = get_block_idx();
//    TinyBlock block = HwMgr::ins()->get_mem_block(block_idx);
//    block.clear();
//}

//void MemIter::push_back(const TinyTuple& tuple)
//{
//    size_t block_idx = get_block_idx();
//    TinyBlock block = HwMgr::ins()->get_mem_block(block_idx);
//    block.push_back(tuple);
//}
void MemIter::set_tuple(const TinyTuple& tuple)
{
    size_t block_idx = get_block_idx();
    size_t tuple_idx = get_tuple_idx();
    TinyBlock block = HwMgr::ins()->get_mem_block(block_idx);
    block.set_tuple(tuple_idx, tuple);
}

TinyTuple MemIter::get_tuple() const
{
    size_t block_idx = get_block_idx();
    size_t tuple_idx = get_tuple_idx();
    TinyBlock block = HwMgr::ins()->get_mem_block(block_idx);
    return block.get_tiny_tuple(tuple_idx);
}

size_t MemIter::get_block_idx() const
{
    return _pos / _relation->tuple_per_block();
}

size_t MemIter::get_tuple_idx() const
{
    return _relation->get_tuple_idx_by_pos(_pos);
}

bool MemIter::is_equal_to(const MemIter& rhs) const
{
    assert(_relation == rhs._relation);

    bool res = _pos == rhs._pos;
    return res;
}

string MemIter::dump_str() const
{
    //return tiny_dump_str(_pos);
    return tiny_dump_str(get_block_idx(), get_tuple_idx());
}
//------------------------------------------------------------------------------
//   RelIter
//------------------------------------------------------------------------------
//RelIter::RelIter(TinyRelation* r)
RelIter::RelIter(const TinyRelation* r)
: _relation(r)
//, _block(0)
, _pos(0)
//, _mem_idx(0)
{
    ;
}

//RelIter::RelIter(TinyRelation* r, size_t pos)
RelIter::RelIter(const TinyRelation* r, size_t pos)
: _relation(r)
, _pos(pos)
{
    ;
}
//void RelIter::inc()
//{
//    // TODO
//}
//
bool RelIter::is_equal_to(const RelIter& rhs) const
{
    assert(_relation == rhs._relation);
    //assert(_mem_idx == rhs._mem_idx);

    //bool res = _block == rhs._block && _pos == rhs._pos;
    bool res = _pos == rhs._pos;
    return res;
}

size_t RelIter::get_block_idx() const
{
    return _relation->get_block_idx_by_pos(_pos);
}

size_t RelIter::get_tuple_idx() const
{
    return _relation->get_tuple_idx_by_pos(_pos);
}

TinyTuple RelIter::load_to_mem(size_t mem_idx) const
{
    pair<size_t, size_t> block_tuple_idx = _relation->get_idx_by_pos(_pos);
    size_t block_idx = block_tuple_idx.first;
    size_t tuple_idx = block_tuple_idx.second;
    _relation->load_block_to_mem(block_idx, mem_idx);
    TinyBlock block = HwMgr::ins()->get_mem_block(mem_idx);
    return  block.get_tiny_tuple(tuple_idx);
}

TinyTuple RelIter::get_from_mem(size_t mem_idx) const
{
    pair<size_t, size_t> block_tuple_idx = _relation->get_idx_by_pos(_pos);
    size_t tuple_idx = block_tuple_idx.second;
    TinyBlock block = HwMgr::ins()->get_mem_block(mem_idx);
    return  block.get_tiny_tuple(tuple_idx);
}

void RelIter::skip_null()
{
    while (_relation->end() != *this && is_null()) {
        ++_pos;
    }
}

bool RelIter::is_null() const
{
    return _relation->is_null(_pos);
}

//bool RelIter::is_end() const
//{
//    return _relation->end() == *this;
//}

string RelIter::dump_str() const
{
    //return tiny_dump_str(_pos);
    return tiny_dump_str(get_block_idx(), get_tuple_idx());
}
//TinyTuple RelIter::get_tuple() const
//{
//    _relation->load_block_to_mem(_block, _mem_idx);
//    //TinyBlock block = HwMgr::ins()->get_tiny_block(_mem_idx);
//    TinyBlock block = HwMgr::ins()->get_mem_block(_mem_idx);
//    vector<TinyTuple> tuples = block.get_tiny_tuples();
//
//    if (_pos >= tuples.size()) {
//        return _relation->create_null_tuple();
//    }
//
//    return tuples[_pos];
//    //return block.get_tiny_tuple(_pos);
//
//            //_relation->getBlock(i, mem_index);
//            //Block* block = HwMgr::ins()->get_mem_block(mem_index);
//            //vector<Tuple> tuples = block->getTuples();
//            //for (const auto& tuple : tuples) {
//            //    //dump_normal(TinyTuple(tuple));
//            //    if (!tuple.isNull())
//            //        table.add_row(TinyTuple(tuple).str_list());
//            //}
//        //}
//}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
RelPartialScanner::RelPartialScanner(TinyRelation* r,
    size_t mem_idx, const RelIter& it, const RelIter& it_end)
: _relation(r)
, _mem_idx(mem_idx)
, _it(it)
, _it_end(it_end)
{
    ;
}


//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
//RelScanner::RelScanner(TinyRelation* r, size_t base_idx, size_t mem_size)
RelScanner::RelScanner(const TinyRelation* r, size_t base_idx, size_t mem_size)
: _relation(r)
, _base_idx(base_idx)
, _mem_size(mem_size)
, _m_iter(r, 0)
, _m_iter_end(r, 0)
, _iter(r->begin())
, _iter_end(r->end())
{
    assert(_relation != NULL);
    assert(_base_idx >= 0 && _base_idx < HwMgr::ins()->get_mem_size());
    assert(_mem_size >= 1);
    assert(_base_idx + _mem_size - 1 < HwMgr::ins()->get_mem_size());
}


TinyTuple RelScanner::get_next()
{
    if (_m_iter == _m_iter_end) {
        _iter.skip_null();
        //if (_iter.is_end()) {
        if (is_iter_end()) {
            return _relation->create_null_tuple();
        } else {
            load_to_mem();
        }
    }

    TinyTuple res = _m_iter.get_tuple();
    ++_m_iter;

    return res;
}

TinyTuple RelScanner::peep_next() // TODO
{
    if (_m_iter == _m_iter_end) {
        _iter.skip_null();
        //if (_iter.is_end()) {
        if (is_iter_end()) {
            return _relation->create_null_tuple();
        } else {
            load_to_mem();
        }
    }

    TinyTuple res = _m_iter.get_tuple();

    return res;
}

bool RelScanner::sort(const vector<string>& attr_list)
{
    //assert(!name.empty());

    vector<TinyTuple> tuple_list;
    for (MemIter it = _m_iter; it != _m_iter_end; ++it) {
        tuple_list.push_back(it.get_tuple());
    }

    //dump_pretty(tuple_list);

    //vector<string> search_name_list;
    //for (const string& attr : attr_list) {
    //    string search_name = _relation->get_attr_search_name(attr);
    //    search_name_list.push_back(search_name);
    //}
    vector<string> search_name_list = _relation->get_attr_search_name_list(attr_list);

    std::sort(tuple_list.begin(), tuple_list.end(),
        [&](const TinyTuple& a, const TinyTuple& b) -> bool
    { 
        //return a.is_less_than_by_attr(b, search_name);
        return a.is_less_than_by_attr(b, search_name_list);

        //DataValue a_value = a.get_value(search_name);
        //DataValue b_value = b.get_value(search_name);

        //if (a_value < b_value) {
        //    return true;
        //} else if (a_value > b_value) {
        //    return false;
        //}

        //return a < b;
    });

    size_t i = 0;
    for (MemIter it = _m_iter; it != _m_iter_end; ++it) {
        it.set_tuple(tuple_list[i]);
        ++i;
        //tuple_list.push_back(it.get_tuple());
    }
    //cout << endl;
    //dump_pretty(tuple_list);

    return true;
}

void RelScanner::clear_mem()
{
    for (size_t i = 0; i < _mem_size; ++i) {
        size_t mem_idx = _base_idx + i;
        clear_mem_block(mem_idx);
    }
}

void RelScanner::clear_mem_block(size_t i)
{
    assert(i >= _base_idx);
    assert(i < _base_idx + _mem_size);

    HwMgr::ins()->get_mem_block(i).clear();
}

//void RelScanner::move_to_non_null()
//{
//    while (!_iter.is_end() && _iter.is_null()) {
//        ++_iter;
//    }
//}

void RelScanner::load_to_mem()
{
    clear_mem();

    _m_iter = m_begin();
    bool reuse_block = false;
    size_t old_block_idx = 0;
    size_t load_idx = get_last_mem_block();
    for (_m_iter = m_begin(); _m_iter != m_load_end(); ++_m_iter) {
        //move_to_non_null();
        _iter.skip_null();
        //if (_iter.is_end()) {
        if (is_iter_end()) {
            break;
        }

        size_t block_idx = _iter.get_block_idx();
        //TinyTuple tuple(_relation);
        TinyTuple tuple = _relation->create_null_tuple();
        if (reuse_block && block_idx == old_block_idx) {
            tuple = _iter.get_from_mem(load_idx); 
        } else {
            reuse_block = true;
            old_block_idx = block_idx;
            tuple = _iter.load_to_mem(load_idx); 
        }
        ++_iter;

        _m_iter.set_tuple(tuple);
    }
    //dump_normal(_iter);

    //_m_iter_end = _m_iter;
    if (_m_iter == m_load_end()) {
        //move_to_non_null();
        _iter.skip_null();
        //if (!_iter.is_end()) {
        if (!is_iter_end()) {
            const size_t last_block_idx = _iter.get_block_idx();
            const size_t first_tuple_idx = _iter.get_tuple_idx();
            //while (!_iter.is_end() && _iter.get_block_idx() == last_block_idx) {
            //dump_normal(last_block_idx);
            _relation->load_block_to_mem(last_block_idx, load_idx);
            TinyBlock block = HwMgr::ins()->get_mem_block(load_idx);
            // remove front
            for (size_t i = 0; i < first_tuple_idx; ++i) {
                block.null_tuple(i);
            }

            while (!is_iter_end() && _iter.get_block_idx() == last_block_idx) {
                ++_iter;
            }

            // remove back
            if (_iter.get_block_idx() == last_block_idx) {
                const size_t end_tuple_idx = _iter.get_tuple_idx();
                for (size_t i = end_tuple_idx; i < block.size(); ++i) {
                    block.null_tuple(i);
                }
            }

            block.remove_null_tuple();
            //block.dump();
            for (size_t i = 0; i < block.size(); ++i) {
                ++_m_iter;
            }

            //dump_normal(_m_iter);
        }
    }

    _m_iter_end = _m_iter;
    _m_iter = m_begin();
}

void RelScanner::add_mem_into(TinyRelation& new_relation) const
{
    for (MemIter it = _m_iter; it != _m_iter_end; ++it) {
        new_relation.push_back(it.get_tuple());
    }
}
//TinyTuple RelScanner::get_from_mem()
//{
//}
bool RelScanner::is_iter_end() const
{
    bool res = _iter == _iter_end;
    return res;
}

bool RelScanner::is_end() const
{
    //bool res = _iter.is_end() && _m_iter == _m_iter_end;
    bool res = is_iter_end() && _m_iter == _m_iter_end;
    return res;
}

MemIter RelScanner::m_begin()
{
    size_t mem_pos = _base_idx * _relation->tuple_per_block();
    return MemIter(_relation, mem_pos);
}

MemIter RelScanner::m_load_end()
{
    size_t mem_pos = (_base_idx + _mem_size - 1) * _relation->tuple_per_block();
    return MemIter(_relation, mem_pos);
}

MemIter RelScanner::m_end()
{
    size_t mem_pos = (_base_idx + _mem_size) * _relation->tuple_per_block();
    return MemIter(_relation, mem_pos);
}

size_t RelScanner::get_last_mem_block() const
{
    return _base_idx + _mem_size - 1;
}

void RelScanner::dump() const
{
    dump_normal(_base_idx);
    dump_normal(_mem_size);
    dump_normal(_m_iter);
    dump_normal(_m_iter_end);
    dump_normal(_iter);
    dump_normal(_iter_end);
}


//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
RelSorter::RelSorter(TinyRelation* r, size_t base_idx, size_t mem_size)
: _relation(r)
, _base_idx(base_idx)
, _mem_size(mem_size)
//, _sorted_relation(NULL)
//, _scanner_list(NULL)
{
    assert(_relation != NULL);
    assert(_base_idx >= 0 && _base_idx < HwMgr::ins()->get_mem_size());
    assert(_mem_size >= 1);
    assert(_base_idx + _mem_size - 1 < HwMgr::ins()->get_mem_size());
}

RelSorter::~RelSorter()
{
    //delete_not_null(_scanner_list);
    //if (_sorted_relation != NULL) {
    //    HwMgr::ins()->drop_table(_sorted_relation->get_name());
    //}
}

void RelSorter::sort(const string& attr)
{
    assert(!attr.empty());

    _attr_list.clear();
    _attr_list.push_back(attr);    
    sort();
}

//bool RelSorter::sort()
void RelSorter::sort()
{
    //assert(_sub_list.empty());
    //assert(_sorted_relation == NULL);

    string new_table_name = "tmp " + _relation->get_name();
    TinyRelation* new_relation = HwMgr::ins()->create_relation(
        new_table_name, _relation->get_tiny_schema());
    
    //_relation->dump();
    //dump_normal(new_relation->end());
    //RelIter it = new_relation->end();
    RelScanner scanner(_relation, _base_idx, _mem_size);
    RelIter it = new_relation->end();
    vector<pair<RelIter, RelIter>> sub_list;
    while (!scanner.is_iter_end()) {
        scanner.load_to_mem();
        scanner.sort(_attr_list);
        scanner.add_mem_into(*new_relation);
        //scanner.dump();
        //cout << endl;

        RelIter it_end = new_relation->end();
        sub_list.push_back(make_pair(it, it_end));
        it = it_end;
    }
    //new_relation->dump();
    //dump_pretty(_sub_list);
    //scanner.load_to_mem();
    //scanner.sort(_attr);
    //scanner.dump();
    //HwMgr::ins()->dump_memory();
    //_relation->clear();
    //if (_mem_size < sub_list.size()) {
    //    cout << "error" << endl;
    //}

    while (_mem_size < sub_list.size()) {
        new_relation = reduce_sub_list(new_relation, sub_list);
    }

    //_relation->dump();
    //new_relation->dump();
    //dump_pretty(sub_list);
    
    //_sorted_relation = new_relation;

    //assert(_scanner_list == NULL);
    //if (_scanner_list == NULL) {
        //scanner_list = new vector<RelScanner>();




    vector<RelScanner> scanner_list = get_scanner_list(new_relation, sub_list);
    //vector<RelScanner> scanner_list;
    //for (size_t i = 0; i < sub_list.size(); ++i) {
    //    size_t mem_idx = _base_idx + i;
    //    const RelIter& it = sub_list[i].first;
    //    const RelIter& it_end = sub_list[i].second;

    //    //scanner_list.push_back( RelScanner(_sorted_relation, mem_idx, 1) );
    //    scanner_list.push_back( RelScanner(new_relation, mem_idx, 1) );
    //    scanner_list.back().set_begin(it);
    //    scanner_list.back().set_end(it_end);

    //    //_scanner_list->back().dump();
    //    //cout << endl;
    //}
    //}

    //_relation->dump();
    //_sorted_relation->dump();
    //TinyTuple tmp = get_max(*_sorted_relation, scanner_list);
    _relation->clear();
    TinyTuple tmp = get_max(scanner_list);
    while (!tmp.is_null()) {
        //dump_normal(tmp);
        _relation->push_back(tmp);
        //tmp = get_max(*_sorted_relation, scanner_list);
        tmp = get_max(scanner_list);
    }

    //_relation->dump();
    //_relation->clear();
    //_relation->dump();

    HwMgr::ins()->drop_table(new_relation->get_name());

    //return _relation->create_null_tuple();
    //return true;
}

TinyRelation* RelSorter::reduce_sub_list(TinyRelation* r, SubListType& sub_list)
{
    string new_table_name = "tmp " + r->get_name();
    TinyRelation* new_relation = HwMgr::ins()->create_relation(
        new_table_name, r->get_tiny_schema());

    vector<pair<RelIter, RelIter>> new_sub_list;

    //SubListType tmp_sub_list;
    //for (size_t i = 0; i < _mem_size(); ++i) {
    //    tmp_sub_list.push_back
    //}
    vector<SubListType> sub_list_split = vector_split(sub_list, _mem_size);
    //dump_pretty(sub_list_split);

    //for (size_t i = 0; i < sub_list_split.size(); ++i) {
    RelIter it = new_relation->end();
    for (const auto& tmp_sub_list : sub_list_split) {
        vector<RelScanner> scanner_list = get_scanner_list(r, tmp_sub_list);

        TinyTuple tmp = get_max(scanner_list);
        while (!tmp.is_null()) {
            new_relation->push_back(tmp);
            tmp = get_max(scanner_list);
        }

        RelIter it_end = new_relation->end();
        new_sub_list.push_back(make_pair(it, it_end));
        it = it_end;
    }
    //vector<RelScanner> scanner_list;
    //for (size_t i = 0; i < tmp_sub_list.size(); ++i) {
    //    size_t mem_idx = _base_idx + i;
    //    const RelIter& it = tmp_sub_list[i].first;
    //    const RelIter& it_end = tmp_sub_list[i].second;

    //    scanner_list.push_back( RelScanner(new_relation, mem_idx, 1) );
    //    scanner_list.back().set_begin(it);
    //    scanner_list.back().set_end(it_end);
    //}

    //TinyTuple tmp = get_max(scanner_list);
    //while (!tmp.is_null()) {
    //    relation->push_back(tmp);
    //    tmp = get_max(scanner_list);
    //}

    HwMgr::ins()->drop_table(r->get_name());
    sub_list = new_sub_list;
    return new_relation;
}

TinyTuple RelSorter::get_max(vector<RelScanner>& scanner_list)
{
    assert(scanner_list.size() <= _mem_size);
    //assert(_sorted_relation != NULL);
    //assert (_mem_size >= _sub_list.size());

    //vector<RelScanner> scanner_list;
    //vector<size_t> scanner
    //_sorted_relation->dump();
    //size_t max_pos = 0;
    //string search_name = _sorted_relation->get_attr_search_name(_attr);
    //TinyTuple res = _sorted_relation->create_null_tuple();
    //string search_name = r.get_attr_search_name(_attr);
    //TinyTuple res = r.create_null_tuple();
    vector<string> search_name_list = _relation->get_attr_search_name_list(_attr_list);
    //string search_name = _relation->get_attr_search_name(_attr);
    TinyTuple res = _relation->create_null_tuple();
    RelScanner* min_scanner = NULL;
    //for (auto& scanner : *_scanner_list) {
    for (auto& scanner : scanner_list) {
        TinyTuple tmp = scanner.peep_next();
        if (tmp.is_null())
            continue;

        if (res.is_null()) {
            res = tmp;
            min_scanner = &scanner;
        //} else if (tmp.is_less_than_by_attr(res, search_name)) {
        } else if (tmp.is_less_than_by_attr(res, search_name_list)) {
            res = tmp;
            min_scanner = &scanner;
        }
    }

    //dump_normal(res);

    if (min_scanner != NULL) {
        min_scanner->get_next();
    }

    return res;
    //return min_scanner->get_next();;
}

vector<RelScanner> RelSorter::get_scanner_list(TinyRelation* r, const SubListType& sub_list) const
{
    assert(sub_list.size() <= _mem_size);

    vector<RelScanner> res;
    for (size_t i = 0; i < sub_list.size(); ++i) {
        size_t mem_idx = _base_idx + i;
        const RelIter& it = sub_list[i].first;
        const RelIter& it_end = sub_list[i].second;

        //scanner_list.push_back( RelScanner(_sorted_relation, mem_idx, 1) );
        res.push_back( RelScanner(r, mem_idx, 1) );
        res.back().set_begin(it);
        res.back().set_end(it_end);

        //_scanner_list->back().dump();
        //cout << endl;
    }

    return res;
}

//------------------------------------------------------------------------------
//   TinyRelation
//------------------------------------------------------------------------------

TinyRelation::TinyRelation(Relation* r)
: _relation(r)
//, _is_tmp(false)
, _with_prefix(false)
//, _size(0)
{
    assert(_relation != NULL);
}

void TinyRelation::push_back(const TinyTuple& tuple)
{
    //dump_pretty(tuple);
    size_t mem_index = 0;
    //Block* block = HwMgr::ins()->get_mem_block(mem_index);
    TinyBlock block = HwMgr::ins()->get_mem_block(mem_index);
    if (next_is_new_block()) {
        //block->clear();
        block.clear();
        //block->appendTuple(tuple);
        block.push_back(tuple);
        _relation->setBlock(_relation->getNumOfBlocks(), mem_index);
    } else {
        _relation->getBlock(_relation->getNumOfBlocks()-1, mem_index);
        //block->appendTuple(tuple); // append the tuple
        block.push_back(tuple);
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

void TinyRelation::clear()
{
    _space.clear();
    reduce_blocks_to(0);
}

//void TinyRelation::set_with_prefix()
//{
//    _with_prefix = true;
//    dump_normal(get_name());
//    dump_normal(_with_prefix);
//}
//
//bool TinyRelation::is_with_prefix() const
//{
//    dump_normal(get_name());
//    dump_normal(_with_prefix);
//    return _with_prefix;
//}

void TinyRelation::refresh_block_num()
{
    size_t total_size = size() + _space.size();
    for (size_t i = _space.size(); i-- > 0;) {
        if (_space[i] == total_size - 1) {
            total_size--;
            _space.pop_back();
        } else {
            break;
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

void TinyRelation::reduce_blocks_to(size_t block_num) const
{
    if (block_num < get_num_of_block()) {
        _relation->deleteBlocks(block_num);
    }
}

TinyTuple TinyRelation::create_tuple(bool is_null) const
{
    TinyTuple t = _relation->createTuple();
    t.init();
    if (is_null) {
        t.set_null();
    }
    return t;
}

string TinyRelation::get_name() const
{
    return _relation->getRelationName();
}

string TinyRelation::get_base_name() const
{
    vector<string> toks = tokenize(get_name(), " ");
    
    return toks.back();
}

string TinyRelation::get_attr_search_name(const ColumnName& column_name) const
{
    string res = column_name;

    string table = column_name.get_table();
    if (!table.empty() && !is_with_prefix() && table == get_base_name()) {
        res = column_name.get_column();
    }

    return res;
}

vector<string> TinyRelation::get_attr_search_name_list(const vector<string>& attr_list) const
{
    vector<string> res;
    for (const string& attr : attr_list) {
        string search_name = get_attr_search_name(attr);
        res.push_back(search_name);
    }
    return res;
}

TinySchema TinyRelation::get_tiny_schema() const
{
    return TinySchema(_relation->getSchema());
}

size_t TinyRelation::size() const
{
    return (size_t)_relation->getNumOfTuples();
}

bool TinyRelation::empty() const
{
    return size() == 0;
}

size_t TinyRelation::get_num_of_attribute() const
{
    return get_tiny_schema().size();
}

vector<DataType> TinyRelation::get_type_list() const
{
    return get_tiny_schema().get_type_list();
}

vector<string> TinyRelation::get_attr_list() const
{
    return get_tiny_schema().get_attr_list();
}

vector<pair<string, DataType>> TinyRelation::get_attr_type_list() const
{
    return get_tiny_schema().get_attr_type_list();
}

vector<pair<string, DataType>> TinyRelation::get_attr_type_list_with_name() const
{
    vector<pair<string, DataType>> attr_list = get_tiny_schema().get_attr_type_list();
    vector<pair<string, DataType>> res;
    for (const auto& name_type : attr_list) {
        const string& name = name_type.first;
        res.push_back( make_pair(get_name() + "." + name, name_type.second) );
    }
    return res;
}

size_t TinyRelation::get_num_of_block() const
{
    return (size_t)_relation->getNumOfBlocks();
}

size_t TinyRelation::tuple_per_block() const
{
    return get_tiny_schema().tuple_per_block();
}

size_t TinyRelation::get_block_idx_by_pos(size_t pos) const
{
    return pos / tuple_per_block();
}

size_t TinyRelation::get_tuple_idx_by_pos(size_t pos) const
{
    return pos % tuple_per_block();
}

pair<size_t, size_t> TinyRelation::get_idx_by_pos(size_t pos) const
{
    pair<size_t, size_t> tmp;
    tmp.first = pos / tuple_per_block();
    tmp.second = pos % tuple_per_block();
    return tmp;
}

bool TinyRelation::is_null(size_t pos) const
{
    return is_contain(_space, pos);
}

bool TinyRelation::is_attr_exist(const string& name) const
{
    string search_name = get_attr_search_name(name);
    bool res = get_tiny_schema().is_field_name_exist(search_name);
    return res;
}

TinyRelation::iterator TinyRelation::begin() const
{
    return iterator(this);
}

TinyRelation::iterator TinyRelation::end() const
{
    size_t end_pos = get_total_pos();
    return iterator(this, end_pos);
}

size_t TinyRelation::get_total_pos() const
{
    return size() + _space.size();
}

bool TinyRelation::next_is_new_block() const
{
    return (size() + _space.size()) % tuple_per_block() == 0; 
}

void TinyRelation::print_table() const
{
    DrawTable table(get_num_of_attribute(), DrawTable::MYSQL_TABLE);
    vector<DataType> type_list = get_type_list();
    for (size_t i = 0; i < type_list.size(); ++i) {
        if (type_list[i] == TINY_INT) {
            table.set_align_right(i);
        }
    }

    table.set_header(get_attr_list());

    RelScanner scanner(this, 1, 1);
    while(!scanner.is_end()) {
        TinyTuple tuple = scanner.get_next();
        assert(!tuple.is_null());

        table.add_row(tuple.str_list());
        //to_relation->push_back(tuple);
    }
    //size_t mem_index = 0;
    //size_t num_of_block = get_num_of_block();
    ////dump_normal(num_of_block);
    //for (size_t i = 0; i < num_of_block; ++i) {
    //    relation->load_block_to_mem(i, mem_index);
    //    //_relation->getBlock(i, mem_index);
    //    //Block* block = HwMgr::ins()->get_mem_block(mem_index);
    //    TinyBlock block = HwMgr::ins()->get_mem_block(mem_index);
    //    //vector<Tuple> tuples = block->getTuples();
    //    vector<Tuple> tuples = block.get_tuples();
    //    for (const auto& tuple : tuples) {
    //        //dump_normal(TinyTuple(tuple));
    //        if (!tuple.isNull())
    //            table.add_row(TinyTuple(tuple).str_list());
    //    }
    //}

    if (table.size() == 0) {
        cout << "Empty set" << endl;
    } else {
        table.draw();
        cout << table.size() << " ";
        cout << ((table.size() == 1) ? "row" : "rows");
        cout << " in set";
        cout << endl;
    }
}

void TinyRelation::dump() const
{
    DrawTable table(2, DrawTable::MYSQL_TABLE);
    table.add_row("name", get_name());
    table.add_row("size", tiny_dump_str(size()));
    table.add_row("# of blocks", tiny_dump_str(get_num_of_block()));
    table.add_row("tuple per block", tiny_dump_str(tuple_per_block()));
    table.add_row("_with_prefix", tiny_dump_str(_with_prefix));
    //table.add_row("schema", get_tiny_schema().dump_str());
    //cout << "name: " << get_name() << endl;
    cout << "schema: " << get_tiny_schema().dump_str() << endl;
    //cout << "size: " << size() << endl; 
    //cout << "num of block: " << get_num_of_block() << endl; 
    //cout << "tuple per block: " << tuple_per_block() << endl; 
    cout << "_space" << jjjj222::dump_str(_space) << endl;
    table.draw();

    //DrawTable t(2);
    //t.add_row("name", get_name());
    //t.add_row("schema", get_tiny_schema().dump_str());
    //t.draw();
    dump_tuples();
}

void TinyRelation::dump_tuples() const
{
    DrawTable table(get_num_of_attribute() + 3, DrawTable::MYSQL_TABLE);
    table.set_align_right(0);
    table.set_align_right(1);
    vector<DataType> type_list = get_type_list();
    for (size_t i = 0; i < type_list.size(); ++i) {
        if (type_list[i] == TINY_INT) {
            table.set_align_right(i+3);
        }
    }

    vector<string> header = {"i", "j", "d"};
    add_into(header, get_attr_list());
    table.set_header(header);

    size_t mem_index = 0;
    size_t num_of_block = get_num_of_block();
    for (size_t i = 0; i < num_of_block; ++i) {
        _relation->getBlock(i, mem_index);
        //Block* block = HwMgr::ins()->get_mem_block(mem_index);
        TinyBlock tiny_block = HwMgr::ins()->get_mem_block(mem_index);
        //vector<Tuple> tuples = block->getTuples();
        //vector<Tuple> tuples = tiny_block.get_tuples();
        vector<TinyTuple> tuples = tiny_block.get_tiny_tuples();
        //for (const auto& tuple : tuples) {
        for (size_t j = 0; j < tuples.size(); ++j) {
            //const Tuple& tuple = tuples[j];
            const TinyTuple& tuple = tuples[j];

            vector<string> row = {jjjj222::dump_str(i), jjjj222::dump_str(j)};
            //cout << i << "-" << j;
            //if (tuple.isNull()) {
            if (tuple.is_null()) {
                //cout << "<delete>" << endl;
                row.push_back("D");
            } else {
                row.push_back("");
                //dump_normal(TinyTuple(tuple));
                //dump_normal(tuple);
                add_into(row, tuple.str_list());
            }
            table.add_row(row);
        }
    }

    table.draw();
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
