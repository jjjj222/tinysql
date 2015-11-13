#include <iostream>
#include <cassert>

using std::cout;
using std::endl;

#include "obj_util.h"
#include "debug.h"

using namespace jjjj222;

#include "dbMgr.h"
#include "parser.h"

bool QueryMgr::exec_query(const string& query)
{
    SqlParser parser;
    tree_node_t* root = parser.parse_string(query);
    parser.dump();
    if (parser.is_error())
        return false;


    if (root == NULL)
        return true;

    bool error = false;
    assert(root->next == NULL);

    if (node_is_create_table(root)) {
        if (!create_table(root))
            error = true;
    } else if (node_is_insert(root)) {
        if (!insert_into(root))
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

bool QueryMgr::select_from(tree_node_t* node)
{
    assert(node != NULL);
    assert(node_is_select(node));

    tree_node_t* child = node->child;
    assert(child != NULL);

    bool is_distinct = false;
    if (node_is_distinct(child)) {
        is_distinct = true;
        child = child->next;
        assert(child != NULL);
    }

    vector<string> attr_list;
    if (child->type == '*') {

    } else {
        assert(node_is_select_list(child));
        cout << "TODO: SELECT_LIST" << endl;
    }

    tree_node_t* name_list_node = child->next;
    assert(name_list_node != NULL);
    vector<string> name_list = get_string_list(name_list_node);

    child = name_list_node->next;
    tree_node_t* where_tree = NULL;
    if (child != NULL && node_is_where(child)) {
        where_tree = child;
        child = child->next;
    }

    string order_by = "";
    if (child != NULL) {
        assert(node_is_order_by(child));
        assert(child->child != NULL);
        order_by = child->child->value;
        //dump_normal(order_by);
    }
    //dump_normal(name_list);
    //while (child != NULL) {
    //    //attr_list.push_back(get_name_type(child));
    //    child = child->next;
    //}    
    bool res = HwMgr::ins()->select_from(name_list, attr_list, where_tree, is_distinct);
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
