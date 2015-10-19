%{
    #include "y.tab.h"
    #include "parser.h"
    //#include <stdlib.h>
    void yyerror(char *);

    //#define
    #define KEYWORD(word) { debug_lex(word, yytext); return word; }
%}

letter     [A-Za-z]
digit      [0-9]
name       {letter}({digit}|{letter})* 

%option nounput noinput

%%

CREATE      KEYWORD(CREATE)
TABLE       KEYWORD(TABLE)
INT         KEYWORD(INT)
STR20       KEYWORD(STR20)
INSERT      KEYWORD(INSERT)
INTO        KEYWORD(INTO)
VALUES      KEYWORD(VALUES)
NULL        KEYWORD(NULL_VALUE)
SELECT      KEYWORD(SELECT)
DISTINCT    KEYWORD(DISTINCT)
FROM        KEYWORD(FROM)
WHERE       KEYWORD(WHERE)
ORDER       KEYWORD(ORDER)
BY          KEYWORD(BY)
DELETE      KEYWORD(DELETE)
DROP        KEYWORD(DROP)
OR          KEYWORD(OR)
AND         KEYWORD(AND)
NOT         KEYWORD(NOT)

{name}      {
                debug_lex(NAME, yytext);
                return NAME;
            }

{name}\.{name} {
                debug_lex(COLUMN_NAME, yytext);
                return COLUMN_NAME;
            }

{digit}+    {
                debug_lex(INTEGER, yytext);
                //yylval = atoi(yytext);
                return INTEGER;
            }

\"[^\"]*\"  {
                debug_lex(LITERAL, yytext);
                return LITERAL;
            }

[(),*\[\]]  {
                debug_lex(*yytext, yytext);
                return *yytext;
            }

[<>=]       {
                debug_lex(COMP_OP, yytext);
                return COMP_OP;
            }


[-+*/]      {
                debug_lex(*yytext, yytext);
                return *yytext;
            }

[ \t]       { 
                /* skip whitespace */ 
            }

#.*         {
                //debug_lex(COMMENT, yytext); 
            }

[\n]        { 
                lineno++;
                return *yytext; 
            }


.           { yyerror("unknown character"); }

%%
//letter[digit|letter]]*  {
//                            return NAME; 
//                        }

//[<>=]       {
//                return COMP_OP;
//            }
void yyerror(char* s)
{
    fprintf(stderr, "Error: line %d: %s at '%s'\n", lineno, s, yytext);
}

int yywrap(void) {
    return 1;
}
