%{
    #include "parser.h"
    #include "y.tab.h"
    void yyerror(char *);

    #define KEYWORD(word) { debug_lex(word, yytext); yylval = NULL; return word; }
    #define NEW_NODE(type) {\
        debug_lex(type, yytext);\
        yylval = new_tree_node(type, lookup_symbol(yytext));\
        return type;\
    }
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

{name}          NEW_NODE(NAME);
{name}\.{name}  NEW_NODE(COLUMN_NAME);
{digit}+        NEW_NODE(INTEGER);
\"[^\"]*\"      NEW_NODE(LITERAL);
[<>=]           NEW_NODE(COMP_OP);

[(),\[\]]   {
                debug_lex(*yytext, yytext);
                return *yytext;
            }

[-+*/]      {
                debug_lex(*yytext, yytext);
                return *yytext;
            }

[ \t]       { 
                /* skip whitespace */ 
            }

--.*        {
                /* skip comment */
            }

#.*         {
                /* skip comment */
            }

[\n]        { 
                return *yytext; 
            }


.           {
                yyerror("unknown character"); 
            }

%%
void yyerror(char* s)
{
    if (parser_file_name != NULL) {
        fprintf(stderr, "Error: file %s line %d: %s at '%s'\n",
             parser_file_name,
             parser_file_lineno, s, yytext);
    } else {
        fprintf(stderr, "Error: %s at '%s'\n", s, yytext);
    }
    parser_set_error();
}

int yywrap(void) {
    return 1;
}
