#ifndef WRAPPER_H
#define WRAPPER_H

class MainMemory;
class Disk;
class SchemaManager;
class Schema;
class Relation;
class Tuple;
class Block;

class DataValue;

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
string dump_field_type_str(const FIELD_TYPE&);
string dump_tiny_type_str(const DataType&);
DataType field_to_tiny_type(const FIELD_TYPE&);
FIELD_TYPE tiny_to_field_type(const DataType&);
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
        TinyTuple(const Tuple&);
        ~TinyTuple();


        operator const Tuple& () const { return *_tuple; }
        //operator const vector<>& () const { return *_tuple; }

        void init();
        bool set_value(const string&, const string&);
        bool set_str_value(const string&, const string&);
        bool set_int_value(const string&, int);

        DataValue get_value(const string&) const;
        //string get_value_str(const string&) const;
        TinySchema get_tiny_schema() const;
        vector<string> get_attr_list() const;
        vector<string> str_list() const;
        //const string& get_str_value(const string&) const;
        //int get_int_value(const string&) const;

        void dump() const;
        string dump_str() const;


    private:
        Tuple* _tuple;
};

//------------------------------------------------------------------------------
//   TinyRelation
//------------------------------------------------------------------------------
class TinyRelation
{
    public:
        TinyRelation(Relation*);

        //operator const Relation& () const { return *_relation; }

        void push_back(const TinyTuple&);
        void add_space(size_t, size_t);
        void add_space(size_t);

        void refresh_block_num();
        bool load_block_to_mem(size_t, size_t) const;
        bool save_block_to_disk(size_t, size_t) const;

        bool reduce_blocks_to(size_t) const;

        // get
        TinyTuple create_tuple() const;

        string get_name() const;
        TinySchema get_tiny_schema() const;
        vector<string> get_attr_list() const;
        size_t size() const;
        size_t get_num_of_attribute() const;
        size_t get_num_of_block() const;
        size_t tuple_per_block() const;

        //
        bool next_is_new_block() const;

        // debug
        void dump() const;
        string dump_str() const;
        Relation* get_relation() const { return _relation; }

    private:
        Relation* _relation;

    private:
    //    //size_t              _size;
        vector<size_t>      _space;
};


#endif
