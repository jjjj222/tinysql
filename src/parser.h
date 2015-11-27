#ifndef PARSER_H
#define PARSER_H

//------------------------------------------------------------------------------
//   debug flag
//------------------------------------------------------------------------------
//#define DEBUG_YACC
//#define DEBUG_LEX

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
#define YYSTYPE tree_node_t*
extern int yyparse();

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
struct tree_node
{
    int type;
    const char* value;
    struct tree_node* next;
    struct tree_node* child;
};
typedef struct tree_node tree_node_t;

struct str_list
{
    const char* str;
    struct str_list* next;
};
typedef struct str_list str_list_t;

struct tree_node_list
{
    tree_node_t* node;
    struct tree_node_list* next;
};
typedef struct tree_node_list tree_node_list_t;
//------------------------------------------------------------------------------
//   global variable
//------------------------------------------------------------------------------

extern int parser_file_lineno;
extern const char* parser_file_name;

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//   parser
//------------------------------------------------------------------------------
tree_node_t* parse_sql_file(const char*);
tree_node_t* parse_sql_string(const char*);

int parser_get_error();
void parser_set_error();
void parser_reset();

//------------------------------------------------------------------------------
//   node type
//------------------------------------------------------------------------------
int node_is_create_table(tree_node_t*);
int node_is_drop_table(tree_node_t*);
int node_is_insert(tree_node_t*);
int node_is_delete(tree_node_t*);
int node_is_select(tree_node_t*);
int node_is_where(tree_node_t*);
int node_is_order_by(tree_node_t*);
int node_is_attribute_type_list(tree_node_t*);
int node_is_name_type(tree_node_t*);
int node_is_name_list(tree_node_t*);
int node_is_value_list(tree_node_t*);
int node_is_select_list(tree_node_t*);
int node_is_int(tree_node_t*);
int node_is_str20(tree_node_t*);
int node_is_distinct(tree_node_t*);
int node_is_null(tree_node_t*);
int node_is_or(tree_node_t*);
int node_is_and(tree_node_t*);
int node_is_not(tree_node_t*);
int node_is_comp_op(tree_node_t*);
int node_is_arith_op(tree_node_t*);
int node_is_name(tree_node_t*);
int node_is_column_name(tree_node_t*);
int node_is_literal(tree_node_t*);
int node_is_integer(tree_node_t*);


//------------------------------------------------------------------------------
//   parse tree
//------------------------------------------------------------------------------
tree_node_t* tree_add_child_front(tree_node_t* node, tree_node_t* child);
tree_node_t* new_tree_node(int type, const char* value);
tree_node_t* new_tree_node_0(int type);
tree_node_t* new_tree_node_1(int type, tree_node_t*);
tree_node_t* new_tree_node_2(int type, tree_node_t*, tree_node_t*);
tree_node_t* new_tree_node_3(int type, tree_node_t*, tree_node_t*, tree_node_t*);
tree_node_t* new_tree_node_n(int type, int nops, ...);
void free_parse_tree(tree_node_t*);
void dump_tree_node(const tree_node_t*);
void dump_tree_node_indent(const tree_node_t* node, const char* indent);

void add_to_query_list(tree_node_t*);
void dump_query_list();
void dump_node_list();

tree_node_list_t* new_tree_node_list(tree_node_t*);
void free_tree_node_list(tree_node_list_t*);
void dump_tree_node_list(tree_node_list_t*);

//------------------------------------------------------------------------------
//   name mgr 
//------------------------------------------------------------------------------
const char* lookup_symbol(const char*);
void dump_name_list();

//------------------------------------------------------------------------------
//   util
//------------------------------------------------------------------------------
char* new_str(const char*);
str_list_t* new_str_list(const char*);
void free_str_list(str_list_t*);
void dump_str_list(str_list_t*);

//------------------------------------------------------------------------------
//   debug
//------------------------------------------------------------------------------
void debug_lex(int type, const char*);
const char* get_lex_macro_str(int type);

#ifdef __cplusplus
}
#endif

#endif
