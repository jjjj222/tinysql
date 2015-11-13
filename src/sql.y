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

%token STATEMENT
%token CREATE_TABLE_STATEMENT DROP_TABLE_STATEMENT
%token SELECT_STATEMENT DELETE_STATEMENT INSERT_STATEMENT
%token ATTRIBUTE_TYPE_LIST
%token INSERT_TUPLES
%token NAME_TYPE NAME_LIST VALUE_LIST
%token SELECT_LIST
%token ORDER_OPTION WHERE_OPTION
%token COMPARISON_PREDICATE
%token SEARCH_CONDITION
%%

statement_line:
        one_statement '\n' statement_line
    |   '\n' statement_line
    |
    ;

one_statement:
        statement { 
            add_to_query_list($1);
#ifdef DEBUG_YACC
            dump_tree_node($1);
            printf("------------------- line %d ------------------\n", parser_file_lineno++);
#endif
        }
    ;

statement:
        create_table_statement
    |   drop_table_statement
    |   select_statement
    |   delete_statement
    |   insert_statement
    ;

create_table_statement:
        CREATE TABLE NAME '(' attribute_type_list ')' {
            $$ = new_tree_node_2(CREATE_TABLE_STATEMENT, $3, $5);
        }
    ;

drop_table_statement:
        DROP TABLE NAME {
            $$ = new_tree_node_1(DROP_TABLE_STATEMENT, $3);
        }
    ;

select_statement:
        SELECT select_option select_list FROM name_list where_option order_option {
            $$ = new_tree_node_n(SELECT_STATEMENT, 5, $2, $3, $5, $6, $7);
        }
    ;

select_option:
        DISTINCT {
            $$ = new_tree_node_0(DISTINCT);
        }
    |   {
            $$ = NULL;
        }
    ;

where_option:
        WHERE search_condition {
            $$ = new_tree_node_1(WHERE_OPTION, $2);
        }
    |   {
            $$ = NULL;
        }
    ;

order_option:
        ORDER BY column_name {
            $$ = new_tree_node_1(ORDER_OPTION, $3);
        }
    |   {
            $$ = NULL;
        }
    ;

delete_statement:
        DELETE FROM NAME where_option {
            $$ = new_tree_node_n(DELETE_STATEMENT, 2, $3, $4);
        }
    ;

insert_statement:
        INSERT INTO NAME '(' name_list ')' insert_tuples {
            $$ = new_tree_node_3(INSERT_STATEMENT, $3, $5, $7);
        }
    ;

insert_tuples:
        VALUES '(' value_list ')' {
            $$ = new_tree_node_1(INSERT_TUPLES, $3);
        }
    |   select_statement {
            $$ = new_tree_node_1(INSERT_TUPLES, $1);
        }
    ;

attribute_type_list:
        name_type ',' attribute_type_list {
            $$ = tree_add_child_front($3, $1);
        }
    |   name_type {
            $$ = new_tree_node_1(ATTRIBUTE_TYPE_LIST, $1);
        }
    ;

name_type:
        NAME data_type {
            $$ = new_tree_node_2(NAME_TYPE, $1, $2);
        }
    ;

select_list:
        '*' {
            $$ = new_tree_node_0('*'); 
        }
    |   select_sub_list
    ;

select_sub_list:
        column_name ',' select_sub_list {
            $$ = tree_add_child_front($3, $1);
        }
    |   column_name {
            $$ = new_tree_node_1(SELECT_LIST, $1);
        }
    ;

name_list:
        NAME ',' name_list {
            $$ = tree_add_child_front($3, $1);
        }
    |   NAME {
            $$ = new_tree_node_1(NAME_LIST, $1);
        }
    ;

value_list:
        value ',' value_list {
            $$ = tree_add_child_front($3, $1);
        }
    |   value {
            $$ = new_tree_node_1(VALUE_LIST, $1);
        }
    ;

data_type:
        INT { 
            $$ = new_tree_node_0(INT); 
        }
    |   STR20 {
            $$ = new_tree_node_0(STR20);
        }
    ;

column_name:
        COLUMN_NAME
    |   NAME
    ;

value:
        LITERAL
    |   INTEGER
    |   NULL_VALUE {
            $$ = new_tree_node_0(NULL_VALUE);
        }
    ;

search_condition:
        boolean_term OR search_condition {
            if ($3->type == OR) {
                $$ = tree_add_child_front($3, $1);
            } else {
                $$ = new_tree_node_2(OR, $1, $3);
            }
        }
    |   boolean_term
    ;

boolean_term:
        boolean_factor AND boolean_term {
            if ($3->type == AND) {
                $$ = tree_add_child_front($3, $1);
            } else {
                $$ = new_tree_node_2(AND, $1, $3);
            }
        }
    |   boolean_factor
    ;

boolean_factor:
        NOT boolean_primary {
        $$ = new_tree_node_1(NOT, $2);
    }
    |   boolean_primary
    ;

boolean_primary:
        '[' search_condition ']' {
            $$ = $2;
        }
    |   comparison_predicate
    ;

comparison_predicate:
        expression COMP_OP expression {
            $$ = tree_add_child_front($2, $3);
            $$ = tree_add_child_front($2, $1);
        }
    ;

expression:
        term '+' expression {
            if ($3->type == '+') {
                $$ = tree_add_child_front($3, $1);
            } else {
                $$ = new_tree_node_2('+', $1, $3);
            }
        }
    |   term '-' expression {
            //NOTE: not commutative
            $$ = new_tree_node_2('-', $1, $3);
        }
    |   term
    ;

term:
        factor '*' term {
            if ($3->type == '*') {
                $$ = tree_add_child_front($3, $1);
            } else {
                $$ = new_tree_node_2('*', $1, $3);
            }
        }
    |   factor '/' term {
            $$ = new_tree_node_2('/', $1, $3);
        }
    |   factor
    ;

factor:
        '(' expression ')' {
            $$ = $2;
        }
    |   column_name
    |   LITERAL
    |   INTEGER
    ;
%%

