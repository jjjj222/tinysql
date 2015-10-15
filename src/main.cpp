#include<iostream>
using namespace std;

#include "parser.h"

int main(int argc, char* argv [])
{
    cout << "Hello World!!" << endl;
    //yyparse();
    //yylex();
    //int i = yylex();
    sql_parser();
    //cout << i << endl;
    return 0;
}
