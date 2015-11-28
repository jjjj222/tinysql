#ifndef ENUM_H
#define ENUM_H

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
enum DataType {
    TINY_INT,
    TINY_STR20,
    TINY_UNKNOWN
};

string tiny_dump_str(const DataType&);

#endif
