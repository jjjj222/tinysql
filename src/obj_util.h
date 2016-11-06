#ifndef JJJJ222_OBJ_UTIL_H
#define JJJJ222_OBJ_UTIL_H

#include <vector>
#include <string>

namespace jjjj222 {

using std::vector;
using std::string;

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
vector<string> tokenize(const string& str, const string& del);

//------------------------------------------------------------------------------
//   DrawTable
//------------------------------------------------------------------------------
class DrawTable
{
    public:
        enum Style {
            MYSQL_TABLE,
            DEFAULT
        };

    public:
        DrawTable(size_t width);
        DrawTable(size_t width, Style style);

        size_t size() const { return _rows.size(); }

        void set_header(const vector<string>&);
        void set_align(size_t, bool);
        void set_align_right(size_t i) { set_align(i, true); }
        void add_row(const string&);
        void add_row(const string&, const string&);
        void add_row(const vector<string>&);

        void draw() const;

    private:
        vector<size_t> get_widths() const;
        void calculate_width(const vector<string>&, vector<size_t>&) const;
        string get_border(char, size_t) const;
        bool get_align(size_t) const;

        void draw_mysql_table() const;
        void draw_mysql_table_border(const vector<size_t>&) const;
        void draw_row(const vector<size_t>&, const vector<string>&, char) const;

        void draw_default() const;


    private:
        size_t                  _width;
        Style                   _style;
        vector<bool>            _align;
        vector<string>          _header;
        vector<vector<string>>  _rows;
};

//------------------------------------------------------------------------------
//   error message
//------------------------------------------------------------------------------
void error_msg_not_exist(const string&, const string&);
void error_msg(const string&);
void warning_msg(const string&);

} // namespace jjjj222
#endif
