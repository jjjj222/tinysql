#ifndef DB_MGR_H
#define DB_MGR_H

#include <vector>
#include <string>
#include <utility>

using std::vector;
using std::string;
using std::pair;

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

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
enum DataType {
    TINY_INT,
    TINY_STR20,
    TINY_UNKNOWN
};

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

        Relation* create_relation(const string& name, const Schema&);
        Tuple create_tuple(const Relation&); // TODO
        Block* get_mem_block(size_t);
        vector<Tuple> get_block_tuple(const Block&) const;
        TinyRelation* get_tiny_relation(const string& name) const;

        //
        bool create_table(const string&, const vector<pair<string, DataType>>&);
        bool insert_into(const string&, const vector<pair<string, string>>&);

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

    private:
        static HwMgr*   _ins;

    private:
        //vector<string>          _table_names;
        vector<TinyRelation*>   _relations;

    private:
        MainMemory*             _mem;
        Disk*                   _disk;
        SchemaManager*          _schema_mgr;
};

//------------------------------------------------------------------------------
//   query mgr
//------------------------------------------------------------------------------
class QueryMgr
{
    public:
        QueryMgr() {};

        // exec
        bool exec_query(const string&);

    private:
        bool create_table(tree_node_t*);
        bool insert_into(tree_node_t*);
        bool select_from(tree_node_t*);

    private:
        vector<pair<string, DataType>> get_attribute_type_list(tree_node_t*);
        pair<string, DataType> get_name_type(tree_node_t*);
        vector<string> get_string_list(tree_node_t*);
};

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
void error_msg_table_not_exist(const string& name);

#endif
