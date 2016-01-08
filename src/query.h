#ifndef QUERY_H
#define QUERY_H

#include "enum.h"

struct tree_node;
typedef struct tree_node tree_node_t;

class DataValue;
class TinyRelation;
class TinyTuple;
class RelScanner;
class ColumnName;
class ConditionMgr;

//------------------------------------------------------------------------------
//   ConditionNode
//------------------------------------------------------------------------------
class ConditionNode
{
    public:
        friend class ConditionMgr;

    public:
        ConditionNode() {}
        virtual ~ConditionNode();

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
        VarNode(const ConditionMgr*, const ColumnName&);

        DataValue get_value(const TinyTuple&) const;
        const string& get_table() const { return _table; }
        const string& get_column() const { return _column; }

        string dump_str() const;

    private:
        const ConditionMgr*     _cond_mgr;
        string                  _table;
        string                  _column;
};

class LiteralNode : public ConditionNode
{
    public:
        LiteralNode(const string&);

        // get
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

        // get
        DataValue get_value(const TinyTuple&) const;

        // debug
        string dump_str() const;

    private:
        int _value;
};

//------------------------------------------------------------------------------
//   ConditionMgr
//------------------------------------------------------------------------------
class ConditionMgr
{
    public:
        ConditionMgr(tree_node_t*, TinyRelation* relation);
        ~ConditionMgr();

        bool is_error() const { return _error; }
        bool is_tuple_match(const Tuple&);
        TinyRelation* get_tiny_relation() const { return _tiny_relation; }

        // debug
        void dump() const;

    private:
        bool build_condition_node(tree_node_t*);
        bool check_condition_tree() const;
        bool check_condition_rec(ConditionNode*) const;
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
        friend class QueryMgr;

    public:
        enum NodeType {
            DISTINCT,
            CROSS_PRODUCT,
            NATURAL_JOIN,
            WHERE,
            PROJECT,
            ORDER_BY,
            BASE_NODE
        };

    public:
        QueryNode();
        virtual ~QueryNode();

        void set_tmp_table(TinyRelation*);
        void set_relation(TinyRelation*);
        void add_child(QueryNode*);

        virtual bool print_result();
        virtual bool calculate_result();

        //get
        TinyRelation* get_or_create_relation();
        virtual NodeType get_type() const { return BASE_NODE; };

        bool has_node(NodeType) const;
        QueryNode* get_node(NodeType);
        QueryNode* get_node_by_base_table(const vector<string>&);
        vector<string> get_base_table_name() const;
        string get_base_name() const; // TODO
        virtual size_t get_estimate_size() const;

        bool check() const;

        // debug
        virtual void dump() const;
        virtual string dump_str() const;
        virtual void dump_tree(const string& indent, bool is_last) const;


    private:
        string get_type_str(NodeType) const;


    protected:
        const vector<QueryNode*>& get_childs() const { return _childs; }


    protected:
        TinyRelation*       _relation;
        bool                _delete_relation;
        QueryNode*          _parent;
        vector<QueryNode*>  _childs;
};

class DistinctNode : public QueryNode
{
    public:
        virtual NodeType get_type() const { return DISTINCT; }
        virtual bool calculate_result();
        string dump_str() const;
};

class OrderByNode : public QueryNode
{
    public:
        OrderByNode(const string&);

        virtual NodeType get_type() const { return ORDER_BY; }
        virtual bool calculate_result();

        string dump_str() const;

    private:
        string  _name;
};

class ProjectNode : public QueryNode
{
    public:
        ProjectNode(const vector<string>&);

        virtual NodeType get_type() const { return PROJECT; }
        virtual bool calculate_result();

        string dump_str() const;

    private:
        vector<string> _attr_list;
};

class WhereNode : public QueryNode
{
    public:
        WhereNode(tree_node_t* node);
        virtual NodeType get_type() const { return WHERE; }

        vector<tree_node_t*> split_and_equal() const;

        virtual bool calculate_result();
        string dump_str() const;
        void dump_tree(const string& indent, bool is_last) const;
    private:
        tree_node_t* _where_tree;
};

class CrossProductNode : public QueryNode
{
    public:
        virtual NodeType get_type() const { return CROSS_PRODUCT; }

        virtual bool calculate_result();

        virtual string dump_str() const;

        virtual size_t get_estimate_size() const;

        void split(); // TODO: remove

    public:
        void do_cross_product(const RelScanner&, const RelScanner&, bool);        
};

class NaturalJoinNode : public CrossProductNode
{
    public:
        NaturalJoinNode(const vector<string>&);
        virtual NodeType get_type() const { return NATURAL_JOIN; }

        virtual bool calculate_result();

        string dump_str() const;

    private:
        vector<string>  _attr_list;
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

        void print_elapse_io() const;

    private:
        bool insert_into_from_select(const string&, const vector<string>&, tree_node_t*);

        bool build_select_tree(tree_node_t*);
        QueryNode* build_cross_product(const vector<string>&);
        QueryNode* build_base_node(const string&);
        QueryNode* build_where(tree_node_t*);
        QueryNode* build_project(const vector<string>&);
        QueryNode* build_order_by(const string&);
        QueryNode* build_distinct();

        void optimize_select_tree();
        void optimize_cross_product(QueryNode*);
        void optimize_where_with_cross_product(QueryNode*);
        void create_natural_join(const vector<string>&);
        void insert_where(const string&, tree_node_t*);
        vector<string> get_all_table(tree_node_t*) const;

        bool check_select_tree() const;

    private:
        vector<pair<string, DataType>> get_attribute_type_list(tree_node_t*);
        pair<string, DataType> get_name_type(tree_node_t*);
        vector<string> get_string_list(tree_node_t*);

    private:
        QueryNode*  _select_root;
};


#endif
