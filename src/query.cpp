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

#define SHOW_OPTIMIZE

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
//TableInfo::TableInfo(const string& name)
//: _name(name)
////, _is_in_disk(false)
//, _is_tmp(false)
//{
//    ;
//}
//
//TableInfo::~TableInfo()
//{
//    if (_is_tmp) {
//        HwMgr::ins()->drop_table(_name);
//    }
//}
//
//void TableInfo::print_table() const
//{
//    TinyRelation* relation = HwMgr::ins()->get_tiny_relation(_name);
//    assert(relation != NULL);
//
//    relation->print_table();
//    //DrawTable table(relation->get_num_of_attribute(), DrawTable::MYSQL_TABLE);
//    //vector<DataType> type_list = relation->get_type_list();
//    //for (size_t i = 0; i < type_list.size(); ++i) {
//    //    if (type_list[i] == TINY_INT) {
//    //        table.set_align_right(i);
//    //    }
//    //}
//
//    //table.set_header(relation->get_attr_list());
//
//    //size_t mem_index = 0;
//    //size_t num_of_block = relation->get_num_of_block();
//    ////dump_normal(num_of_block);
//    //for (size_t i = 0; i < num_of_block; ++i) {
//    //    relation->load_block_to_mem(i, mem_index);
//    //    //_relation->getBlock(i, mem_index);
//    //    //Block* block = HwMgr::ins()->get_mem_block(mem_index);
//    //    TinyBlock block = HwMgr::ins()->get_mem_block(mem_index);
//    //    //vector<Tuple> tuples = block->getTuples();
//    //    vector<Tuple> tuples = block.get_tuples();
//    //    for (const auto& tuple : tuples) {
//    //        //dump_normal(TinyTuple(tuple));
//    //        if (!tuple.isNull())
//    //            table.add_row(TinyTuple(tuple).str_list());
//    //    }
//    //}
//
//    //if (table.size() == 0) {
//    //    cout << "Empty set" << endl;
//    //} else {
//    //    table.draw();
//    //    cout << table.size() << " ";
//    //    cout << ((table.size() == 1) ? "row" : "rows");
//    //    cout << " in set";
//    //    cout << endl;
//    //}
//}
//
//void TableInfo::dump() const
//{
//    cout << dump_str() << endl;
//}
//
//string TableInfo::dump_str() const
//{
//    return _name;
//}

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
    //assert(node_is_where(node));

    tree_node_t* child = node;
    if(node_is_where(node)) {
        child = node->child;
    }
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

    string field_name = _tiny_relation->get_attr_search_name(column_name);;

    //if (table.empty()) {
    //    if (_tiny_relation->is_with_prefix()) {
    //        error_msg("ambiguous \'" + column + "\'");
    //        return false;
    //        //field_name = table + "." + column;
    //        //field_name = column_name;
    //        //field_name = column;
    //    } else {

    //        field_name = column;
    //    }
    //} else { // !table.empty()
    //    if (_tiny_relation->is_with_prefix()) {
    //        //field_name = table + "." + column;
    //        field_name = column_name;
    //    } else {
    //        if (table != _tiny_relation->get_base_name()) {
    //            error_msg_not_exist("attribute", column_name);
    //            return false;
    //        }

    //        field_name = column;
    //    }

    //}
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
    //HwMgr::ins()->delete_tmp_table();
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
        //cout << "TODO: SELECT in INSERT" << endl; // TODO
        bool res = insert_into_from_select(table_name, name_list, tuples_node);
        return res;
    }   

    assert(node_is_value_list(tuples_node));

    vector<string> value_list = get_string_list(tuples_node);
    if (name_list.size() != value_list.size()) {
        //error_msg("size of attributes and data");
        error_msg("size of attributes and data should be the same");
        //if (name_list.size() > value_list.size()) {
        //    error_msg("missing data");
        //} else {
        //    error_msg("missing a");
        //}
        return false;
    }

    //dump_pretty(value_list);
    vector<pair<string, string>> data = vector_make_pair(name_list, value_list);
    //dump_pretty(data);
    bool res = HwMgr::ins()->insert_into(table_name, data);
    return res;
}

bool QueryMgr::insert_into_from_select(const string& table_name, const vector<string>& attr_list, tree_node_t* node)
{
    assert(node != NULL);
    assert(node_is_select(node));

    TinyRelation* to_relation = HwMgr::ins()->get_tiny_relation(table_name);
    if (to_relation == NULL) {
        error_msg_table_not_exist(table_name);
        return false;
    }
    
    bool is_ok = build_select_tree(node);
    if (!is_ok) {
        return false;
    }

    if (_select_root == NULL) // TODO: remove
        return false;

    TinyRelation* from_relation = _select_root->get_or_create_relation(); // TODO
    if (from_relation == NULL)
        return false;

    RelScanner scanner(from_relation, 1, 1);
    while(!scanner.is_end()) {
        TinyTuple tuple = scanner.get_next();
        assert(!tuple.is_null());

        to_relation->push_back(tuple);
    }
    
    return true;
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

#ifdef SHOW_OPTIMIZE
    dump_pretty(_select_root);
#endif
    optimize_select_tree();
#ifdef SHOW_OPTIMIZE
    dump_pretty(_select_root);
#endif
    //QueryNode* optimized_tree =  QueryMgr::optimize_select_tree(_select_root);
    //QueryNode* optimized_tree =  QueryMgr::optimize_select_tree(_select_root);
    //dump_pretty(optimized_tree);


    //return true;
    bool res = _select_root->print_result();

    return res;
}

void QueryMgr::optimize_select_tree()
{
    QueryNode* node = _select_root;
    assert(node != NULL);

    QueryNode* cross_node = node->get_node(QueryNode::CROSS_PRODUCT);
    //if (node->has_node(QueryNode::CROSS_PRODUCT)) {
    if (cross_node != NULL) {
        if (!node->has_node(QueryNode::WHERE)) {
            optimize_cross_product(cross_node);
        } else {
            QueryNode* where_node = node->get_node(QueryNode::WHERE);
            optimize_where_with_cross_product(where_node);
            // TODO: natural join
            //optimize_cross_product(cross_node);
            //vector 
        }
    }
}

void QueryMgr::optimize_cross_product(QueryNode* node)
{
    assert(node != NULL);
    assert(node->get_type() == QueryNode::CROSS_PRODUCT);

    //cout << "yo" << endl;
    //if 
    CrossProductNode* cross_node = dynamic_cast<CrossProductNode*>(node);
    assert(cross_node != NULL);

    cross_node->split(); // TODO
}

void QueryMgr::optimize_where_with_cross_product(QueryNode* node)
{
    assert(node != NULL);
    assert(node->get_type() == QueryNode::WHERE);

    WhereNode* where_node = dynamic_cast<WhereNode*>(node);
    assert(where_node != NULL);

    //cout << "tata" << endl;
    vector<tree_node_t*> and_equal_list = where_node->split_and_equal();
    //if (and_node_list.empty()) {
    //    return;
    //}
    //vector<tree_node_t*> single_table;
    for (const auto& and_equal : and_equal_list) {
        if (node_is_comp_op(and_equal)) {
            string comp_type = and_equal->value;
            if (comp_type == "=") {
                tree_node_t* child_left = and_equal->child;
                assert(child_left != NULL);

                tree_node_t* child_right = child_left->next;
                assert(child_right != NULL);

                if (node_is_column_name(child_left) &&
                    node_is_column_name(child_right)
                ) {
                    vector<string> attr_list;
                    attr_list.push_back(child_left->value);
                    attr_list.push_back(child_right->value);
                    create_natural_join(attr_list);
                    continue; // TODO
                }
            }
        }


        vector<string> all_table = get_all_table(and_equal);
        //dump_pretty(all_table);
        if (all_table.size() == 1) {
            insert_where(all_table[0], and_equal);
        }
    }
}

void QueryMgr::create_natural_join(const vector<string>& attr_list)
{
    assert(attr_list.size() == 2);

    QueryNode* cross_node = _select_root->get_node(QueryNode::CROSS_PRODUCT);
    if (cross_node == NULL)
        return;
    //assert(cross_node != NULL);

    vector<string> table_list;
    for (const string& attr : attr_list) {
        table_list.push_back( ColumnName(attr).get_table() );
    }

    vector<QueryNode*> natural;
    const vector<QueryNode*>& childs = cross_node->get_childs();
    for (size_t i = 0; i < childs.size(); ++i) {
        vector<string> base_table_name = childs[i]->get_base_table_name();
        //string base_name = childs[i]->get_base_name();
        //if (is_contain(table_list, base_name)) {
        if (is_contain(table_list, base_table_name)) {
            natural.push_back(childs[i]);
        }
    }

    if (natural.size() == 2) {
        QueryNode* new_node = new NaturalJoinNode(attr_list);
        new_node->add_child(natural[0]); 
        new_node->add_child(natural[1]); 

        remove_from(cross_node->_childs, natural[0]);
        remove_from(cross_node->_childs, natural[1]);

        if (cross_node->_childs.empty()) {
            QueryNode* parent = cross_node->_parent;
            if (parent != NULL) {
                replace_all(parent->_childs, cross_node, new_node);
            }  else {
                assert(_select_root == cross_node);
                _select_root = new_node;
            }

            delete cross_node;
            //cout << "TODO delete cross" << endl;
            //delete
        } else {
            cross_node->add_child(new_node);
        }
    }
}

void QueryMgr::insert_where(const string& table_name, tree_node_t* where_tree)
{
    assert(!table_name.empty());
    assert(where_tree != NULL);

    //dump_normal(table_name);

    vector<string> tmp;
    tmp.push_back(table_name);
    QueryNode* node = _select_root->get_node_by_base_table(tmp);
    assert(node != NULL); 

    QueryNode* new_node = build_where(where_tree);
    replace_all(node->_parent->_childs, node, new_node);
    new_node->add_child(node);
}

vector<string> QueryMgr::get_all_table(tree_node_t* node) const
{
    assert(node != NULL);

    vector<string> res;

    if (node_is_column_name(node)) {
        ColumnName cn(node->value);    
        assert(!cn.get_table().empty());
        add_into(res, cn.get_table());
    }

    tree_node_t* child = node->child;
    while (child != NULL) {
        add_into(res, get_all_table(child));
        child = child->next;
    }

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

    if (!order_by.empty()) {
        // TODO: build fail; return false;
        QueryNode* order_by_node = build_order_by(order_by);
        order_by_node->add_child(_select_root);
        _select_root = order_by_node;
    }

    if (!attr_list.empty()) {
        // TODO: build fail; return false;
        QueryNode* project_node = build_project(attr_list);
        project_node->add_child(_select_root);
        _select_root = project_node;
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
    //node_ptr->set_real_table(table_name, relation);
    node_ptr->set_relation(relation);
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
: _relation(NULL)
, _delete_relation(false)
, _parent(NULL)
//, _table_info(NULL)
{
    ;
}
QueryNode::~QueryNode()
{
    //cout << _relation->get_name() << endl;
    if (_delete_relation) {
        assert(_relation != NULL);
        //assert(_relation->is_tmp());
        HwMgr::ins()->drop_table(_relation->get_name());
        //HwMgr::ins()->drop_table(_name);
        //delete_not_null(_relation);
    }
    //delete_not_null(_table_info);
    delete_all(_childs);
}

void QueryNode::add_child(QueryNode* child)
{
    assert(child != NULL);

    child->_parent = this;
    _childs.push_back(child);
}

void QueryNode::set_tmp_table(TinyRelation* r)
{
    //assert(_table_info != NULL);
    //_table_info->set_is_tmp();

    assert(_relation != NULL);
    //assert(_relation->get_name() == _table_info->get_name());
    //r->set_is_tmp();
    _delete_relation = true;
}

//void QueryNode::set_real_table(const string& name, TinyRelation* r)
void QueryNode::set_relation(TinyRelation* r)
{
    //delete_not_null(_table_info);
    //_table_info = new TableInfo(name);
    //_table_info->set_is_in_disk();
    _relation = r;
}

bool QueryNode::print_result()
{
    //assert(_table_info != NULL);
    //if (_table_info == NULL) {
    if (_relation == NULL) {
        if(!calculate_result())
            return false;
    }
    
    //assert(_table_info != NULL);
    assert(_relation != NULL);
    //_table_info->print_table(); 
    _relation->print_table(); 
    return true;
}

bool QueryNode::calculate_result()
{
    //assert(_table_info != NULL);
    assert(_relation != NULL);
    // do nothing
    return true;
}

TinyRelation* QueryNode::get_or_create_relation()
{
    if (_relation == NULL) {
        if (!calculate_result())
            return NULL;
    }
    
    assert(_relation != NULL);
    return _relation;
}

bool QueryNode::has_node(NodeType type) const
{
    if (get_type() == type)
        return true;

    for (const auto& child_ptr : _childs) {
        if (child_ptr->has_node(type))
            return true;
    }

    return false;
}

QueryNode* QueryNode::get_node(NodeType type)
{
    if (get_type() == type)
        return this;

    for (const auto& child_ptr : _childs) {
        QueryNode* tmp = child_ptr->get_node(type);
        if (tmp != NULL)
            return tmp;
    }

    return NULL;
}

QueryNode* QueryNode::get_node_by_base_table(const vector<string>& table_list)
{
    vector<string> sorted_table = table_list;
    std::sort(sorted_table.begin(), sorted_table.end());
    if (get_base_table_name() == sorted_table) {
        return this;
    }

    for (const auto& child_ptr : _childs) {
        QueryNode* tmp = child_ptr->get_node_by_base_table(table_list);
        if (tmp != NULL)
            return tmp;
    }

    return NULL;
}

vector<string> QueryNode::get_base_table_name() const // TODO: store
{
    vector<string> res;
    if (_relation != NULL) {
        res.push_back(_relation->get_name());
    }

    for (const auto& child_ptr : _childs) {
        add_into(res, child_ptr->get_base_table_name());
    }

    std::sort(res.begin(), res.end());
    return res;
}

string QueryNode::get_base_name() const
{
    if (_relation != NULL) {
        return _relation->get_name();
    }

    string res;
    for (const auto& child_ptr : _childs) {
        if (!res.empty()) {
            res += " ";
        }

        res += child_ptr->get_base_name();
    }

    return res;
}
//bool QueryNode::has_cross_product() const
//{
//    if (get_type() == CROSS_PRODUCT)
//        return true;
//
//
//    return false;
//}
//
//bool QueryNode::has_where() const
//{
//}

#define NODE_TYPE(name) case name: return #name
string QueryNode::get_type_str(NodeType t) const
{
    switch (t) {
        NODE_TYPE(DISTINCT);
        NODE_TYPE(CROSS_PRODUCT);
        NODE_TYPE(NATURAL_JOIN);
        NODE_TYPE(WHERE);
        NODE_TYPE(PROJECT);
        NODE_TYPE(ORDER_BY);
        NODE_TYPE(BASE_NODE);
    }

    return "ERROR";
}

void QueryNode::dump() const
{
    dump_tree("", true);
}

string QueryNode::dump_str() const
{
    //string tmp = "{";
    string tmp;
    tmp += get_type_str(get_type());
    tmp += ": ";
    //tmp += jjjj222::dump_str(_table_info);
    tmp += ((_relation == NULL) ? ("<NULL>") : (_relation->get_name()));
    //tmp += "}";
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
bool DistinctNode::calculate_result()
{
    //if (_table_info != NULL)
    if (_relation != NULL)
        return true;

    const vector<QueryNode*>& childs = get_childs();
    assert(childs.size() == 1);

    QueryNode* child = childs[0];
    assert(child != NULL);

    TinyRelation* relation = child->get_or_create_relation();
    if (relation == NULL)
        return false;

    size_t total_size = HwMgr::ins()->get_mem_size() - 1;
    size_t base_index = 1;
    size_t mem_size = total_size;
    RelSorter sorter(relation, base_index, mem_size); // TODO: skip sort if sorted
    sorter.sort();

    string new_table_name = "distinct " + relation->get_name();
    TinyRelation* new_relation = HwMgr::ins()->create_relation(
        new_table_name, relation->get_tiny_schema());

    RelScanner scanner(relation, 1, 1);
    TinyTuple previous_tuple = new_relation->create_null_tuple();
    while(!scanner.is_end()) {
        TinyTuple tuple = scanner.get_next();
        assert(!tuple.is_null());

        if (previous_tuple.is_null()) {
            previous_tuple = tuple;
            continue;
        }

        if (tuple == previous_tuple) {
            continue;
        }

        new_relation->push_back(previous_tuple);
        previous_tuple = tuple;
    }
    new_relation->push_back(previous_tuple);

    if (relation->is_with_prefix()) {
        new_relation->set_with_prefix();
    }
    //set_real_table(new_table_name, new_relation);
    set_relation(new_relation);
    set_tmp_table(new_relation);

    return true;
}

string DistinctNode::dump_str() const
{
    return "DISTINCT";
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
    //if (_table_info != NULL)
    if (_relation != NULL)
        return true;

    const vector<QueryNode*>& childs = get_childs();
    assert(childs.size() == 1);

    QueryNode* child = childs[0];
    assert(child != NULL);

    TinyRelation* relation = child->get_or_create_relation();
    if (relation == NULL)
        return false;

    if (!relation->is_attr_exist(_name)) {
        error_msg_not_exist("attribute", _name);
        return false;
    }
    //string new_table_name = "order_by " + relation->get_name();
    //TinyRelation* new_relation = HwMgr::ins()->create_relation(
    //    new_table_name, relation->get_tiny_schema());

    size_t total_size = HwMgr::ins()->get_mem_size() - 1;
    size_t base_index = 1;
    size_t mem_size = total_size;
    //RelSorter sorter(relation, 1, 3); // TODO
    RelSorter sorter(relation, base_index, mem_size); // TODO
    //sorter.set_attr(_name);
    //sorter.sort();
    sorter.sort(_name);

    //set_real_table(relation->get_name(), relation); // TODO: remove
    set_relation(relation);

    return true;
}

string OrderByNode::dump_str() const
{
    string res = "ORDER_BY: ";
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
    //if (_table_info != NULL)
    if (_relation != NULL)
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
    //set_real_table(tmp_table_name, new_relation);
    set_relation(new_relation);
    set_tmp_table(new_relation);
    return true;
}

string ProjectNode::dump_str() const
{
    string res = "PROJECTION: ";
    res += tiny_dump_str(_attr_list);
    return res;
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
    //if (_table_info != NULL)
    if (_relation != NULL)
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
    //set_real_table(tmp_table_name, new_relation);
    set_relation(new_relation);
    set_tmp_table(new_relation);
    return true;
}

vector<tree_node_t*> WhereNode::split_and_equal() const
{
    assert(_where_tree != NULL);
    assert(node_is_where(_where_tree));

    tree_node_t* child = _where_tree->child;
    assert(child != NULL);

    vector<tree_node_t*> res;

    if (node_is_and(child)) {
        child = child->child;
    } else if (node_is_comp_op(child)) {
        // do nothing
    } else {
        return res;
    }

    while (child != NULL) {
        //if (node_is_comp_op(child)) {
        //    string comp_type = child->value;
        //    if (comp_type == "=") {
        //        res.push_back(child);
        //    } 
        //}
        res.push_back(child);
        //ConditionNode* tmp = build_condition_node_rec(child);
        //new_node->add_child(tmp);
        child = child->next;
    }

    return res;
}


string WhereNode::dump_str() const
{
    return "WHERE";
}

void WhereNode::dump_tree(const string& indent, bool is_last) const
{
    const string current_indent = indent + (is_last ? " `- " : " |- ");
    cout << current_indent << dump_str() << endl;

    const string next_indent = indent + (is_last ? "    " : " |  ");
    dump_tree_node_indent(_where_tree, next_indent.c_str());

    const vector<QueryNode*>& childs = get_childs();
    for (size_t i = 0; i < childs.size(); ++i) {
        const QueryNode* node_ptr = childs[i];
        node_ptr->dump_tree(next_indent, i == childs.size() - 1);
    }
}
//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
bool CrossProductNode::calculate_result()
{
    const vector<QueryNode*>& childs = get_childs();
    assert(childs.size() == 2); // TODO

    TinyRelation* relation_s = childs[0]->get_or_create_relation();
    TinyRelation* relation_l = childs[1]->get_or_create_relation();
    if (relation_s == NULL || relation_l == NULL)
        return false;

    TinyRelation* cross_relation = HwMgr::ins()->create_tmp_relation(relation_s, relation_l);
    cross_relation->set_with_prefix();
    set_relation(cross_relation);
    set_tmp_table(cross_relation);
    //cross_relation->dump();

    bool is_swap = false;
    if (relation_s->size() > relation_l->size()) {
        swap(relation_s, relation_l);
        is_swap = true;
    }


    MemRange total_range = HwMgr::ins()->get_mem_range().get_not_first_block();
    MemRange l_range = total_range.get_first_block();
    MemRange s_range = total_range.get_not_first_block();
    //dump_normal(total_range);
    //size_t total_size = HwMgr::ins()->get_mem_size() - 1;
    //size_t l_index = 1;
    //size_t l_size = 1;
    //size_t s_index = l_index + l_size;
    //size_t s_size = total_size - l_size;;

    //RelScanner scanner_s(relation_s, s_index, s_size);
    //RelScanner scanner_l(relation_l, l_index, l_size);
    RelScanner scanner_s(relation_s, s_range);
    RelScanner scanner_l(relation_l, l_range);
    do_cross_product(scanner_s, scanner_l, is_swap);

    //RelScanner scanner_s(relation_s, s_index, s_size);
    //while(!scanner_s.is_end()) {
    //    TinyTuple tuple_s = scanner_s.get_next();
    //    RelScanner scanner_l(relation_l, l_index, l_size);
    //    while(!scanner_l.is_end()) {
    //        TinyTuple tuple_l = scanner_l.get_next();
    //        //cout << scanner_s.dump_str() << " | " << scanner_l.dump_str() << endl;
    //    
    //        TinyTuple new_tuple = cross_relation->create_tuple();
    //        if (is_swap) {
    //            new_tuple.set_value(tuple_l, tuple_s);
    //        } else {
    //            new_tuple.set_value(tuple_s, tuple_l);
    //        }
    //        //new_tuple.dump();
    //        //cout << new_tuple.dump_str() << endl;
    //        cross_relation->push_back(new_tuple); // TODO: it use mem addr 0
    //    }
    //}

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

void CrossProductNode::do_cross_product(const RelScanner& scanner_s, const RelScanner& scanner_l, bool is_swap)
{
    assert(_relation != NULL);

    RelScanner scanner_i = scanner_s;
    while(!scanner_i.is_end()) {
        TinyTuple tuple_s = scanner_i.get_next();
        RelScanner scanner_j = scanner_l;
        while(!scanner_j.is_end()) {
            TinyTuple tuple_l = scanner_j.get_next();
        
            TinyTuple new_tuple = _relation->create_tuple();
            if (is_swap) {
                new_tuple.set_value(tuple_l, tuple_s);
            } else {
                new_tuple.set_value(tuple_s, tuple_l);
            }

            _relation->push_back(new_tuple); // TODO: it use mem addr 0
        }
    }
}

void CrossProductNode::split()
{
    while (_childs.size() > 2) {
        QueryNode* child_r = _childs.back();
        _childs.pop_back();
        QueryNode* child_l = _childs.back();
        _childs.pop_back();
        QueryNode* new_child= new CrossProductNode();
        new_child->add_child(child_l); 
        new_child->add_child(child_r); 
        //_childs.push_back(new_child); // TODO
        add_child(new_child);
    }
}

//void CrossProductNode::convert_to_natural(const vector<string>& attr_list)
//{
//    assert(attr_list.size() == 2);
//
//    //dump_pretty(attr_list);
//    vector<string> table_list;
//    for (const string& attr : attr_list) {
//        table_list.push_back( ColumnName(attr).get_table() );
//    }
//
//    vector<QueryNode*> natural;
//    for (size_t i = 0; i < _childs.size(); ++i) {
//        string base_name = _childs[i]->get_base_name();
//        //dump_normal(base_name);
//        if (is_contain(table_list, base_name)) {
//            natural.push_back(_childs[i]);
//            //cout << "yes" << endl;
//        }
//    }
//
//    if (natural.size() == 2) {
//        QueryNode* new_node = new NaturalJoinNode(attr_list);
//        new_node->add_child(natural[0]); 
//        new_node->add_child(natural[1]); 
//
//        remove_from(_childs, natural[0]);
//        remove_from(_childs, natural[1]);
//
//        if (_childs.empty()) {
//            //cout << "TODO delete cross" << endl;
//            //delete
//        } else {
//            add_child(new_node);
//        }
//    }
//}

string CrossProductNode::dump_str() const
{
    return "CROSS_PRODUCT";
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
NaturalJoinNode::NaturalJoinNode(const vector<string>& attr_list)
: _attr_list(attr_list)
{
    ;
}

bool NaturalJoinNode::calculate_result()
{
    const vector<QueryNode*>& childs = get_childs();
    assert(childs.size() == 2);

    TinyRelation* relation_0 = childs[0]->get_or_create_relation();
    TinyRelation* relation_1 = childs[1]->get_or_create_relation();
    if (relation_0 == NULL || relation_1 == NULL)
        return false;

    TinyRelation* join_relation = HwMgr::ins()->create_tmp_relation(relation_0, relation_1);
    join_relation->set_with_prefix();
    set_relation(join_relation);
    set_tmp_table(join_relation);

    //bool is_swap = false;
    //if (relation_s->size() > relation_l->size()) {
    //    swap(relation_s, relation_l);
    //    is_swap = true;
    //}

    string attr_0 = _attr_list[0];
    string attr_1 = _attr_list[1];
    if (!relation_0->is_attr_exist(attr_0)) {
        swap(attr_0, attr_1);
    }
    assert(relation_0->is_attr_exist(attr_0));
    assert(relation_1->is_attr_exist(attr_1));


    size_t total_size = HwMgr::ins()->get_mem_size() - 1;
    size_t base_index = 1;
    size_t mem_size = total_size;
    RelSorter sorter_s(relation_0, base_index, mem_size);
    sorter_s.sort(attr_0);
    RelSorter sorter_l(relation_1, base_index, mem_size);
    sorter_l.sort(attr_1);

    //vector<pair<DataValue, pair<RelIter, RelIter>>> sub_list_0 =
    //    relation_0->get_sub_list_by_attr(attr_0, 1);
    vector<pair<DataValue, RelRange>> sub_list_0 =
        relation_0->get_sub_list_by_attr(attr_0, 1);
    //vector<pair<DataValue, pair<RelIter, RelIter>>> sub_list_1 =
    //    relation_1->get_sub_list_by_attr(attr_1, 1);
    vector<pair<DataValue, RelRange>> sub_list_1 =
        relation_1->get_sub_list_by_attr(attr_1, 1);
    //dump_pretty(sub_list_0);
    //size_t l_index = 1;
    //size_t l_size = 1;
    //size_t s_index = l_index + l_size;
    //size_t s_size = total_size - l_size;;
    //dump_normal(is_swap);
    //size_t total_size = HwMgr::ins()->get_mem_size() - 1;
    //size_t l_index = 1;
    //size_t l_size = 1;
    //size_t s_index = l_index + l_size;
    //size_t s_size = total_size - l_size;;
    MemRange total_range = HwMgr::ins()->get_mem_range().get_not_first_block();
    MemRange l_range = total_range.get_first_block();
    MemRange s_range = total_range.get_not_first_block();

    size_t i_0 = 0;
    size_t i_1 = 0;
    while (i_0 < sub_list_0.size() && i_1 < sub_list_1.size()) {
        const auto& value_iter_0 = sub_list_0[i_0];
        const auto& value_iter_1 = sub_list_1[i_1];

        const auto& value_0 = value_iter_0.first;
        const auto& value_1 = value_iter_1.first;

        if (value_0 == value_1) {
            const auto& range_0 = value_iter_0.second;
            const auto& range_1 = value_iter_1.second;
            //size_t index_diff_0 = iter_0.second.get_block_idx() - iter_0.first.get_block_idx();
            size_t size_0 = range_0.num_of_block();
            size_t size_1 = range_1.num_of_block();

            //size_t index_0, size_0, index_1, size_1;
            MemRange mem_range_0, mem_range_1;
            bool is_swap = false;
            if (size_0 < size_1) {
                mem_range_0 = s_range;
                mem_range_1 = l_range;
            //    index_0 = s_index; 
            //    size_0 = s_size;
            //    index_1 = l_index;
            //    size_1 = l_size;
            } else {
                mem_range_0 = l_range;
                mem_range_1 = s_range;
                is_swap = true;
            //    index_0 = s_index; 
            //    size_0 = s_size;
            //    index_1 = l_index;
            //    size_1 = l_size;
            }
            RelScanner scanner_0(relation_0, mem_range_0);
            scanner_0.set_range(range_0);
            //scanner_0.set_begin(iter_0.first);
            //scanner_0.set_end(iter_0.second);
            RelScanner scanner_1(relation_1, mem_range_1);
            scanner_1.set_range(range_1);

            if (is_swap) {
                do_cross_product(scanner_1, scanner_0, is_swap);
            } else {
                do_cross_product(scanner_0, scanner_1, is_swap);
            }
            //RelScanner scanner_1(relation_1, index_1, size_1);
            //scanner_1.set_begin(iter_1.first);
            //scanner_1.set_end(iter_1.second);
            //    //RelScanner scanner_1(relation_1, l_index, l_size);
            //    //do_cross_product(scanner_0, scanner_1, false);        
            //if (index_diff_0 < index_diff_1) {
            //} else {
            //}
            //dump_normal(value_0);
            //RelScanner
            ++i_0;
            ++i_1;
        } else if (value_0 < value_1) {
            ++i_0;
        } else { // value_0 > value_1
            ++i_1;
        }
    }

    

    //RelScanner scanner_s(relation_s, s_index, s_size);
    //RelScanner scanner_l(relation_l, l_index, l_size);
    //TinyTuple tuple_s = relation_s->create_null_tuple();
    //TinyTuple tuple_l = relation_l->create_null_tuple();
    //while(!scanner_s.is_end() && !scanner_l.is_end()) {
    //    if (tuple_s.is_null()) {
    //        tuple_s = scanner_s.get_next();
    //    }

    //    if (tuple_l.is_null()) {
    //        tuple_l = scanner_l.get_next();
    //    }

    //}

    //    TinyTuple tuple_s = scanner_s.get_next();
    //    RelScanner scanner_l(relation_l, l_index, l_size);
    //    while(!scanner_l.is_end()) {
    //        TinyTuple tuple_l = scanner_l.get_next();
    //    
    //        TinyTuple new_tuple = cross_relation->create_tuple();
    //        if (is_swap) {
    //            new_tuple.set_value(tuple_l, tuple_s);
    //        } else {
    //            new_tuple.set_value(tuple_s, tuple_l);
    //        }

    //        cross_relation->push_back(new_tuple); // TODO: it use mem addr 0
    //    }
    //}

    return true;
}

string NaturalJoinNode::dump_str() const
{
    string res = "NATURAL_JOIN: ";
    res += tiny_dump_str(_attr_list);
    return res;
}
