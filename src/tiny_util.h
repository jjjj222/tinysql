#ifndef TINY_UTIL_H
#define TINY_UTIL_H

#include <string>
#include <utility>

using namespace std;

#include "debug.h"

using namespace jjjj222;

#include "enum.h"

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
void error_msg_table_not_exist(const string& name);
void error_msg_attribute_not_exist(const string& name);
string get_literal_value(const string&);

//------------------------------------------------------------------------------
//   dump
//------------------------------------------------------------------------------
template<typename T>
string tiny_dump_str(const T& data)
{
    return jjjj222::dump_str(data);
}

template<typename T, typename D>
string tiny_dump_str(const T& data, const D& data2)
{
    return jjjj222::dump_str(data, data2);
}

//------------------------------------------------------------------------------
//   ColumnName
//------------------------------------------------------------------------------
class ColumnName
{
    public:
        ColumnName(const string&);
        ColumnName(const string&, const string&);
        operator string () const { return get_column_name(); }

        // get
        const string& get_table() const { return _table; }
        const string& get_column() const { return _column; }
        string get_column_name() const;

        // debug
        string dump_str() const { return get_column_name(); }

    private:
        string _table;
        string _column;
};


//------------------------------------------------------------------------------
//   DataValue
//------------------------------------------------------------------------------

class DataValue {
    public:
        DataValue();
        DataValue(const DataType&);
        DataValue(const string&);
        DataValue(int);

        // operator
        bool operator==(const DataValue& rhs) const { return is_equal_to(rhs); }
        bool operator!=(const DataValue& rhs) const { return !is_equal_to(rhs); }
        bool operator>(const DataValue& rhs) const { return is_greater_than(rhs); }
        bool operator<(const DataValue& rhs) const { return is_less_than(rhs); }
        DataValue& operator+=(const DataValue& rhs) { return add(rhs); }
        DataValue& operator-=(const DataValue& rhs) { return sub(rhs); }
        DataValue& operator*=(const DataValue& rhs) { return mult(rhs); }
        DataValue& operator/=(const DataValue& rhs) { return div(rhs); }
        friend DataValue operator+(DataValue lhs, const DataValue& rhs) {
            lhs += rhs; return lhs;
        }
        friend DataValue operator-(DataValue lhs, const DataValue& rhs) {
            lhs -= rhs; return lhs;
        }
        friend DataValue operator*(DataValue lhs, const DataValue& rhs) {
            lhs *= rhs; return lhs;
        }
        friend DataValue operator/(DataValue lhs, const DataValue& rhs) {
            lhs /= rhs; return lhs;
        }

        // get
        DataType get_type() const { return _type; }
        int get_int() const { return _integer; }
        string get_str() const { return _literal; }

        // debug
        string dump_str() const;

    private:
        bool is_equal_to(const DataValue& rhs) const; 
        bool is_greater_than(const DataValue& rhs) const;
        bool is_less_than(const DataValue& rhs) const;
        DataValue& add(const DataValue& rhs);
        DataValue& sub(const DataValue& rhs);
        DataValue& mult(const DataValue& rhs);
        DataValue& div(const DataValue& rhs);

    private:
        DataType    _type;
        int         _integer;
        string      _literal;
};

//------------------------------------------------------------------------------
//   MemRange
//------------------------------------------------------------------------------
class MemRange
{
    public:
        MemRange();
        MemRange(size_t base_idx, size_t size);

        // get
        size_t get_base_idx() const { return _base_idx; }
        size_t size() const { return _size; }
        MemRange get_first_block() const;        
        MemRange get_not_first_block() const;        

        // debug
        string dump_str() const;

    private:
        size_t  _base_idx;
        size_t  _size;
};

#endif
