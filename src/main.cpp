#include<iostream>
using namespace std;

#include "parser.h"
#include "cmd.h"
#include "test.h"

int test_storage_mgr();

extern FILE * yyin;

int main(int argc, char* argv [])
{
    //cout << "Hello World!!" << endl;
    //test_storage_mgr();

    //free_str_list(name_list); // TODO
    // open a file handle to a particular file:

    //FILE *myfile = fopen(argv[1], "r");
    //// make sure it's valid:
    //if (!myfile) {
    //    cout << "I can't open file!" << endl;
    //    return -1;
    //}
    //// set lex to read from it instead of defaulting to STDIN:
    //yyin = myfile;
    ////fclose (pFile);

    //// lex through the input:
    ////yylex();
    //sql_parser();

    cmd_readline();
    return 0;
}

