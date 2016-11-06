#ifndef WRAPPER_H
#define WRAPPER_H

#include "enum.h"

class MainMemory;
class Disk;
class SchemaManager;
class Schema;
class Relation;
class Tuple;
class Block;

class TinyRelation;
class DataValue;
class ColumnName;
class MemRange;

//------------------------------------------------------------------------------
//   TinySchema
//------------------------------------------------------------------------------
class TinySchema
{
    public:
        TinySchema(const Schema&);
        TinySchema(const vector<pair<string, DataType>>&);
        TinySchema(const TinySchema&);
        TinySchema(TinySchema&&);
        ~TinySchema();

        // operator
        operator const Schema& () const { return *_schema; }
        bool operator==(const TinySchema& rhs) const { return is_equal_to(rhs); }
        TinySchema& operator=(const TinySchema& rhs) { assign(rhs); return *this; }

        // get
        vector<string> get_attr_list() const;
        vector<DataType> get_type_list() const;
        vector<pair<string, DataType>> get_attr_type_list() const;
        DataType get_data_type(const string&) const;
        
        size_t size() const;
        size_t tuple_per_block() const;

        // is
        bool is_field_name_exist(const string&) const;

        // debug
        void dump() const;
        string dump_str() const;

    private:
        void assign(const TinySchema&);
        bool is_equal_to(const TinySchema& rhs) const;

    private:
        Schema* _schema;
};

//------------------------------------------------------------------------------
//   TinyTuple
//------------------------------------------------------------------------------
class TinyTuple
{
    public:
        TinyTuple(const Tuple&);
        TinyTuple(const TinyTuple&);
        TinyTuple(TinyTuple&&);
        ~TinyTuple();

        // operator
        operator const Tuple& () const { return *_tuple; }
        TinyTuple& operator=(const TinyTuple& rhs) { assign(rhs); return *this; }
        bool operator==(const TinyTuple& rhs) const { return is_equal_to(rhs); }
        bool operator<(const TinyTuple& rhs) const { return is_less_than(rhs); }
        bool is_less_than_by_attr(const TinyTuple&, const vector<string>&) const;

        // set
        void init();
        void set_null();
        bool set_raw_value(const string&, const string&);
        void set_value(const string&, const DataValue&);
        void set_value(size_t, const DataValue&);
        void set_value(const TinyTuple&, const TinyTuple&);

        // get
        size_t size() const;
        TinySchema get_tiny_schema() const;
        DataType get_data_type(const string&) const;
        DataValue get_data_value(const string&) const;

        vector<string> get_attr_list() const;
        vector<DataType> get_type_list() const;
        vector<pair<string, DataType>> get_attr_type_list() const;
        vector<DataValue> get_value_list() const;
        vector<string> get_str_list() const;

        // is
        bool is_null() const;
        bool is_attr_exist(const string&) const;

        // debug
        void dump() const;
        string dump_str() const;

    private:
        void assign(const TinyTuple&);
        bool is_equal_to(const TinyTuple&) const;
        bool is_less_than(const TinyTuple&) const;

        bool set_str_value(const string&, const string&);
        bool set_int_value(const string&, int);

    private:
        Tuple* _tuple;
};

//------------------------------------------------------------------------------
//   TinyBlock
//------------------------------------------------------------------------------
class TinyBlock
{
    public:
        TinyBlock(Block*);

        vector<Tuple> get_tuples() const;
        vector<TinyTuple> get_tiny_tuples() const;

        TinyTuple get_tiny_tuple(size_t) const;

        size_t size();

        void remove_null_tuple();
        void null_tuple(size_t);
        void clear();
        void push_back(const TinyTuple&);
        void push_back(const vector<TinyTuple>&);
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
//   MemIter
//------------------------------------------------------------------------------
class MemIter
{
    public:
        MemIter(const TinyRelation*, size_t);

        // operator
        MemIter& operator++() { ++_pos; return *this; }
        MemIter operator++(int) { MemIter tmp(*this); ++tmp; return tmp; }
        bool operator==(const MemIter& rhs) const { return is_equal_to(rhs); }
        bool operator!=(const MemIter& rhs) const { return !is_equal_to(rhs); }

        // get
        size_t get_block_idx() const;
        size_t get_tuple_idx() const;

        void set_tuple(const TinyTuple&);
        TinyTuple get_tuple() const;

        // debug
        string dump_str() const;

    private:
        bool is_equal_to(const MemIter&) const;

    private:
        const TinyRelation*     _relation;
        size_t                  _pos;
};

//------------------------------------------------------------------------------
//   RelIter
//------------------------------------------------------------------------------
class RelIter
{
    public:
        RelIter(const TinyRelation*);
        RelIter(const TinyRelation*, size_t);

        // operator
        RelIter& operator++() { ++_pos; return *this; }
        RelIter operator++(int) { RelIter tmp(*this); ++tmp; return tmp; }
        bool operator==(const RelIter& rhs) const { return is_equal_to(rhs); }
        bool operator!=(const RelIter& rhs) const { return !is_equal_to(rhs); }
        bool operator<=(const RelIter& rhs) const { return !is_greater_than(rhs); }

        // get
        size_t get_block_idx() const;
        size_t get_tuple_idx() const;

        void skip_null();
        TinyTuple load_to_mem(size_t) const;
        TinyTuple get_from_mem(size_t) const;

        // is
        bool is_null() const;

        // debug
        string dump_str() const;

    private:
        bool is_equal_to(const RelIter&) const;
        bool is_greater_than(const RelIter&) const;

    private:
        const TinyRelation*     _relation;
        size_t                  _pos;
};

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
class RelRange
{
    public:
        RelRange(const RelIter&, const RelIter&);

        bool operator==(const RelRange& rhs) const { return is_equal_to(rhs); }

        const RelIter& begin() const { return _begin; }
        const RelIter& end() const { return _end; }
        size_t num_of_block() const;

        string dump_str() const;

    private:
        bool is_equal_to(const RelRange&) const;

    private:
        const RelIter   _begin;
        const RelIter   _end;
};

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
class RelWriter
{
    public:
        RelWriter(TinyRelation*);
        ~RelWriter();

        void push_back(const TinyTuple&);
        void flush();

    private:
        TinyRelation*       _relation;
        //size_t              _mem_idx;
        vector<TinyTuple>   _tuples;
};

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
class RelScanner
{
    public:
        RelScanner(const TinyRelation*, size_t, size_t); // TODO: remove
        RelScanner(const TinyRelation*, const MemRange&);

        void set_range(const RelRange&);
        void set_begin(const RelIter& it) { _iter = it; }
        void set_end(const RelIter& it) { _iter_end = it; }

        TinyTuple get_next();
        TinyTuple peep_next();
        bool sort(const vector<string>&);
        void load_to_mem();
        void add_mem_into(TinyRelation&) const;

        bool is_iter_end() const;
        bool is_end() const;

        MemIter m_iter_begin() const { return _m_iter; }
        MemIter m_iter_end() const { return _m_iter_end; }
        void skip_mem() { _m_iter = _m_iter_end; }

        void dump() const;
        string dump_str() const;

    private:
        void clear_mem();
        void clear_mem_block(size_t);

        MemIter m_begin();
        MemIter m_load_end();

        size_t get_last_mem_block() const;

    private:
        const TinyRelation*     _relation;
        size_t                  _base_idx;
        size_t                  _mem_size;
        MemIter                 _m_iter;
        MemIter                 _m_iter_end;
        RelIter                 _iter;
        RelIter                 _iter_end;
};

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
class RelSorter
{
    public:
        typedef vector<pair<RelIter, RelIter>> SubListType;

    public:
        RelSorter(TinyRelation*, size_t, size_t);

        vector<pair<DataValue, RelRange>> sort_return();
        vector<pair<DataValue, RelRange>> sort_return(const string&);
        void sort();
        void sort(const string&);

    private:
        TinyTuple get_max(vector<RelScanner>&);

        TinyRelation* reduce_sub_list(TinyRelation*, SubListType&);
        vector<RelScanner> get_scanner_list(TinyRelation*, const SubListType&) const;

    private:
        TinyRelation*   _relation;
        size_t          _base_idx;
        size_t          _mem_size;
        vector<string>  _attr_list;
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
        ~TinyRelation();

        void push_back(const TinyTuple&);
        void push_back_block(const vector<TinyTuple>&);
        void add_space(size_t, size_t);
        void add_space(size_t);
        void set_with_prefix() { _with_prefix = true; }
        void set_pipe_queue();
        void clear();

        void refresh_block_num();
        bool load_block_to_mem(size_t, size_t) const;
        bool save_block_to_disk(size_t, size_t) const;

        void reduce_blocks_to(size_t) const;

        // get
        TinyTuple create_null_tuple() const { return create_tuple(true); }
        TinyTuple create_tuple(bool = false) const;

        string get_name() const;
        string get_base_name() const;
        string get_attr_search_name(const ColumnName&) const;
        vector<string> get_attr_search_name_list(const vector<string>&) const;
        TinySchema get_tiny_schema() const;
        vector<DataType> get_type_list() const;
        vector<string> get_attr_list() const;
        vector<pair<string, DataType>> get_attr_type_list() const;
        vector<pair<string, DataType>> get_attr_type_list_with_name() const;
        const vector<TinyTuple>& get_pipe_queue() const;
        size_t size() const;
        size_t size_by_block() const;
        bool empty() const;
        size_t get_num_of_attribute() const;
        size_t get_num_of_block() const;
        size_t tuple_per_block() const;
        size_t get_block_idx_by_pos(size_t) const;
        size_t get_tuple_idx_by_pos(size_t) const;
        size_t get_last_block_capacity() const;
        pair<size_t, size_t> get_idx_by_pos(size_t) const;
        bool is_null(size_t) const;
        bool is_with_prefix() const { return _with_prefix; }
        bool is_attr_exist(const string&) const;
        bool is_pipe_queue() const { return _pipe_queue != NULL; }

        iterator begin() const;
        iterator end() const;

        void print_table() const;

        bool next_is_new_block() const;

        vector<pair<DataValue, RelRange>> get_sub_list_by_attr(
            const string& attr, size_t mem_idx) const; 

        // debug
        void dump() const;
        string dump_str() const;
        Relation* get_relation() const { return _relation; }
    private:
        void dump_tuples() const;

        size_t get_total_pos() const;

    private:
        Relation*           _relation;
        vector<TinyTuple>*  _pipe_queue;
        bool                _with_prefix;
        vector<size_t>      _space;
};


#endif
