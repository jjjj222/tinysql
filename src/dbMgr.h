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

class TinySchema;
class TinyRelation;
class TinyBlock;
class MemRange;

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

        // get
        TinyBlock get_mem_block(size_t);
        size_t get_mem_size() const;
        size_t get_block_size() const;
        MemRange get_mem_range() const;

        TinyRelation* get_tiny_relation(const string& name) const;
        TinyRelation* create_relation(const string& name, const TinySchema&);
        TinyRelation* create_tmp_relation(TinyRelation*, TinyRelation*); // TODO

        // do
        bool create_table(const string&, const vector<pair<string, DataType>>&);
        bool drop_table(const string&);
        bool insert_into(const string&, const vector<pair<string, string>>&);
        bool delete_from(const string&, tree_node_t* where_node);
        
        // debug
        void dump();
        void dump_memory() const;
        void dump_relations() const;
        void dump_relation(const string&) const;

    public:
        static HwMgr* ins();

    private:
        HwMgr();
        void init();

        //bool is_table_exist(const string&) const;
        Relation* get_relation(const string& name) const;

    private:
        static HwMgr*   _ins;

    private:
        vector<TinyRelation*>   _relations;

    private:
        MainMemory*             _mem;
        Disk*                   _disk;
        SchemaManager*          _schema_mgr;
};

#endif
