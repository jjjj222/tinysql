#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>

#include "parser.h"

#include "y.tab.h"
#include "lex.yy.h"

//------------------------------------------------------------------------------
//   global variable
//------------------------------------------------------------------------------
int parser_error = 0;
int parser_file_lineno = 0;
const char* parser_file_name = NULL;
str_list_t* name_list = NULL;
tree_node_list_t* node_list = NULL;
tree_node_t* query_list = NULL;
tree_node_t* query_list_last = NULL;

//------------------------------------------------------------------------------
//   parse tree
//------------------------------------------------------------------------------
tree_node_t* tree_add_child_front(tree_node_t* node, tree_node_t* child)
{
    assert(node != NULL);
    assert(child != NULL);

    child->next = node->child;
    node->child = child;

    return node;
}

tree_node_t* new_tree_node(int type, const char* value)
{
    tree_node_t* node = malloc(sizeof(tree_node_t));
    node->type = type;
    node->value = value;
    node->next = NULL;
    node->child = NULL;

    tree_node_list_t* tmp = new_tree_node_list(node);
    if (node_list == NULL) {
        node_list = tmp;
    } else {
        tmp->next = node_list;
        node_list = tmp;
    }
    return node;
}

tree_node_t* new_tree_node_0(int type)
{
    return new_tree_node(type, NULL);
}

tree_node_t* new_tree_node_1(int type, tree_node_t* child)
{
    assert(child != NULL);

    tree_node_t* tmp = new_tree_node_0(type);
    tree_add_child_front(tmp, child);

    return tmp;
}

tree_node_t* new_tree_node_2(int type, tree_node_t* child1, tree_node_t* child2)
{
    assert(child1 != NULL);
    assert(child2 != NULL);

    tree_node_t* tmp = new_tree_node_1(type, child2);
    tree_add_child_front(tmp, child1);

    return tmp;
}

tree_node_t* new_tree_node_3(int type, tree_node_t* child1, tree_node_t* child2, tree_node_t* child3)
{
    assert(child1 != NULL);
    assert(child2 != NULL);
    assert(child3 != NULL);

    tree_node_t* tmp = new_tree_node_2(type, child2, child3);
    tree_add_child_front(tmp, child1);

    return tmp;
}

tree_node_t* new_tree_node_n(int type, int nops, ...)
{
    va_list ap;

    tree_node_t* top = new_tree_node_0(type);
    tree_node_t* last = NULL;

    va_start(ap, nops);
    for (size_t i = 0;  i < nops; ++i) {
        tree_node_t* tmp = va_arg(ap, tree_node_t*);
        if (tmp == NULL)
            continue;

        if (last == NULL) {
            tree_add_child_front(top, tmp);
            last = tmp;
        } else {
            last->next = tmp;
            last = tmp;
        }
    }
    va_end(ap);

    return top;
}

void free_parse_tree(tree_node_t* node)
{
    if (node == NULL)
        return;
    
    free_parse_tree(node->child);
    free_parse_tree(node->next);
    free(node);
}

void dump_tree_node_rec(const tree_node_t* node, const char* indent, bool is_last)
{
    const char* current_indent = is_last ? " `- " : " |- ";
    printf("%s%s%s%s%s\n",
        indent,
        current_indent,
        get_lex_macro_str(node->type),
        node->value ? ": ": "",
        node->value ? node->value : ""
    ); 

    const char* indent_one = is_last ? "    " : " |  ";
    size_t indent_one_len = strlen(indent_one);

    size_t indent_len = strlen(indent);
    char* next_indent = malloc(sizeof(char) * (indent_len + indent_one_len + 1));
    memcpy(next_indent, indent, indent_len);
    memcpy(next_indent + indent_len, indent_one, indent_one_len + 1);

    const tree_node_t* child = node->child;
    while (child != NULL) {
        dump_tree_node_rec(child, next_indent, child->next == NULL);
        child = child->next;
    }
    free(next_indent);
}

void dump_tree_node(const tree_node_t* node)
{
    dump_tree_node_rec(node, "", true);
}

void dump_tree_node_indent(const tree_node_t* node, const char* indent)
{
    dump_tree_node_rec(node, indent, false);
}

void add_to_query_list(tree_node_t* node)
{
    if (query_list_last == NULL) {
        query_list = query_list_last = node;
    } else {
        query_list_last->next = node;
        query_list_last = node;
    }
}

void dump_query_list()
{
    tree_node_t* tmp = query_list;

    while (tmp != NULL) {
        dump_tree_node(tmp);
        tmp = tmp->next;
    }
}

void dump_node_list()
{
    dump_tree_node_list(node_list);
}

tree_node_list_t* new_tree_node_list(tree_node_t* node)
{
    tree_node_list_t* list_node = malloc(sizeof(tree_node_list_t));
    list_node->next = NULL;
    list_node->node = node;

    return list_node;
}

void free_tree_node_list(tree_node_list_t* list)
{
    if (list == NULL)
        return;

    free_tree_node_list(list->next);
    free(list->node);
    free(list);
}

void dump_tree_node_list(tree_node_list_t* list)
{
    if (list == NULL)
        return;

    tree_node_list_t* tmp = list;
    while (tmp != NULL) {
        dump_tree_node(tmp->node);
        tmp = tmp->next;
    }
}


//------------------------------------------------------------------------------
//   node type
//------------------------------------------------------------------------------
int node_is_create_table(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == CREATE_TABLE_STATEMENT;
}

int node_is_drop_table(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == DROP_TABLE_STATEMENT;
}

int node_is_insert(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == INSERT_STATEMENT;
}

int node_is_delete(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == DELETE_STATEMENT;
}

int node_is_select(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == SELECT_STATEMENT;
}

int node_is_attribute_type_list(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == ATTRIBUTE_TYPE_LIST;
}

int node_is_name_type(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == NAME_TYPE;
}

int node_is_name_list(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == NAME_LIST;
}

int node_is_value_list(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == VALUE_LIST;
}

int node_is_select_list(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == SELECT_LIST;
}

int node_is_where(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == WHERE_OPTION;
}

int node_is_order_by(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == ORDER_OPTION;
}

int node_is_int(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == INT;
}

int node_is_str20(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == STR20;
}

int node_is_distinct(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == DISTINCT;
}

int node_is_null(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == NULL_VALUE;
}

int node_is_or(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == OR;
}

int node_is_and(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == AND;
}

int node_is_not(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == NOT;
}

int node_is_comp_op(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == COMP_OP;
}

int node_is_arith_op(tree_node_t* node)
{
    assert(node != NULL);
    
    int type = node->type;
    return type == '+' || type == '-' || type == '*' || type == '/';
}

int node_is_name(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == NAME;
}

int node_is_column_name(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == COLUMN_NAME;
}

int node_is_literal(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == LITERAL;
}

int node_is_integer(tree_node_t* node)
{
    assert(node != NULL);
    return node->type == INTEGER;
}
//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
const char* lookup_symbol(const char* name)
{
    assert(name != NULL);

    str_list_t* list_node = new_str_list(name);

    if (name_list == NULL) {
        name_list = list_node;
    } else {
        list_node->next = name_list;
        name_list = list_node;
    }

    return list_node->str;
}

void dump_name_list()
{
    dump_str_list(name_list);
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
tree_node_t* parse_sql_file(const char* file_name)
{
    assert(file_name != NULL);

    FILE* file = fopen(file_name, "r");

    if (file) {
        parser_file_lineno = 1;

        yyin = file;
        YY_BUFFER_STATE state = yy_create_buffer(yyin, YY_BUF_SIZE);
        yy_switch_to_buffer(state);

        yyparse();
        fclose (file);
        yy_delete_buffer(state);

        yyin = stdin;
    } else {
        printf("error: can't open file: %s\n", file_name);
    }

    return query_list;
}

tree_node_t* parse_sql_string(const char* sql_string)
{
    assert(sql_string != NULL);

    size_t len = strlen(sql_string);
    char* tmp = malloc(sizeof(char) * (len + 3));
    memcpy(tmp, sql_string, len);
    tmp[len + 0] = '\n';
    tmp[len + 1] = '\0';
    tmp[len + 2] = '\0';

    YY_BUFFER_STATE state = yy_scan_buffer(tmp, sizeof(char) * (len + 3));
    yyparse();

    yy_delete_buffer(state);
    free((void*)tmp);

    return query_list;
}

int parser_get_error()
{
    return parser_error;
}

void parser_set_error()
{
    parser_error = 1;
}

void parser_reset()
{
    parser_error = 0;

    free_str_list(name_list);
    name_list = NULL;

    free_tree_node_list(node_list);
    node_list = NULL;

    query_list = NULL;
    query_list_last = NULL;
}

//------------------------------------------------------------------------------
//   util
//------------------------------------------------------------------------------
char* new_str(const char* str)
{
    size_t len = strlen(str);
    char* tmp = malloc(sizeof(char) * (len + 1));
    memcpy(tmp, str, len + 1);
    return tmp;
}

str_list_t* new_str_list(const char* str)
{
    char* tmp = new_str(str);

    str_list_t* list_node = malloc(sizeof(str_list_t));
    list_node->next = NULL;
    list_node->str = tmp;

    return list_node;
}

void free_str_list(str_list_t* str_list)
{
    if (str_list == NULL)
        return;

    free((void*)str_list->str);
    free_str_list(str_list->next);
    free(str_list);
}

void dump_str_list(str_list_t* list)
{
    str_list_t* tmp = list;
    while (tmp != NULL) {
        printf("%p: %s\n", (void*)(tmp->str), tmp->str);
        tmp = tmp->next;
    }
}

//------------------------------------------------------------------------------
//   debug
//------------------------------------------------------------------------------
void debug_lex(int type, const char* yytext)
{
#ifdef DEBUG_LEX
    const char* type_str = get_lex_macro_str(type);
    printf("%s: %s\n", type_str, yytext);
#endif
}

#define TOKEN(name) case name: return #name
#define CHAR(name) case name: return "CHAR"
#define CHAR_STR(name, str) case name: return str
const char* get_lex_macro_str(int type)
{
    switch (type) {
        TOKEN(COMMENT);
        TOKEN(CREATE);
        TOKEN(TABLE);
        TOKEN(NAME);
        TOKEN(COLUMN_NAME);
        TOKEN(INT);
        TOKEN(STR20);
        TOKEN(INSERT);
        TOKEN(INTO);
        TOKEN(VALUES);
        TOKEN(INTEGER);
        TOKEN(LITERAL);
        TOKEN(SELECT);
        TOKEN(DISTINCT);
        TOKEN(FROM);
        TOKEN(WHERE);
        TOKEN(ORDER);
        TOKEN(BY);
        TOKEN(DELETE);
        TOKEN(DROP);
        TOKEN(OR);
        TOKEN(AND);
        TOKEN(NOT);
        TOKEN(COMP_OP);
        TOKEN(NULL_VALUE);
        CHAR('(');
        CHAR(')');
        CHAR(',');
        CHAR_STR('*', "*");
        CHAR('[');
        CHAR(']');
        CHAR_STR('+', "+");
        CHAR_STR('-', "-");
        CHAR_STR('/', "/");
        TOKEN(CREATE_TABLE_STATEMENT);
        TOKEN(DROP_TABLE_STATEMENT);
        TOKEN(SELECT_STATEMENT);
        TOKEN(DELETE_STATEMENT);
        TOKEN(INSERT_STATEMENT);
        TOKEN(ATTRIBUTE_TYPE_LIST);
        TOKEN(NAME_TYPE);
        TOKEN(NAME_LIST);
        TOKEN(VALUE_LIST);
        TOKEN(INSERT_TUPLES);
        TOKEN(SELECT_LIST);
        TOKEN(ORDER_OPTION);
        TOKEN(WHERE_OPTION);
        TOKEN(COMPARISON_PREDICATE); 
        TOKEN(SEARCH_CONDITION);
    }

    return "?";
}
