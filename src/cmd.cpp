#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>

using namespace std;

#include <readline/readline.h>
#include <readline/history.h>

#include "util.h"
#include "debug.h"

using jjjjj222::dump_pretty_impl;
using jjjjj222::tokenize;

#include "parser.h"

int cmd_readline()
{
    char* buf;
    //rl_bind_key('\t',rl_abort);//disable auto-complete
    while ((buf = readline("tinysql> ")) != NULL) {
        //if (strcmp(buf, "quit") == 0)
        //    break;

        printf("[%s]\n",buf);
        vector<string> tokens = tokenize(buf, " \t");
        if (tokens.empty())
            continue;

        if (tokens[0] == "make") {
            break;
        }

        if (tokens[0] == "source") {
            if (tokens.size() < 2) {
                cout << "error: missing file" << endl;
            } else {
                parse_sql_file(tokens[1].c_str());
            }
        } else if (tokens[0] == "test_file") {
            parse_sql_file("./testcases/example.in");
        } else if (tokens[0] == "test_str") {
            const char tstr [] = "CREATE TABLE course (sid INT, homework INT, project INT, exam INT, grade STR20)";
            parse_sql_string(tstr);
        } else if (tokens[0] == "stdin") {
            sql_parser();
        } else {
            parse_sql_string(buf);
        }

        //dump_name_list();
        //dump_node_list();
        dump_query_list();
        parser_reset();

        //dump_pretty(tokens);
        //if (buf[0] != 0) {
        //if (!tokens.empty()) {
        add_history(buf);
        //}
    }

    free(buf);
    return 0;
}

