#include <iostream>
#include <cassert>

using std::cout;
using std::endl;

#include "obj_util.h"
#include "debug.h"

using namespace jjjj222;

#include "Block.h"
#include "Config.h"
#include "Disk.h"
#include "Field.h"
#include "MainMemory.h"
#include "Relation.h"
#include "Schema.h"
#include "SchemaManager.h"
#include "Tuple.h"

#include "enum.h"
#include "wrapper.h"
#include "query.h"
#include "dbMgr.h"
#include "parser.h"
#include "tiny_util.h"

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
TableInfo::TableInfo(const string& name)
: _name(name)
//, _is_in_disk(false)
, _is_tmp(false)
{
    ;
}

TableInfo::~TableInfo()
{
    if (_is_tmp) {
        HwMgr::ins()->drop_table(_name);
    }
}

void TableInfo::print_table() const
{
    //if (_is_in_disk) {
        //bool res = HwMgr::ins()->select_from(name_list, attr_list, where_tree, is_distinct);
        TinyRelation* relation = HwMgr::ins()->get_tiny_relation(_name);
        assert(relation != NULL);
        //if (relation == NULL) {
        //    error_msg_table_not_exist(table_name);
        //    return false;
        //}

        DrawTable table(relation->get_num_of_attribute(), DrawTable::MYSQL_TABLE);
        table.set_header(relation->get_attr_list());

        size_t mem_index = 0;
        size_t num_of_block = relation->get_num_of_block();
        //dump_normal(num_of_block);
        for (size_t i = 0; i < num_of_block; ++i) {
            relation->load_block_to_mem(i, mem_index);
            //_relation->getBlock(i, mem_index);
            //Block* block = HwMgr::ins()->get_mem_block(mem_index);
            TinyBlock block = HwMgr::ins()->get_mem_block(mem_index);
            //vector<Tuple> tuples = block->getTuples();
            vector<Tuple> tuples = block.get_tuples();
            for (const auto& tuple : tuples) {
                //dump_normal(TinyTuple(tuple));
                if (!tuple.isNull())
                    table.add_row(TinyTuple(tuple).str_list());
            }
        }

        table.draw();
        //cout << table.size() << " "
    //} else {
    //    cout << "TODO: !_is_in_disk" << endl;
    //}
}

void TableInfo::dump() const
{
    cout << dump_str() << endl;
}

string TableInfo::dump_str() const
{
    return _name;
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
ConditionNode::~ConditionNode()
{
    delete_all(_childs);
}

void ConditionNode::add_child(ConditionNode* node)
{
    assert(node != NULL);
    _childs.push_back(node);
}

DataValue ConditionNode::get_value(const TinyTuple&) const
{
    return DataValue();
}

void ConditionNode::dump() const
{
    dump_tree("", true);
}

void ConditionNode::dump_tree(const string& indent, bool is_last) const
{
    const string current_indent = indent + (is_last ? " `- " : " |- ");
    cout << current_indent << dump_str() << endl;

    const string next_indent = indent + (is_last ? "    " : " |  ");
    for (size_t i = 0; i < _childs.size(); ++i) {
        const ConditionNode* node_ptr = _childs[i];
        node_ptr->dump_tree(next_indent, i == _childs.size() - 1);
    }
}

bool AndNode::is_tuple_match(const TinyTuple& tuple) const
{
    const vector<ConditionNode*>& childs = get_childs();
    assert(childs.size() >= 2);

    for (const auto& child_ptr : childs) {
        bool res = child_ptr->is_tuple_match(tuple);
        if (!res)
            return false;
    }

    return true;
}

bool OrNode::is_tuple_match(const TinyTuple& tuple) const
{
    const vector<ConditionNode*>& childs = get_childs();
    assert(childs.size() >= 2);

    for (const auto& child_ptr : childs) {
        bool res = child_ptr->is_tuple_match(tuple);
        if (res)
            return true;
    }

    return false;
}

bool NotNode::is_tuple_match(const TinyTuple& tuple) const
{
    const vector<ConditionNode*>& childs = get_childs();
    assert(childs.size() == 1);

    bool res = !childs[0]->is_tuple_match(tuple);
    return res;
}

bool CompNode::is_tuple_match(const TinyTuple& tiny_tuple) const
{
    const vector<ConditionNode*>& childs = get_childs();
    assert(childs.size() == 2);

    DataValue l_v = childs[0]->get_value(tiny_tuple);
    DataValue r_v = childs[1]->get_value(tiny_tuple);

    bool res = comp_op(l_v, r_v);
    return res;
}

bool EqNode::comp_op(const DataValue& lhs, const DataValue& rhs) const
{
    return lhs == rhs;
}

bool GtNode::comp_op(const DataValue& lhs, const DataValue& rhs) const
{
    return lhs > rhs;
}

bool LtNode::comp_op(const DataValue& lhs, const DataValue& rhs) const
{
    return lhs < rhs;
}

VarNode::VarNode(const string& table, const string& column)
: _table(table)
, _column(column)
{
    ;
}

DataValue VarNode::get_value(const TinyTuple& tuple) const
{
    ColumnName column_name(_table, _column);
    return tuple.get_value(column_name.get_column_name());
}

string VarNode::dump_str() const
{
    if (_table.empty())
        return _column;

    return _table + "." + _column;
}

LiteralNode::LiteralNode(const string& value)
: _value(value)
{
    ;
}

DataValue ArithNode::get_value(const TinyTuple& tuple) const
{
    const vector<ConditionNode*>& childs = get_childs();
    assert(childs.size() >= 2);

    //DataValue l_v = childs[0]->get_value(tiny_tuple);
    //DataValue r_v = childs[1]->get_value(tiny_tuple);
    DataValue tmp = childs[0]->get_value(tuple);
    for (size_t i = 1; i < childs.size(); ++i) {
        ConditionNode* child_ptr = childs[i];
        tmp = arith_op(tmp, child_ptr->get_value(tuple));
    }

    //bool res = comp_op(l_v, r_v);
    //return res;
    return tmp;
}


DataValue AddNode::arith_op(const DataValue& lhs, const DataValue& rhs) const
{
    return lhs + rhs;
}

DataValue MinusNode::arith_op(const DataValue& lhs, const DataValue& rhs) const
{
    return lhs - rhs;
}

DataValue MultiNode::arith_op(const DataValue& lhs, const DataValue& rhs) const
{
    return lhs * rhs;
}

DataValue DivNode::arith_op(const DataValue& lhs, const DataValue& rhs) const
{
    return lhs / rhs;
}

DataValue LiteralNode::get_value(const TinyTuple&) const
{
    return DataValue(_value);
}

IntegerNode::IntegerNode(int value)
: _value(value)
{
    ;
}

DataValue IntegerNode::get_value(const TinyTuple&) const
{
    return DataValue(_value);
}

string IntegerNode::dump_str() const
{
    return jjjj222::dump_str(_value);
}
//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
ConditionMgr::ConditionMgr(tree_node_t* node, TinyRelation* relation)
: _root(NULL)
, _tiny_relation(relation)
, _error(false)
, _tiny_tuple(NULL)
{
    assert(relation != NULL);

    //_relations.push_back(relation);
    //init(node);
    if (node != NULL) {
        bool res = build_condition_node(node);
        _error = !res;
    }
}

ConditionMgr::~ConditionMgr()
{
    delete_not_null(_root);
    delete_not_null(_tiny_tuple);
    //delete_not_null(_tiny_schema);
}

bool ConditionMgr::is_tuple_match(const Tuple& tuple)
{
    if (_root == NULL)
        return true;

    assert(TinyTuple(tuple).get_tiny_schema() == _tiny_relation->get_tiny_schema());

    delete_not_null(_tiny_tuple);
    _tiny_tuple = new TinyTuple(tuple);

    bool res = _root->is_tuple_match(*_tiny_tuple);
    return res;
}


bool ConditionMgr::build_condition_node(tree_node_t* node)
{
    assert(node != NULL);
    assert(node_is_where(node));

    tree_node_t* child = node->child;
    assert(child != NULL);

    _root = build_condition_node_rec(child);

    bool res = check_condition_tree();
    return res;
}

ConditionNode* ConditionMgr::build_condition_node_rec(tree_node_t* node)
{
    assert(node != NULL);

    ConditionNode* node_ptr = NULL;
    if (node_is_or(node) || node_is_and(node) || node_is_not(node)) {
        node_ptr = build_boolean_node(node);
    } else if (node_is_comp_op(node)) {
        node_ptr = build_comp_op_node(node);
    } else {
        assert(false);
    }

    return node_ptr;
}

bool ConditionMgr::check_condition_tree() const
{
    assert(_root != NULL);

    bool res = check_condition_rec(_root); 
    return res;
}

bool ConditionMgr::check_condition_rec(ConditionNode* node) const
{
    assert(node != NULL);

    for (const auto& child_ptr : node->_childs) {
        if (check_condition_rec(child_ptr) == false)
            return false;
    }

    VarNode* var_node = dynamic_cast<VarNode*>(node);
    //if (node->get_type() == ConditionNode::VAR) {
    if (var_node != NULL) {
        if (check_var_node(var_node) == false)
            return false;
    }

    return true;
}

//bool ConditionMgr::check_var_node(ConditionNode* node) const
bool ConditionMgr::check_var_node(VarNode* node) const
{
    assert(node != NULL);
    //assert(node->get_type() == ConditionNode::VAR);

    //VarNode* var_node = dynamic_cast<VarNode*>(node);
    //assert(var_node != NULL);

    //string table = var_node->get_table();
    //string column = var_node->get_column();
    string table = node->get_table();
    string column = node->get_column();
    ColumnName column_name(table, column);

    string field_name;

    if (table.empty()) {
        if (_tiny_relation->is_with_prefix()) {
            error_msg("ambiguous \'" + column + "\'");
            return false;
            //field_name = table + "." + column;
            //field_name = column_name;
            //field_name = column;
        } else {

            field_name = column;
        }
    } else { // !table.empty()
        if (_tiny_relation->is_with_prefix()) {
            //field_name = table + "." + column;
            field_name = column_name;
        } else {
            if (table != _tiny_relation->get_base_name()) {
                error_msg_not_exist("attribute", column_name);
                return false;
            }

            field_name = column;
        }

    }
    //if (!table.empty() && _tiny_relation->get_name() != table) {
    //dump_normal(field_name);

    TinySchema schema = _tiny_relation->get_tiny_schema();
    if (!schema.is_field_name_exist(field_name)) {
        //error_msg_not_exist("attribute", build_column_name(table, column));
        error_msg_not_exist("attribute", column_name.get_column_name());
        return false;
    }
    //schema

    //if (table.empty()) {
        //size_t count = _tiny_schema->count_field_name(var_node->get_table(), var_node->get_column());

    //} else {
    
        //for (const auto& relation_ptr : _relations) {
        //    if (relation_ptr->get_name == )
        //}
    //}
    //dump_normal(table);
    //dump_normal(column);
        //if (count == 0) {
        //    error_msg("Con't find \'" + var_node->dump_str() + "\'" );
        //    return false;
        //} else if (count >= 2) {
        //    error_msg("Ambiguous \'" + var_node->dump_str() + "\'" );
        //    return false;
        //}
    return true;
}

ConditionNode* ConditionMgr::build_boolean_node(tree_node_t* node)
{
    assert(node != NULL);

    tree_node_t* child = node->child;
    assert(child != NULL);

    ConditionNode* new_node = NULL;

    if (node_is_or(node)) {
        new_node = new OrNode();
    } else if (node_is_and(node)) {
        new_node = new AndNode();
    } else if (node_is_not(node)) {
        new_node = new NotNode();
    } else {
        // do nothing
    }
    assert(new_node != NULL);
    
    while (child != NULL) {
        ConditionNode* tmp = build_condition_node_rec(child);
        new_node->add_child(tmp);
        child = child->next;
    }

    return new_node;
}

ConditionNode* ConditionMgr::build_comp_op_node(tree_node_t* node)
{
    assert(node != NULL);
    assert(node_is_comp_op(node));

    tree_node_t* l_child = node->child;
    assert(l_child != NULL);

    tree_node_t* r_child = l_child->next;
    assert(r_child != NULL);
    assert(r_child->next == NULL);

    ConditionNode* l_new_node = build_expression_node(l_child);
    ConditionNode* r_new_node = build_expression_node(r_child);
    assert(l_new_node != NULL);
    assert(r_new_node != NULL);

    string comp_type = node->value;
    
    ConditionNode* new_node = NULL;
    if (comp_type == "=") {
        new_node = new EqNode();
    } else if (comp_type == ">") {
        new_node = new GtNode();
    } else {
        assert(comp_type == "<");
        new_node = new LtNode();
    }

    new_node->add_child(l_new_node);
    new_node->add_child(r_new_node);

    return new_node;
}

ConditionNode* ConditionMgr::build_arith_op_node(tree_node_t* node)
{
    assert(node != NULL);
    assert(node_is_arith_op(node));

    tree_node_t* child = node->child;
    assert(child != NULL);

    ConditionNode* new_node = NULL;

    switch (node->type) {
        case '+':
            new_node = new AddNode();
            break;
        case '-':
            new_node = new MinusNode();
            break;
        case '*':
            new_node = new MultiNode();
            break;
        case '/':
            new_node = new DivNode();
            break;
    }
    assert(new_node != NULL);
    
    while (child != NULL) {
        ConditionNode* tmp = build_expression_node(child);
        new_node->add_child(tmp);
        child = child->next;
    }

    return new_node;
}

ConditionNode* ConditionMgr::build_expression_node(tree_node_t* node)
{
    assert(node != NULL);

    ConditionNode* new_node = NULL;
    if (node_is_name(node) || node_is_column_name(node)) {
        new_node = build_var_node(node);
    } else if (node_is_literal(node)) {
        new_node = build_literal_node(node);
    } else if (node_is_integer(node)) {
        new_node = build_integer_node(node);
    } else if (node_is_arith_op(node)) {
        new_node = build_arith_op_node(node);
    } else {
        //cout << "todo: expression" << endl;
    }

    return new_node;
}

ConditionNode* ConditionMgr::build_var_node(tree_node_t* node)
{
    assert(node != NULL);
    assert(node_is_name(node) || node_is_column_name(node));

    //string var_name;
    //string table_name;
    //if (node_is_column_name(node)) {
    //pair<string, string> table_var = get_column_name_value(node->value);
    ColumnName column_name(node->value);
    //dump_normal(table_var);
    //ConditionNode* new_node = new VarNode(table_var.first, table_var.second);
    ConditionNode* new_node = new VarNode(column_name.get_table(), column_name.get_column());

    return new_node;
}

ConditionNode* ConditionMgr::build_literal_node(tree_node_t* node)
{
    assert(node != NULL);
    assert(node_is_literal(node));

    string value = get_literal_value(node->value);

    ConditionNode* new_node = new LiteralNode(value);
    return new_node;
}

ConditionNode* ConditionMgr::build_integer_node(tree_node_t* node)
{
    assert(node != NULL);
    assert(node_is_integer(node));

    int value = str_to<int>(node->value);

    ConditionNode* new_node = new IntegerNode(value);
    return new_node;
}

void ConditionMgr::dump() const
{
    //dump_normal(_tiny_schema);
    dump_normal(_error);
    dump_normal(_tiny_relation->get_tiny_schema());
    dump_pretty(_root);
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
QueryMgr::QueryMgr()
: _select_root(NULL)
{
    ;
}

QueryMgr::~QueryMgr()
{
    delete_not_null(_select_root);
}

bool QueryMgr::exec_query(const string& query)
{
    SqlParser parser;
    tree_node_t* root = parser.parse_string(query);
    //parser.dump();

    if (parser.is_error())
        return false;


    if (root == NULL)
        return true;

    bool error = false;
    assert(root->next == NULL);

    if (node_is_create_table(root)) {
        if (!create_table(root))
            error = true;
    } else if (node_is_drop_table(root)) {
        if (!drop_table(root))
            error = true;
    } else if (node_is_insert(root)) {
        if (!insert_into(root))
            error = true;
    } else if (node_is_delete(root)) {
        if (!delete_from(root))
            error = true;
    } else if (node_is_select(root)) {
        if (!select_from(root))
            error = true;
    } else {

    }

    return !error;
} 

bool QueryMgr::create_table(tree_node_t* node)
{
    assert(node != NULL);
    assert(node_is_create_table(node));

    tree_node_t* child = node->child;
    assert(child != NULL);
    assert(child->next != NULL);

    string name = child->value; 
    vector<pair<string, DataType>> attr_list = get_attribute_type_list(child->next);

    //dump_normal(name);
    //dump_pretty(attr_list);

    //return true;
    bool res = HwMgr::ins()->create_table(name, attr_list);
    return res;
}

bool QueryMgr::drop_table(tree_node_t* node)
{
    assert(node != NULL);
    assert(node_is_drop_table(node));

    tree_node_t* name_node = node->child;
    assert(name_node != NULL);
    assert(name_node->next == NULL);
    assert(name_node->child == NULL);

    
    bool res = HwMgr::ins()->drop_table(name_node->value);
    return res;
}

bool QueryMgr::insert_into(tree_node_t* node)
{
    assert(node != NULL);
    assert(node_is_insert(node));

    tree_node_t* name_node = node->child;
    assert(name_node != NULL);
    string table_name = name_node->value;

    tree_node_t* name_list_node = name_node->next;
    assert(name_list_node != NULL);
    vector<string> name_list = get_string_list(name_list_node);
    //dump_pretty(name_list);

    assert(name_list_node->next != NULL);
    tree_node_t* tuples_node = name_list_node->next->child;
    assert(tuples_node != NULL);

    if (node_is_select(tuples_node)) {
        cout << "TODO: SELECT in INSERT" << endl;
        return true;
    }   

    assert(node_is_value_list(tuples_node));

    vector<string> value_list = get_string_list(tuples_node);
    if (name_list.size() != value_list.size()) {
        error_msg("name_list.size() != value_list.size()");
        return false;
    }

    //dump_pretty(value_list);
    vector<pair<string, string>> data = vector_make_pair(name_list, value_list);
    //dump_pretty(data);
    bool res = HwMgr::ins()->insert_into(table_name, data);
    return res;
}

bool QueryMgr::delete_from(tree_node_t* node)
{
    assert(node != NULL);
    assert(node_is_delete(node));

    tree_node_t* name_node = node->child;
    assert(name_node != NULL);
    string name = name_node->value;

    tree_node_t* where_node = name_node->next;

    bool res = HwMgr::ins()->delete_from(name, where_node);
    return res;
}

bool QueryMgr::select_from(tree_node_t* node)
{
    assert(node != NULL);
    assert(node_is_select(node));

    // TODO
    bool is_ok = build_select_tree(node);
    if (!is_ok) {
        return false;
    }

    //dump_print(_root);
    if (_select_root == NULL)
        return false;

    dump_pretty(_select_root);
    //return true;
    bool res = _select_root->print_result();

    return res;
}

bool QueryMgr::build_select_tree(tree_node_t* node)
{
    assert(node != NULL);
    assert(node_is_select(node));
    assert(_select_root == NULL);

    tree_node_t* child = node->child;
    assert(child != NULL);

    // DISTINCT
    bool is_distinct = false;
    if (node_is_distinct(child)) {
        is_distinct = true;
        child = child->next;
        assert(child != NULL);
    }

    // SELECT *
    vector<string> attr_list;
    if (child->type != '*') {
        assert(node_is_select_list(child));
        attr_list = get_string_list(child);
    }

    // FROM
    tree_node_t* name_list_node = child->next;
    assert(name_list_node != NULL);
    vector<string> name_list = get_string_list(name_list_node);

    // WHERE
    child = name_list_node->next;
    tree_node_t* where_tree = NULL;
    if (child != NULL && node_is_where(child)) {
        where_tree = child;
        child = child->next;
    }

    // ORDER BY
    string order_by = "";
    if (child != NULL) {
        assert(node_is_order_by(child));
        assert(child->child != NULL);
        order_by = child->child->value;
    }
    //dump_normal(name_list);
    //while (child != NULL) {
    //    //attr_list.push_back(get_name_type(child));
    //    child = child->next;
    //}    

    //delete_not_null(_select_root);

    QueryNode* cross_product_node = build_cross_product(name_list);
    if (cross_product_node == NULL)
        return false;

    _select_root = cross_product_node;

    if (where_tree != NULL) {
        QueryNode* where_node = build_where(where_tree);
        where_node->add_child(_select_root);
        _select_root = where_node;
    }

    if (!attr_list.empty()) {
        // TODO: build fail; return false;
        QueryNode* project_node = build_project(attr_list);
        project_node->add_child(_select_root);
        _select_root = project_node;
    }

    if (!order_by.empty()) {
        // TODO: build fail; return false;
        QueryNode* order_by_node = build_order_by(order_by);
        order_by_node->add_child(_select_root);
        _select_root = order_by_node;
    }

    if (is_distinct) {
        QueryNode* distinct_node = build_distinct();
        distinct_node->add_child(_select_root);
        _select_root = distinct_node;
    }

    return true;
}

QueryNode* QueryMgr::build_cross_product(const vector<string>& table_list)
{
    assert(!table_list.empty()); 

    QueryNode* node_ptr = NULL;
    if (table_list.size() == 1) {
        node_ptr = build_base_node(table_list[0]);
        if (node_ptr == NULL)
            return NULL;
    } else {
        node_ptr = new CrossProductNode();
        QueryNode* child_ptr = NULL;
        for (const auto& table_name : table_list) {
            child_ptr = build_base_node(table_name);
            if (child_ptr == NULL)
                return NULL;
            node_ptr->add_child(child_ptr);
        }
    }

    return node_ptr;
}

QueryNode* QueryMgr::build_base_node(const string& table_name)
{
    TinyRelation* relation = HwMgr::ins()->get_tiny_relation(table_name);
    if (relation == NULL) {
        error_msg_table_not_exist(table_name);
        return NULL;
    }

    QueryNode* node_ptr = new QueryNode();
    node_ptr->set_real_table(table_name);
    return node_ptr;
}

QueryNode* QueryMgr::build_where(tree_node_t* node)
{
    QueryNode* res = new WhereNode(node);
    return res;
}

QueryNode* QueryMgr::build_project(const vector<string>& attr_list)
{
    QueryNode* res = new ProjectNode(attr_list);
    return res;
}

QueryNode* QueryMgr::build_order_by(const string& name)
{
    QueryNode* res = new OrderByNode(name);
    return res;
}

QueryNode* QueryMgr::build_distinct()
{
    QueryNode* res = new DistinctNode();
    return res;
}


vector<pair<string, DataType>> QueryMgr::get_attribute_type_list(tree_node_t* node)
{
    assert(node != NULL);
    assert(node_is_attribute_type_list(node));

    tree_node_t* child = node->child;
    assert(child != NULL);

    vector<pair<string, DataType>> attr_list;
    while (child != NULL) {
        attr_list.push_back(get_name_type(child));
        child = child->next;
    }    
    return attr_list;
}

pair<string, DataType> QueryMgr::get_name_type(tree_node_t* node)
{
    assert(node != NULL);
    assert(node_is_name_type(node));

    tree_node_t* child = node->child;
    assert(child != NULL);
    assert(child->next != NULL);

    string name = child->value;
    DataType type = TINY_STR20;
    if (node_is_int(child->next)) {
        type = TINY_INT;
    } else {
        assert(node_is_str20(child->next));
    }

    return make_pair(name, type);
}

vector<string> QueryMgr::get_string_list(tree_node_t* node)
{
    assert(node != NULL);

    tree_node_t* child = node->child;
    assert(child != NULL);

    vector<string> string_list;
    while (child != NULL) {
        if (node_is_null(child)) {
            string_list.push_back("NULL");
        } else {
            string_list.push_back(child->value);
        }
        child = child->next;
    }

    return string_list;
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
QueryNode::QueryNode()
: _table_info(NULL)
{
    ;
}
QueryNode::~QueryNode()
{
    delete_not_null(_table_info);
    delete_all(_childs);
}

void QueryNode::add_child(QueryNode* child)
{
    assert(child != NULL);

    _childs.push_back(child);
}

void QueryNode::set_tmp_table()
{
    assert(_table_info != NULL);
    _table_info->set_is_tmp();
}

void QueryNode::set_real_table(const string& name)
{
    delete_not_null(_table_info);
    _table_info = new TableInfo(name);
    //_table_info->set_is_in_disk();
}

bool QueryNode::print_result()
{
    //assert(_table_info != NULL);
    if (_table_info == NULL) {
        if(!calculate_result())
            return false;
    }
    
    assert(_table_info != NULL);
    _table_info->print_table(); 
    return true;
}

bool QueryNode::calculate_result()
{
    assert(_table_info != NULL);
    // do nothing
    return true;
}

TinyRelation* QueryNode::get_or_create_relation()
{
    if (_table_info == NULL) {
        if (!calculate_result())
            return NULL;
    }
    
    assert(_table_info != NULL);
    TinyRelation* res = HwMgr::ins()->get_tiny_relation(_table_info->get_name());

    assert(res != NULL);
    return res;
}

#define NODE_TYPE(name) case name: return #name
string QueryNode::get_type_str(NodeType t) const
{
    switch (t) {
        NODE_TYPE(DISTINCT);
        NODE_TYPE(CROSS_PRODUCT);
        NODE_TYPE(WHERE);
        NODE_TYPE(PROJECT);
        NODE_TYPE(ORDER_BY);
        NODE_TYPE(BASE_NODE);
    }

    return "ERROR";
}

void QueryNode::dump() const
{
    //cout << dump_str() << endl;
    dump_tree("", true);
}

string QueryNode::dump_str() const
{
    string tmp = "{";
    tmp += get_type_str(get_type());
    tmp += ": ";
    tmp += jjjj222::dump_str(_table_info);
    tmp += "}";
    return tmp;
}

void QueryNode::dump_tree(const string& indent, bool is_last) const
{
    const string current_indent = indent + (is_last ? " `- " : " |- ");
    cout << current_indent << dump_str() << endl;

    const string next_indent = indent + (is_last ? "    " : " |  ");
    for (size_t i = 0; i < _childs.size(); ++i) {
        const QueryNode* node_ptr = _childs[i];
        node_ptr->dump_tree(next_indent, i == _childs.size() - 1);
    }
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
OrderByNode::OrderByNode(const string& name)
: _name(name)
{
    ;
}

bool OrderByNode::calculate_result()
{
    if (_table_info != NULL)
        return true;

    const vector<QueryNode*>& childs = get_childs();
    assert(childs.size() == 1);

    QueryNode* child = childs[0];
    assert(child != NULL);

    TinyRelation* relation = child->get_or_create_relation();
    if (relation == NULL)
        return false;

    //string new_table_name = "order_by " + relation->get_name();
    //TinyRelation* new_relation = HwMgr::ins()->create_relation(
    //    new_table_name, relation->get_tiny_schema());

    RelSorter sorter(relation, 1, 4);
    sorter.set_attr(_name);
    sorter.get_next();
    //relation->dump();
    //RelScanner scanner(relation, 1, 3);
    //scanner.load_to_mem();
    //scanner.sort(_name);
    //scanner.dump();
    //HwMgr::ins()->dump_memory();
    //while(!scanner.is_end()) {
    //    TinyTuple tuple = scanner.get_next();
    //    if (cond_mgr.is_tuple_match(tuple)) {
    //        new_relation->push_back(tuple);
    //    }
    //}

    //if (relation->is_with_prefix()) {
    //    new_relation->set_with_prefix();
    //}
    set_real_table(relation->get_name());
    //set_tmp_table();

    return true;
}

string OrderByNode::dump_str() const
{
    string res = QueryNode::dump_str();
    res += " ";
    res += _name;
    return res;
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
ProjectNode::ProjectNode(const vector<string>& attr_list)
: _attr_list(attr_list)
{
    ;
}

bool ProjectNode::calculate_result()
{
    if (_table_info != NULL)
        return true;

    const vector<QueryNode*>& childs = get_childs();
    assert(childs.size() == 1);

    QueryNode* child = childs[0];
    assert(child != NULL);

    TinyRelation* relation = child->get_or_create_relation();
    if (relation == NULL)
        return false;
    //assert(relation != NULL);

    vector<pair<string, DataType>> new_attr_type_list;

    //bool is_with_prefix = relation->get_is_with_prefix;
    vector<pair<string, DataType>> attr_type_list = relation->get_attr_type_list();
    for (const string& attr : _attr_list) {
        const string project_name = relation->get_attr_search_name(attr);

        bool is_match = false;
        for (const auto& attr_type : attr_type_list) {
            const string& attr_name = attr_type.first;
            //cout << attr_name << " - " << project_name << endl;
            if (attr_name == project_name) {
                is_match = true;
                new_attr_type_list.push_back(make_pair(attr, attr_type.second)); 
                break;
            }
        }

        if (!is_match) {
            error_msg_not_exist("attribute", attr);
            return false;
        }
    }

    string tmp_table_name = "project " + relation->get_name();
    TinyRelation* new_relation = HwMgr::ins()->create_relation(
        tmp_table_name, new_attr_type_list);
    
    //new_relation->dump();
    RelScanner scanner(relation, 1, 1);
    while(!scanner.is_end()) {
        TinyTuple tuple = scanner.get_next();
        TinyTuple new_tuple = new_relation->create_tuple();
        for (const auto& attr_type : new_attr_type_list) {
            const string& name = attr_type.first;
            const string search_name = relation->get_attr_search_name(name);
            new_tuple.set_value(name, tuple.get_value(search_name));
        }
        new_relation->push_back(new_tuple);
    }

    if (relation->is_with_prefix()) {
        new_relation->set_with_prefix();
    }
    set_real_table(tmp_table_name);
    set_tmp_table();
    return true;
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
WhereNode::WhereNode(tree_node_t* node)
: _where_tree(node)
{
    ;
}

bool WhereNode::calculate_result()
{
    //assert(get_table_info() == NULL);
    //if (get_table_info() != NULL) // TODO
    if (_table_info != NULL)
        return true;

    const vector<QueryNode*>& childs = get_childs();
    assert(childs.size() == 1);

    QueryNode* child = childs[0];
    assert(child != NULL);

    //child->calculate_result();
    //TableInfo* child_table_info = child->get_table_info();
    ////assert(child_table_info != NULL);
    
    //TinyRelation* relation = HwMgr::ins()->get_tiny_relation(child_table_info->get_name());
    TinyRelation* relation = child->get_or_create_relation();
    if (relation == NULL)
        return false;
    //assert(relation != NULL);

    ConditionMgr cond_mgr(_where_tree, relation);
    if (cond_mgr.is_error()) {
        //return false;
        //cout << "ERROR in whereNode" << endl;
        return false;
    }

    string tmp_table_name = "where " + relation->get_name();
    //TinyRelation* new_relation = HwMgr::ins()->create_tmp_relation(tmp_table_name, relation);
    TinyRelation* new_relation = HwMgr::ins()->create_relation(
        tmp_table_name, relation->get_tiny_schema());

    RelScanner scanner(relation, 1, 1);
    while(!scanner.is_end()) {
        TinyTuple tuple = scanner.get_next();
        if (cond_mgr.is_tuple_match(tuple)) {
            new_relation->push_back(tuple);
        }
    }

    if (relation->is_with_prefix()) {
        new_relation->set_with_prefix();
    }
    set_real_table(tmp_table_name);
    set_tmp_table();
    return true;
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
//void CrossProductNode::print_result()
bool CrossProductNode::calculate_result()
{
    const vector<QueryNode*>& childs = get_childs();
    assert(childs.size() == 2); // TODO

    //for (auto& child_ptr : childs) {
    //    child_ptr->calculate_result();
    //    //child_ptr->print_result();
    //}
    //TableInfo* get_table_info() const { return _table_info; }
    //string table_0_name = childs[0]->get_table_info()->get_name();
    //string table_1_name = childs[1]->get_table_info()->get_name();
    //TinyRelation* relation_s = HwMgr::ins()->get_tiny_relation(table_0_name);
    //TinyRelation* relation_l = HwMgr::ins()->get_tiny_relation(table_1_name);
    TinyRelation* relation_s = childs[0]->get_or_create_relation();
    TinyRelation* relation_l = childs[1]->get_or_create_relation();
    if (relation_s == NULL || relation_l == NULL)
        return false;
    //assert(relation_s != NULL);
    //assert(relation_l != NULL);

    TinyRelation* cross_relation = HwMgr::ins()->create_tmp_relation(relation_s, relation_l);
    //cross_relation->dump();

    bool is_swap = false;
    if (relation_s->size() > relation_l->size()) {
        swap(relation_s, relation_l);
        is_swap = true;
    }

    //size_t mem_size = HwMgr::ins()->get_mem_size();
    //if (relation_s->size() < mem_size) {

    //} else {

    //}
    //relation_s->dump();
    //relation_l->dump();
    //RelScanner scanner_l(relation_l, 0, 1);
    ////size_t i = 0;
    //while(!scanner_l.is_end()) {
    //    //relation_l->dump();
    //    //scanner_l.dump();
    //    //HwMgr::ins()->dump_memory();
    //    cout << scanner_l.get_next().dump_str() << endl;

    //    //if (++i == 10)
    //    //    break;
    //}
    size_t total_size = HwMgr::ins()->get_mem_size() - 1;
    size_t l_index = 1;
    size_t l_size = 1;
    size_t s_index = l_index + l_size;
    size_t s_size = total_size - l_size;;

    //return;
    RelScanner scanner_s(relation_s, s_index, s_size);
    while(!scanner_s.is_end()) {
        TinyTuple tuple_s = scanner_s.get_next();
        RelScanner scanner_l(relation_l, l_index, l_size);
        while(!scanner_l.is_end()) {
            TinyTuple tuple_l = scanner_l.get_next();
        
            TinyTuple new_tuple = cross_relation->create_tuple();
            if (is_swap) {
                new_tuple.set_value(tuple_l, tuple_s);
            } else {
                new_tuple.set_value(tuple_s, tuple_l);
            }
            //new_tuple.dump();
            //cout << new_tuple.dump_str() << endl;
            cross_relation->push_back(new_tuple); // TODO: it use mem addr 0
        }
    }

    cross_relation->set_with_prefix();
    set_real_table(cross_relation->get_name());
    set_tmp_table();
    //cross_relation->dump();

    //HwMgr::ins()->dump_memory();
    //for (TinyRelation::iterator it = relation_s->begin(); it != relation_s->end(); ++it) {
    ////    cout << (*it).dump_str() << endl;
    //    //TinyTuple = it.load_to_mem(0);
    //    cout << it.load_to_mem(0).dump_str() << endl;
    //}
    

    //dump_pretty(relation_0);
    //dump_pretty(relation_1);
    //size_t mem_index = 0;
    //size_t num_of_block = relation->get_num_of_block();
    //for (size_t i = 0; i < num_of_block; ++i) {
    //    relation->load_block_to_mem(i, mem_index);
    //    //_relation->getBlock(i, mem_index);
    //    Block* block = HwMgr::ins()->get_mem_block(mem_index);
    //    vector<Tuple> tuples = block->getTuples();
    //    for (const auto& tuple : tuples) {
    //        //dump_normal(TinyTuple(tuple));
    //        table.add_row(TinyTuple(tuple).str_list());
    //    }
    //}
    return true;
}

