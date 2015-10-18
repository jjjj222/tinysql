#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "parser.h"

#include "y.tab.h"

#define DEBUG

extern int yyparse();

void sql_parser()
{
    yyparse();
}

//------------------------------------------------------------------------------
//   debug
//------------------------------------------------------------------------------
void debug_lex(int type, const char* yytext)
{
#ifdef DEBUG
    const char* type_str = get_lex_macro_str(type);
    printf("%s: %s\n", type_str, yytext);
#endif
}

#define TOKEN(name) case name: return #name
#define CHAR(name) case name: return "CHAR"
const char* get_lex_macro_str(int type)
{
    switch (type) {
        TOKEN(COMMENT);
        TOKEN(CREATE);
        TOKEN(TABLE);
        TOKEN(NAME);
        TOKEN(INT);
        TOKEN(STR20);
        TOKEN(INSERT);
        TOKEN(INTO);
        TOKEN(VALUES);
        TOKEN(INTEGER);
        TOKEN(LITERAL);
        TOKEN(SELECT);
        TOKEN(FROM);
        CHAR('(');
        CHAR(')');
        CHAR(',');
        CHAR('*');
    }

    return "?";
}
