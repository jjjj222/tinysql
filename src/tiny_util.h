#ifndef TINY_UTIL_H
#define TINY_UTIL_H

#include <string>
#include <utility>

using namespace std;

#include "enum.h"

string get_literal_value(const string&);

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
class ColumnName
{
    public:
        ColumnName(const string&);
        ColumnName(const string&, const string&);

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
//   
//------------------------------------------------------------------------------

class DataValue {
    public:
        DataValue();
        DataValue(const string&);
        DataValue(int);

        bool operator==(const DataValue& rhs) const { return is_equal_to(rhs); }
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


#endif
