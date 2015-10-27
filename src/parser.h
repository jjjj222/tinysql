#ifndef PARSER_H
#define PARSER_H

#define YYSTYPE tree_node_t*

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

extern int lineno;

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//   parser
//------------------------------------------------------------------------------
//void sql_parser();

tree_node_t* parse_sql_file(const char*);
tree_node_t* parse_sql_string(const char*);
void parser_reset();


//------------------------------------------------------------------------------
//   parse tree
//------------------------------------------------------------------------------
tree_node_t* tree_add_child_front(tree_node_t* node, tree_node_t* child);
tree_node_t* new_tree_node(int type, const char* value);
tree_node_t* new_tree_node_0(int type);
tree_node_t* new_tree_node_1(int type, tree_node_t*);
tree_node_t* new_tree_node_2(int type, tree_node_t*, tree_node_t*);
tree_node_t* new_tree_node_3(int type, tree_node_t*, tree_node_t*, tree_node_t*);
tree_node_t* new_tree_node_n(int type, size_t nops, ...);
void free_parse_tree(tree_node_t*);
void dump_tree_node(const tree_node_t*);

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
