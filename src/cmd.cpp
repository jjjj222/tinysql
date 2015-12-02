#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>

using namespace std;

#include <readline/readline.h>
#include <readline/history.h>

#include "tiny_util.h"
#include "util.h"
#include "debug.h"
#include "obj_util.h"

using jjjj222::UpdateTo;
using jjjj222::tokenize;
using jjjj222::error_msg;

#include "parser.h"
#include "cmd.h"
#include "dbMgr.h"
#include "query.h"

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
bool cmd_is_not_comment(const string& line)
{
    bool res = !line.empty() && line[0] != '#' && line[0] != '-';
    return res;
}

CmdState cmd_process(const char* buf)
{
    //printf("[%s]\n",buf);
    vector<string> tokens = tokenize(buf, " \t");
    if (tokens.empty())
        return CMD_OK;

    CmdState state = CMD_OK;

    if (tokens[0] == "make") {
        state =  CMD_QUIT;
    } else if (tokens[0] == "source") {
        if (tokens.size() < 2) {
            cmd_missing_error("file name");
            state =  CMD_ERROR;
        } else {
            state = cmd_readfile(tokens[1].c_str());
        }
    } else if (tokens[0] == "show") {
        if (tokens.size() < 2) {
            cmd_missing_error("command");
            state =  CMD_ERROR;
        } else if (tokens[1] == "tables"){
            HwMgr::ins()->dump_relations();
        } else {
            cmd_unknown_error("command", tokens[1]);
            state =  CMD_ERROR;
        }
    } else if (tokens[0] == "dump") {
        if (tokens.size() < 2) {
            cmd_missing_error("command");
            state =  CMD_ERROR;
        } else if (tokens[1] == "time"){
            HwMgr::ins()->print_time();
        } else if (tokens[1] == "memory"){
            HwMgr::ins()->dump_memory();
        } else if (tokens[1] == "relation"){
            if (tokens.size() < 3) {
                cmd_missing_error("relation name");
                state =  CMD_ERROR;
            } else {
                HwMgr::ins()->dump_relation(tokens[2]);
            }
        } else {
            cmd_unknown_error("command", tokens[1]);
            state =  CMD_ERROR;
        }
    } else if (tokens[0] == "quit") {
        state =  CMD_QUIT;
        cout << "Bye" << endl;
    } else {
        QueryMgr query_mgr;
        if (!query_mgr.exec_query(buf)) {
            state = CMD_ERROR;
        } else {
            //if (cmd_is_not_comment(buf)) {
            //    cout << "Query OK ("<< HwMgr::ins()->get_elapse_io() << " disk I/O)"<< endl;
            //}
        }
    }

    if (cmd_is_not_comment(buf)) {
        cout << endl;
    }

    return state;
}

int cmd_readline()
{
    char* buf;

    while ((buf = readline("tinysql> ")) != NULL) {
        if (cmd_process(buf) == CMD_QUIT)
            break;

        add_history(buf);
    }

    free(buf);
    return 0;
}

CmdState cmd_readfile(const char* file_name)
{
    ifstream fin(file_name);

    if (!fin.is_open()) {
        return CMD_ERROR;
    }

    UpdateTo<const char*> update_name(&parser_file_name, file_name);
    UpdateTo<int> update_lineno(&parser_file_lineno, 0);

    string line;
    while (fin.good()) {
        getline(fin, line);
        parser_file_lineno++;
        if (cmd_is_not_comment(line)) {
            cout << parser_file_name << ":" << parser_file_lineno << "> " << line << endl;
        }

        if (cmd_process(line.c_str()) != CMD_OK) {
            return CMD_ERROR;
        }
    }

    fin.close();

    return CMD_OK;
}

//------------------------------------------------------------------------------
//   error
//------------------------------------------------------------------------------
void cmd_unknown_error(const string& type, const string& str)
{
    cmd_error_file_lineno(
        "unknown " +
        type +
        " \'" +
        str +
        "\'"
    );
}

void cmd_missing_error(const string& str)
{
    cmd_error_file_lineno(
        "missing " +
        str
    );
}

void cmd_error_file_lineno(const string& str)
{
    error_msg(
        tiny_dump_str(parser_file_name) +
        ":" +
        tiny_dump_str(parser_file_lineno) +
        ": " +
        str
    );
}
