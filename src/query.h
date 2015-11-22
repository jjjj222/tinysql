#ifndef QUERY_H
#define QUERY_H

#include "enum.h"

struct tree_node;
typedef struct tree_node tree_node_t;

class DataValue;
class TinyRelation;
class TinyTuple;
//class TinySchema;

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
class TableInfo
{
    public:
        TableInfo(const string&);

        //
        void set_is_in_disk() { _is_in_disk = true; }

        //
        const string& get_name() const { return _name; }

        //
        void print_table() const;

        //
        void dump() const;
        string dump_str() const;

    private:
        string  _name;
        bool    _is_in_disk;
};

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
class ConditionNode
{
    public:
        friend class ConditionMgr;

    //public:
    //    enum NodeType {
    //        VAR,
    //        LITERAL,
    //        INTEGER,
    //        AND,
    //        OR,
    //        NOT,
    //        COMP_EQ,
    //        COMP_GT,
    //        COMP_LT
    //    };

    //protected:
    public:
        ConditionNode() {}
        virtual ~ConditionNode();

        // get
        //virtual NodeType get_type() const = 0;

        virtual bool is_tuple_match(const TinyTuple&) const { return false; }
        virtual DataValue get_value(const TinyTuple&) const;

        virtual string dump_str() const { return "TODO"; }
        void dump() const;

    protected:
        const vector<ConditionNode*>& get_childs() const { return _childs; }

    private:
        void add_child(ConditionNode*);
        void dump_tree(const string& indent, bool is_last) const;

    private:
        vector<ConditionNode*>      _childs;
};

class AndNode : public ConditionNode
{
    public:
        bool is_tuple_match(const TinyTuple&) const;
};

class OrNode : public ConditionNode
{
    public:
        bool is_tuple_match(const TinyTuple&) const;
};

class NotNode : public ConditionNode
{
    public:
        bool is_tuple_match(const TinyTuple&) const;
};

class CompNode : public ConditionNode
{
    public:
        bool is_tuple_match(const TinyTuple&) const;

        virtual bool comp_op(const DataValue&, const DataValue&) const = 0;
};

class EqNode : public CompNode
{
    public:
        bool comp_op(const DataValue&, const DataValue&) const;
        string dump_str() const { return "="; }
};

class GtNode : public CompNode
{
    public:
        bool comp_op(const DataValue&, const DataValue&) const;
        string dump_str() const { return ">"; }
};

class LtNode : public CompNode
{
    public:
        bool comp_op(const DataValue&, const DataValue&) const;
        string dump_str() const { return "<"; }
};

class ArithNode : public ConditionNode
{
    public:
        DataValue get_value(const TinyTuple&) const;
        virtual DataValue arith_op(const DataValue&, const DataValue&) const = 0;
};

class AddNode : public ArithNode
{
    public:
        DataValue arith_op(const DataValue&, const DataValue&) const;
};

class MinusNode : public ArithNode
{
    public:
        DataValue arith_op(const DataValue&, const DataValue&) const;
};

class MultiNode : public ArithNode
{
    public:
        DataValue arith_op(const DataValue&, const DataValue&) const;
};

class DivNode : public ArithNode
{
    public:
        DataValue arith_op(const DataValue&, const DataValue&) const;
};

class VarNode : public ConditionNode
{
    public:
        VarNode(const string&, const string&);

        //NodeType get_type() const { return VAR; }
        DataValue get_value(const TinyTuple&) const;
        const string& get_table() const { return _table; }
        const string& get_column() const { return _column; }

        string dump_str() const;

    private:
        string _table;
        string _column;
};

class LiteralNode : public ConditionNode
{
    public:
        LiteralNode(const string&);

        //NodeType get_type() const { return LITERAL; }
        DataValue get_value(const TinyTuple&) const;

        // debug
        string dump_str() const { return _value; }

    private:
        string _value;
};

class IntegerNode : public ConditionNode
{
    public:
        IntegerNode(int);

        //NodeType get_type() const { return INTEGER; }
        DataValue get_value(const TinyTuple&) const;

        // debug
        string dump_str() const;

    private:
        int _value;
};

class ConditionMgr
{
    public:
        ConditionMgr(tree_node_t*, TinyRelation* relation);
        ~ConditionMgr();

        bool is_error() const { return _error; }
        bool is_tuple_match(const Tuple&);

        void dump() const;

    private:
        //void init(tree_node_t*);

        bool build_condition_node(tree_node_t*);
        bool check_condition_tree() const;
        bool check_condition_rec(ConditionNode*) const;
        //bool check_var_node(ConditionNode*) const;
        bool check_var_node(VarNode*) const;
        ConditionNode* build_condition_node_rec(tree_node_t*);
        ConditionNode* build_boolean_node(tree_node_t*);
        ConditionNode* build_comp_op_node(tree_node_t*);
        ConditionNode* build_arith_op_node(tree_node_t*);
        ConditionNode* build_expression_node(tree_node_t*);
        ConditionNode* build_var_node(tree_node_t*);
        ConditionNode* build_literal_node(tree_node_t*);
        ConditionNode* build_integer_node(tree_node_t*);

    private:
        ConditionNode*          _root;
        TinyRelation*           _tiny_relation;
        bool                    _error;
        TinyTuple*              _tiny_tuple;
};

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
class QueryNode
{
    public:
        enum NodeType {
            DISTINCT,
            CROSS_PRODUCT,
            WHERE,
            BASE_NODE
        };

    public:
        //QueryNode(NodeType);
        QueryNode();
        virtual ~QueryNode();

        void set_real_table(const string&);
        void add_child(QueryNode*);

        virtual void print_result();
        virtual void calculate_result();

        //get
        TableInfo* get_table_info() const { return _table_info; }
        virtual NodeType get_type() const { return BASE_NODE; };

    public:
        virtual void dump() const;
        virtual string dump_str() const;

    private:
        string get_type_str(NodeType) const;

        void dump_tree(const string& indent, bool is_last) const;

    protected:
        const vector<QueryNode*>& get_childs() const { return _childs; }

    private:
        TableInfo*          _table_info;
        vector<QueryNode*>  _childs;
};

class WhereNode : public QueryNode
{
    public:
        //WhereNode(const tree_node_t* node) 
        //WhereNode()

        virtual NodeType get_type() const { return WHERE; }

    private:
};

class CrossProductNode : public QueryNode
{
    public:
        virtual NodeType get_type() const { return CROSS_PRODUCT; }

        virtual void print_result();
    private:
};

//------------------------------------------------------------------------------
//   query mgr
//------------------------------------------------------------------------------
class QueryMgr
{
    public:
        QueryMgr();
        ~QueryMgr();

        // exec
        bool exec_query(const string&);

    private:
        bool create_table(tree_node_t*);
        bool drop_table(tree_node_t*);
        bool insert_into(tree_node_t*);
        bool delete_from(tree_node_t*);
        bool select_from(tree_node_t*);

    private:
        void build_select_tree(tree_node_t*);
        QueryNode* build_cross_product(const vector<string>&);
        QueryNode* build_base_node(const string&);
        QueryNode* build_where(const tree_node_t*);

    private:
        vector<pair<string, DataType>> get_attribute_type_list(tree_node_t*);
        pair<string, DataType> get_name_type(tree_node_t*);
        vector<string> get_string_list(tree_node_t*);

    private:
        QueryNode*  _root;
};


#endif
