%{
    #include <stdio.h>
    int yylex(void);
    void yyerror(char *);
%}

%token INTEGER STRING

%%

program:
        program expr '\n'         { printf("%d\n", $2); }
        |
        ;

expr:
        INTEGER
        | INTEGER '+' expr           { $$ = $1 + $3; }
        | INTEGER '-' expr           { $$ = $1 - $3; }
        ;

%%

void yyerror(char *s) {
    fprintf(stderr, "%s\n", s);
}

//int main(void) {
//    yyparse();
//    return 0;
//}
