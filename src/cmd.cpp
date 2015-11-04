#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>

using namespace std;

#include <readline/readline.h>
#include <readline/history.h>

#include "util.h"
#include "debug.h"

using jjjjj222::dump_str;
using jjjjj222::dump_pretty_impl;
using jjjjj222::tokenize;
using jjjjj222::UpdateTo;

#include "parser.h"
#include "cmd.h"
#include "dbMgr.h"

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
//int cmd_process(const char* buf)
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
        } else {
            state = cmd_readfile(tokens[1].c_str());
        }
    } else if (tokens[0] == "show") {
        if (tokens.size() < 2) {
            cmd_missing_error("command");
        } else if (tokens[1] == "table"){
            cout << "TODO: show table" << endl;
        } else {
            cmd_unknown_error("command", tokens[1]);
        }
    } else {
        SqlParser parser;
        parser.parse_string(buf);
        parser.dump();
        if (parser.is_error())
            state = CMD_ERROR;
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
        if (cmd_process(line.c_str()) != CMD_OK) {
            return CMD_ERROR;
        }
    }

    fin.close();

    return CMD_OK;
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
//bool cmd_match_1(const vector<string>&, const string&)
//{
//}
//
//bool cmd_match_2(const vector<string>&, const string&, const string&);

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
    cmd_error(
        dump_str(parser_file_name) +
        ":" +
        dump_str(parser_file_lineno) +
        ": " +
        str
    );
}

void cmd_error(const string& str)
{
    cerr << "Error: " << str << " !!" << endl;
}
