%{
    #include <stdio.h>
    #include "parser.h"
    int yylex(void);
    void yyerror(char *);
%}

%token COMMENT
%token NAME COLUMN_NAME
%token CREATE TABLE
%token INT STR20
%token INSERT INTO VALUES
%token LITERAL INTEGER NULL_VALUE
%token SELECT DISTINCT FROM WHERE ORDER BY

%%

statement_line:
        one_statement '\n' statement_line
    |   '\n' statement_line
    |
    ;

one_statement:
        statement { printf("------------------- one statement ----------\n"); }
    ;

statement:
        create_table_statement
    |   select_statement
    |   insert_statement
    ;

create_table_statement:
        CREATE TABLE NAME '(' attribute_type_list ')'
    ;

select_statement:
        select_type select_list FROM name_list
    ;

select_type:
        SELECT
    |   SELECT DISTINCT
    ;

insert_statement:
        INSERT INTO NAME '(' name_list ')' insert_tuples
    ;

insert_tuples:
        VALUES '(' value_list ')'
    ;

attribute_type_list:
        NAME data_type ',' attribute_type_list
    |   NAME data_type
    ;

select_list:
        '*'
    |   select_sub_list
    ;

select_sub_list:
        column_name ',' select_sub_list
    |   column_name
    ;

name_list:
        NAME ',' name_list
    |   NAME
    ;

value_list:
        value ',' value_list
    | value
    ;

data_type:
        INT
    |   STR20
    ;

column_name:
        COLUMN_NAME
    |   NAME
    ;

value:
        LITERAL
    |   INTEGER
    |   NULL_VALUE
    ;
%%

//program:
//        program expr '\n'         { printf("%d\n", $2); }
//        |
//        ;
//
//expr:
//        INTEGER
//        | INTEGER '+' expr           { printf("%d + %d\n", $1, $3); $$ = $1 + $3; }
//        | INTEGER '-' expr           { printf("-\n"); $$ = $1 - $3; }
//        ;
//
//%%

void yyerror(char *s) {
    fprintf(stderr, "%s\n", s);
}

//int main(void) {
//    yyparse();
//    return 0;
//}
