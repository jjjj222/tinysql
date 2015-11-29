#include <cassert>

using namespace std;

#include "debug.h"
#include "obj_util.h"

using namespace jjjj222;

#include "tiny_util.h"

#include "field.h"

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
void error_msg_table_not_exist(const string& name)
{
    error_msg_not_exist("Table", name);
}

string get_literal_value(const string& literal)
{
    assert(literal.front() == '\"');
    assert(literal.back() == '\"');

    return literal.substr(1, literal.size() - 2);
}

//------------------------------------------------------------------------------
//   ColumnName
//------------------------------------------------------------------------------
ColumnName::ColumnName(const string& column_name)
{
    vector<string> toks = tokenize(column_name, ".");

    if (toks.size() == 1) {
        _column = toks[0];
    } else {
        assert(toks.size() == 2);

        _table = toks[0];
        _column = toks[1];
    }
}

ColumnName::ColumnName(const string& table, const string& column)
: _table(table)
, _column(column)
{
    ;
}

string ColumnName::get_column_name() const
{
    if (_table.empty())
        return _column;
   
    return _table + "." + _column;
}

//------------------------------------------------------------------------------
//   DataValue
//------------------------------------------------------------------------------
DataValue::DataValue()
: _type(TINY_UNKNOWN)
, _integer(0)
{
    ;
}

DataValue::DataValue(const DataType& type)
: _type(type)
, _integer(0)
{
    ;
}

DataValue::DataValue(const string& s)
: _type(TINY_STR20)
, _integer(0)
, _literal(s)
{
}

DataValue::DataValue(int i)
: _type(TINY_INT)
, _integer(i)
{
}

string DataValue::dump_str() const
{
    if (_type == TINY_INT) {
        return tiny_dump_str(_integer);
    } else if (_type == TINY_STR20){
        return tiny_dump_str(_literal);
    } else {
        return "<TINY_UNKNOWN>";
    }
}

bool DataValue::is_equal_to(const DataValue& rhs) const
{
    assert(_type == rhs._type);

    if (_type == TINY_INT) {
        return _integer == rhs._integer;
    } else {
        assert(_type == TINY_STR20);
        return _literal == rhs._literal;
    }
}

bool DataValue::is_greater_than(const DataValue& rhs) const
{
    assert(_type == rhs._type);

    if (_type == TINY_INT) {
        return _integer > rhs._integer;
    } else {
        assert(_type == TINY_STR20);
        return _literal > rhs._literal;
    }
}

bool DataValue::is_less_than(const DataValue& rhs) const
{
    assert(_type == rhs._type);

    if (_type == TINY_INT) {
        return _integer < rhs._integer;
    } else {
        assert(_type == TINY_STR20);
        return _literal < rhs._literal;
    }
}

DataValue& DataValue::add(const DataValue& rhs)
{
    assert(_type == rhs._type);

    if (_type == TINY_INT) {
        _integer += rhs._integer;
    } else {
        assert(_type == TINY_STR20);
        _literal += rhs._literal;
    }

    return *this;
}

DataValue& DataValue::sub(const DataValue& rhs)
{
    assert(_type == rhs._type);

    if (_type == TINY_INT) {
        _integer -= rhs._integer;
    } else {
        assert(_type == TINY_STR20);
        //_literal -= rhs._literal;
    }

    return *this;
}

DataValue& DataValue::mult(const DataValue& rhs)
{
    assert(_type == rhs._type);

    if (_type == TINY_INT) {
        _integer *= rhs._integer;
    } else {
        assert(_type == TINY_STR20);
        //_literal *= rhs._literal;
    }

    return *this;
}

DataValue& DataValue::div(const DataValue& rhs)
{
    assert(_type == rhs._type);

    if (_type == TINY_INT) {
        _integer /= rhs._integer;
    } else {
        assert(_type == TINY_STR20);
        //_literal /= rhs._literal;
    }

    return *this;
}

//------------------------------------------------------------------------------
//   MemRange
//------------------------------------------------------------------------------
MemRange::MemRange()
: _base_idx(0)
, _size(0)
{
    ;
}

MemRange::MemRange(size_t base_idx, size_t size)
: _base_idx(base_idx)
, _size(size)
{
    ;
}

MemRange MemRange::get_first_block() const
{
    assert(_size > 0);

    return MemRange(_base_idx, 1); 
}

MemRange MemRange::get_not_first_block() const
{
    assert(_size > 1);

    return MemRange(_base_idx + 1, _size - 1); 
}

string MemRange::dump_str() const
{
    if (_size == 0) {
        return "<null>";
    }

    return tiny_dump_str(_base_idx, _base_idx + _size - 1);
}
