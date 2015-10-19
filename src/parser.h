#ifndef PARSER_H
#define PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

extern int lineno;

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
