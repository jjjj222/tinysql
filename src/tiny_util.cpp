#include <cassert>

using namespace std;

#include "debug.h"
#include "obj_util.h"

using namespace jjjj222;

#include "tiny_util.h"

#include "field.h"

string get_literal_value(const string& literal)
{
    assert(literal.front() == '\"');
    assert(literal.back() == '\"');

    return literal.substr(1, literal.size() - 2);
}

//DataType get_data_type_by_field_type(FIELD_TYPE t)
//{
//}

//pair<string, string> get_column_name_value(const string& column_name)
//{
//    vector<string> toks = tokenize(column_name, ".");
//
//    pair<string, string> res;
//    if (toks.size() == 1) {
//        res.second = toks[0];
//    } else {
//        assert(toks.size() == 2);
//
//        res.first = toks[0];
//        res.second = toks[1];
//    }
//
//    return res;
//}
//
//string build_column_name(const string& table, const string& column)
//{
//    assert(!column.empty());
//
//    string tmp;
//    if (!table.empty()) {
//        tmp += table;
//        tmp += ".";
//    }
//    tmp += column;
//
//    return tmp;
//}

//------------------------------------------------------------------------------
//   
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
//   
//------------------------------------------------------------------------------
DataValue::DataValue()
: _type(TINY_UNKNOWN)
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
        return jjjj222::dump_str(_integer);
    } else if (_type == TINY_STR20){
        return jjjj222::dump_str(_literal);
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
        //_literal += rhs._literal;
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
        //_literal += rhs._literal;
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
        //_literal += rhs._literal;
    }

    return *this;
}
