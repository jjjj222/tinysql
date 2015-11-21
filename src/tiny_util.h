#ifndef TINY_UTIL_H
#define TINY_UTIL_H

#include <string>
#include <utility>

using namespace std;

#include "enum.h"

string get_literal_value(const string&);
//pair<string, string> get_column_name_value(const string& column_name);
//string build_column_name(const string&, const string&);

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
class ColumnName
{
    public:
        ColumnName(const string&);
        ColumnName(const string&, const string&);

        //
        const string& get_table() const { return _table; }
        const string& get_column() const { return _column; }
        string get_column_name() const;

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

        string dump_str() const;

    private:
        bool is_equal_to(const DataValue& rhs) const; 

    private:
        DataType    _type;
        int         _integer;
        string      _literal;
};


#endif
