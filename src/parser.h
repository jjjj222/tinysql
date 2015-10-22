#ifndef PARSER_H
#define PARSER_H


struct tree_node
{
    int type;
    const char* value;
    struct tree_node* next;
    struct tree_node* child;
};

typedef struct tree_node tree_node_t;



#ifdef __cplusplus
extern "C" {
#endif

extern int lineno;

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
void sql_parser();


//------------------------------------------------------------------------------
//   debug
//------------------------------------------------------------------------------
void debug_lex(int type, const char*);
const char* get_lex_macro_str(int type);

#ifdef __cplusplus
}
#endif

#endif
