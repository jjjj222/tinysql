%{
    #include <stdio.h>
    #include "parser.h"
    int yylex(void);
    void yyerror(char *);

    //#define KEYWORD(word) {  }
%}

%{
/*
%union {
    //const char* value;
    struct tree_node* node_ptr;
};

%token <node_ptr> NAME
*/
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

%{
/*
%type <node_ptr> statement
%type <node_ptr> create_table_statement
*/
%}

%token STATEMENT
%token CREATE_TABLE_STATEMENT DROP_TABLE_STATEMENT
%token SELECT_STATEMENT DELETE_STATEMENT INSERT_STATEMENT
%token ATTRIBUTE_TYPE_LIST
%token INSERT_TUPLES
%token NAME_TYPE NAME_LIST VALUE_LIST
%%

statement_line:
        one_statement '\n' statement_line
    |   '\n' statement_line
    |
    ;

one_statement:
        statement { 
            //printf("%p: %s\n", (void*)($1), $1->value); 
            add_to_query_list($1);
            //dump_tree_node($1);
            //printf("------------------- line %d ------------------\n", lineno);
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
        '*'
    |   select_sub_list
    ;

select_sub_list:
        column_name ',' select_sub_list
    |   column_name
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
        INT { $$ = new_tree_node_0(INT); }
    |   STR20 {$$ = new_tree_node_0(STR20); }
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
