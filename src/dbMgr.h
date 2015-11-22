#ifndef DB_MGR_H
#define DB_MGR_H

#include <vector>
#include <string>
#include <utility>

using std::vector;
using std::string;
using std::pair;

#include "enum.h"

struct tree_node;
typedef struct tree_node tree_node_t;

class MainMemory;
class Disk;
class SchemaManager;
class Schema;
class Relation;
class Tuple;
class Block;
//enum FIELD_TYPE;

class TinyRelation;
class TinyBlock;

//------------------------------------------------------------------------------
//   SqlParser
//------------------------------------------------------------------------------
class SqlParser
{
    public:
        SqlParser() {}
        ~SqlParser();

        tree_node_t* parse_string(const string&) const;

        // is
        bool is_error() const;

        // debug
        void dump() const;
};

//------------------------------------------------------------------------------
//   HwMgr
//------------------------------------------------------------------------------

class HwMgr
{
    public:
        ~HwMgr();

        Block* get_mem_block(size_t);
        TinyBlock get_tiny_block(size_t);
        size_t get_mem_size() const;
        //vector<Tuple> get_block_tuple(const Block&) const;
        TinyRelation* get_tiny_relation(const string& name) const;

        //
        bool create_table(const string&, const vector<pair<string, DataType>>&);
        bool drop_table(const string&);
        bool insert_into(const string&, const vector<pair<string, string>>&);
        bool delete_from(const string&, tree_node_t* where_node);
        //bool select_from(const string&);
        //bool select_from(
        //    const vector<string>&,
        //    const vector<string>&,
        //    tree_node_t*,
        //    bool is_distinct
        //);

        //bool select_from_single_table(
        //    const string&, 
        //    const vector<string>&,
        //    tree_node_t*,
        //    bool is_distinct
        //);

        // debug
        void dump();
        void dump_memory() const;
        void dump_relations() const;
        //void dump_schema(const Schema&) const;
        //void dump_relation(const Relation&) const;
        void dump_relation(const string&) const;
        void dump_tuple(const Tuple&) const;
        void dump_block(const Block&) const;

    public:
        static HwMgr* ins();

    private:
        HwMgr();
        void init();

        Relation* create_relation(const string& name, const Schema&);
        //Tuple create_tuple(const Relation&); // TODO

        Relation* get_relation(const string& name) const;
        bool delete_relation(const string& name) const;

        bool assert_relations() const;

    private:
        static HwMgr*   _ins;

    private:
        vector<TinyRelation*>   _relations;

    private:
        MainMemory*             _mem;
        Disk*                   _disk;
        SchemaManager*          _schema_mgr;
};

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
void error_msg_table_not_exist(const string& name);

#endif
