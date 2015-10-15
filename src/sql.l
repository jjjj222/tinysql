number      [0-9]
letter      [A-Za-z]
    /* calculator #1 */
%{
    #include "y.tab.h"
    //#include <stdlib.h>
    void yyerror(char *);

    //#define
%}

%option nounput noinput

%%

letter+     {
                return STRING;
            }

[0-9]+      {
                yylval = atoi(yytext);
                return INTEGER;
            }

[-+\n]      { return *yytext; }

[ \t]       ;       /* skip whitespace */

.           yyerror("Unknown character");

%%

int yywrap(void) {
    return 1;
}
