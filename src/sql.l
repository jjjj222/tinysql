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
FROM        KEYWORD(FROM)

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

[(),*]      {
                debug_lex(*yytext, yytext);
                return *yytext;
            }


[-+]        {
                debug_lex(*yytext, yytext);
                return *yytext;
            }

[ \t]       { /* skip whitespace */ }

#.*         { debug_lex(COMMENT, yytext); }

[\n]        { return *yytext; }


.           { yyerror("Unknown character"); }

%%
//letter[digit|letter]]*  {
//                            return NAME; 
//                        }

//[<>=]       {
//                return COMP_OP;
//            }
int yywrap(void) {
    return 1;
}
