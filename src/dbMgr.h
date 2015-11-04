#ifndef DB_MGR_H
#define DB_MGR_H

#include <vector>

using std::vector;

struct tree_node;
typedef struct tree_node tree_node_t;

class MainMemory;
class Disk;
class SchemaManager;
class Schema;
class Relation;
class Tuple;
class Block;

//------------------------------------------------------------------------------
//   SqlParser
//------------------------------------------------------------------------------
class SqlParser
{
    public:
        SqlParser() {}
        ~SqlParser();

        tree_node_t* parse_string(const char*) const;

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

        //
        HwMgr* ins();

        // debug
        void dump();
        void dump_memory() const;
        void dump_schema(const Schema&) const;
        void dump_relation(const Relation&) const;
        void dump_tuple(const Tuple&) const;
        void dump_block(const Block&) const;

    private:
        HwMgr();

        void init();

    private:
        static HwMgr*   _ins;

    private:
        MainMemory*     _mem;
        Disk*           _disk;
        SchemaManager*  _schema_mgr;
};
#endif
