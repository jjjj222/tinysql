#ifndef CMD_H
#define CMD_H

#include <string>
using std::string;

enum CmdState {
    CMD_OK,
    CMD_ERROR,
    CMD_QUIT
};

CmdState cmd_process(const char*);
int cmd_readline();
CmdState cmd_readfile(const char*);

//bool cmd_match_1(const vector<string>&, const string&);
//bool cmd_match_2(const vector<string>&, const string&, const string&);

void cmd_unknown_error(const string&, const string&);
void cmd_missing_error(const string&);
void cmd_error_file_lineno(const string&);
void cmd_error(const string&);
//class SetFileName
//{
//    public:
//        SetFileName(const char* file_name)
//            : _pre_file_name(parser_file_name)
//        {
//            parser_file_name = file_name;
//        }
//
//        ~SetFileName()
//        {
//            parser_file_name = _pre_file_name;
//        }
//
//    private:
//        const char* _pre_file_name;
//};

#endif
