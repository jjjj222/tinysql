#include<iostream>
using namespace std;

//#include "parser.h"
#include "cmd.h"
//#include "test.h"
//#include "dbMgr.h"

int test_storage_mgr();

extern FILE * yyin;

int main(int argc, char* argv [])
{
    if (argc >= 2) {
        cmd_readfile(argv[1]);
    } else {
        cmd_readline();
    }
    //HwMgr hw_mgr;

    //hw_mgr.dump();

    return 0;
}

