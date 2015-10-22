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
%token DELETE DROP
%token OR AND NOT COMP_OP

%%

statement_line:
        one_statement '\n' statement_line
    |   '\n' statement_line
    |
    ;

one_statement:
        statement { printf("------------------- line %d ------------------\n", lineno); }
    ;

statement:
        create_table_statement
    |   drop_table_statement
    |   select_statement
    |   delete_statement
    |   insert_statement
    ;

create_table_statement:
        CREATE TABLE NAME '(' attribute_type_list ')'
    ;

drop_table_statement:
        DROP TABLE NAME
    ;

select_statement:
        SELECT select_option select_list FROM name_list where_option order_option
    ;

select_option:
        DISTINCT
    |
    ;

where_option:
        WHERE search_condition
    |
    ;

order_option:
        ORDER BY column_name
    |
    ;

delete_statement:
        DELETE FROM NAME where_option
    ;

insert_statement:
        INSERT INTO NAME '(' name_list ')' insert_tuples
    ;

insert_tuples:
        VALUES '(' value_list ')'
    | select_statement
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

search_condition:
        boolean_term OR search_condition
    |   boolean_term
    ;

boolean_term:
        boolean_factor AND boolean_term
    |   boolean_factor
    ;

boolean_factor:
        boolean_primary
    |   NOT boolean_primary
    ;

boolean_primary:
        '[' search_condition ']'
    |   comparison_predicate
    ;

comparison_predicate:
        expression COMP_OP expression
    ;

expression:
        term '+' expression
    |   term '-' expression
    |   term
    ;

term:
        factor '*' term
    |   factor '/' term
    |   factor
    ;

factor:
        '(' expression ')'
    |   column_name
    |   LITERAL
    |   INTEGER
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

//void yyerror(char *s) {
//    fprintf(stderr, "%s\n", s);
//}

//int main(void) {
//    yyparse();
//    return 0;
//}
