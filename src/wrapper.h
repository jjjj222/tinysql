#ifndef WRAPPER_H
#define WRAPPER_H

#include <string>
#include <vector>
#include <utility>

using namespace std;

#include "enum.h"
#include "Field.h"

class MainMemory;
class Disk;
class SchemaManager;
class Schema;
class Relation;
class Tuple;
class Block;

class TinyRelation;
class DataValue;

//------------------------------------------------------------------------------
//   TinySchema
//------------------------------------------------------------------------------
class TinySchema
{
    public:
        TinySchema(const Schema&);
        TinySchema(const vector<pair<string, DataType>>&);
        TinySchema(const TinySchema&);
        ~TinySchema();

        bool operator==(const TinySchema& rhs) const { return is_equal_to(rhs); }
        TinySchema& operator=(const TinySchema& rhs) { assign(rhs); return *this; }
        operator const Schema& () const { return *_schema; }

        vector<string> get_attr_list() const;
        vector<pair<string, FIELD_TYPE>> get_name_type_list() const;
        vector<pair<string, DataType>> get_attr_type_list() const;
        vector<DataType> get_type_list() const;
        DataType get_data_type(const string&) const;
        

        size_t size() const;
        size_t tuple_per_block() const;

        bool is_field_name_exist(const string&) const;
        //bool is_field_name_exist(const string&, const string&) const;
        //size_t count_field_name(const string&, const string&) const;

        // debug
        void dump() const;
        string dump_str() const;

    private:
        void assign(const TinySchema&);
        bool is_equal_to(const TinySchema& rhs) const;

        //bool is_field_name_exist(const string&) const;


    private:
        Schema* _schema;
};

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
class TinyTuple
{
    public:
        TinyTuple(TinyRelation*);
        TinyTuple(const Tuple&);
        TinyTuple(const TinyTuple&);
        ~TinyTuple();


        operator const Tuple& () const { return *_tuple; }
        TinyTuple& operator=(const TinyTuple& rhs) { assign(rhs); return *this; }
        //operator const vector<>& () const { return *_tuple; }

        void init();
        void set_null();
        bool set_raw_value(const string&, const string&);
        void set_value(const string&, const DataValue&);
        void set_value(size_t, const DataValue&);
        void set_value(const TinyTuple&, const TinyTuple&);

        size_t size() const;
        TinySchema get_tiny_schema() const;
        DataType get_data_type(const string&) const;
        DataValue get_value(const string&) const;

        vector<DataValue> get_value_list() const;
        //string get_value_str(const string&) const;
        vector<string> get_attr_list() const;
        vector<string> str_list() const;
        //const string& get_str_value(const string&) const;
        //int get_int_value(const string&) const;

        // is
        bool is_null() const;

        // debug
        void dump() const;
        string dump_str() const;
        vector<string> dump_str_list() const;

    private:
        void assign(const TinyTuple&);

        bool set_str_value(const string&, const string&);
        bool set_int_value(const string&, int);

    private:
        Tuple* _tuple;
};

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
class TinyBlock
{
    public:
        TinyBlock(Block*);

        vector<Tuple> get_tuples() const;
        vector<TinyTuple> get_tiny_tuples() const;

        TinyTuple get_tiny_tuple(size_t) const;
        //TinyTuple get_tuple(size_t) const;

        size_t size();

        void remove_null_tuple();
        void null_tuple(size_t);
        void clear();
        void push_back(const TinyTuple&);
        void set_tuple(size_t, const TinyTuple&);

        // is
        bool empty() const;

        // debug
        void dump() const;
        string dump_str() const;  
        vector<string> dump_str_list() const;

    private:
        Block* _block;
};

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
class MemIter
{
    public:
        MemIter(TinyRelation*, size_t);
        MemIter& operator++() { ++_pos; return *this; }
        MemIter operator++(int) { MemIter tmp(*this); ++tmp; return tmp; }
        bool operator==(const MemIter& rhs) const { return is_equal_to(rhs); }
        bool operator!=(const MemIter& rhs) const { return !is_equal_to(rhs); }

        void set_tuple(const TinyTuple&);
        TinyTuple get_tuple() const;
        //void clear_block();
        //void push_back(const TinyTuple&);

        size_t get_block_idx() const;
        size_t get_tuple_idx() const;

        string dump_str() const;
    private:
        bool is_equal_to(const MemIter&) const;

    private:
        TinyRelation*       _relation;
        size_t              _pos;
};

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
class RelIter
{
    public:
        RelIter(TinyRelation*);
        RelIter(TinyRelation*, size_t);
        RelIter& operator++() { ++_pos; return *this; }
        RelIter operator++(int) { RelIter tmp(*this); ++tmp; return tmp; }
        bool operator==(const RelIter& rhs) const { return is_equal_to(rhs); }
        bool operator!=(const RelIter& rhs) const { return !is_equal_to(rhs); }
        //TinyTuple operator*() const { return get_tuple(); }
        size_t get_block_idx() const;
        size_t get_tuple_idx() const;

        void skip_null();
        TinyTuple load_to_mem(size_t) const;
        TinyTuple get_from_mem(size_t) const;

        bool is_end() const;
        bool is_null() const;

        string dump_str() const;

    private:
        //void inc();
        bool is_equal_to(const RelIter&) const;
        //TinyTuple get_tuple() const;

    private:
        TinyRelation*   _relation;
        size_t          _pos;
        //size_t          _block;
        //size_t          _mem_idx;
};

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
class RelScanner
{
    public:
        RelScanner(TinyRelation*, size_t, size_t);

        TinyTuple get_next();

        bool is_end() const;

        void dump() const;

    private:
        void clear_mem();
        void clear_mem_block(size_t);

        //void move_to_non_null();
        void load_to_mem();
        //TinyTuple get_from_mem();
        MemIter m_begin();
        MemIter m_load_end();
        MemIter m_end();

        size_t get_last_mem_block() const;

    private:
        TinyRelation*   _relation;
        size_t          _base_idx;
        size_t          _mem_size;
        MemIter         _m_iter;
        MemIter         _m_iter_end;
        RelIter         _iter;
};


//------------------------------------------------------------------------------
//   TinyRelation
//------------------------------------------------------------------------------
class TinyRelation
{
    public:
        typedef RelIter iterator;

    public:
        TinyRelation(Relation*);

        //operator const Relation& () const { return *_relation; }

        void push_back(const TinyTuple&);
        void add_space(size_t, size_t);
        void add_space(size_t);
        void set_with_prefix() { _with_prefix = true; }

        void refresh_block_num();
        bool load_block_to_mem(size_t, size_t) const;
        bool save_block_to_disk(size_t, size_t) const;

        void reduce_blocks_to(size_t) const;

        // get
        TinyTuple create_null_tuple() const { return create_tuple(true); }
        TinyTuple create_tuple(bool = false) const;

        string get_name() const;
        TinySchema get_tiny_schema() const;
        vector<DataType> get_type_list() const;
        vector<string> get_attr_list() const;
        vector<pair<string, DataType>> get_attr_type_list() const;
        vector<pair<string, DataType>> get_attr_type_list_with_name() const;
        size_t size() const;
        size_t get_num_of_attribute() const;
        size_t get_num_of_block() const;
        size_t tuple_per_block() const;
        size_t get_block_idx_by_pos(size_t) const;
        size_t get_tuple_idx_by_pos(size_t) const;
        pair<size_t, size_t> get_idx_by_pos(size_t) const;
        bool is_null(size_t) const;
        bool is_with_prefix() const { return _with_prefix; }

        iterator begin();
        iterator end();

        //
        bool next_is_new_block() const;

        // debug
        void dump() const;
        string dump_str() const;
        Relation* get_relation() const { return _relation; }
    private:
        void dump_tuples() const;

        size_t get_total_pos() const;

    private:
        Relation*           _relation;
        bool                _with_prefix;
        vector<size_t>      _space;
};


#endif
