#ifndef CMD_H
#define CMD_H

#include <string>

using std::string;

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
enum CmdState {
    CMD_OK,
    CMD_ERROR,
    CMD_QUIT
};

bool cmd_is_not_comment(const string&);
CmdState cmd_process(const char*);
int cmd_readline();
CmdState cmd_readfile(const char*);

//------------------------------------------------------------------------------
//   error
//------------------------------------------------------------------------------
void cmd_unknown_error(const string&, const string&);
void cmd_missing_error(const string&);
void cmd_error_file_lineno(const string&);

#endif
