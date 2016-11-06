#include <iostream>
#include <iomanip>
#include <cassert>

#include "obj_util.h"

namespace jjjj222 {

using std::cout;
using std::cerr;
using std::endl;
using std::setw;
using std::left;
using std::right;

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
vector<string> tokenize(const string& str, const string& del)
{
    vector<string> tokens;
    size_t begin, end = 0;
    while ((begin = str.find_first_not_of(del, end)) != string::npos) {
        end = str.find_first_of(del, begin);
        tokens.push_back(str.substr(begin, end - begin));
    }

    return tokens;
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
DrawTable::DrawTable(size_t width)
: _width(width)
, _style(MYSQL_TABLE)
{
    _align.resize(width);
}

DrawTable::DrawTable(size_t width, Style style)
: _width(width)
, _style(style)
{
    _align.resize(width);
}

void DrawTable::set_align(size_t i, bool value = true)
{
    assert(_align.size() == _width);
    assert(i < _width);

    _align[i] = value;
}

void DrawTable::set_header(const vector<string>& str_list)
{
    assert(str_list.size() == _width);

    _header = str_list;
}

void DrawTable::add_row(const string& str)
{
    assert(_width == 1);

    _rows.resize(_rows.size() + 1);
    _rows.back().push_back(str);
}

void DrawTable::add_row(const string& str1, const string& str2)
{
    assert(_width == 2);

    _rows.resize(_rows.size() + 1);
    _rows.back().push_back(str1);
    _rows.back().push_back(str2);
}

void DrawTable::add_row(const vector<string>& str_list)
{
    assert(str_list.size() <= _width);

    if (str_list.size() != _width) {
        vector<string> tmp = str_list;
        tmp.resize(_width);
        _rows.push_back(tmp);
    } else {
        _rows.push_back(str_list);
    }
}

void DrawTable::draw() const
{
    switch(_style) {
        case MYSQL_TABLE:
            draw_mysql_table();
            break;

        default:
            draw_default();
    }
}
vector<size_t> DrawTable::get_widths() const
{
    vector<size_t> widths(_width, 0);
    if (!_header.empty()) {
        calculate_width(_header, widths);
    }

    for (const auto& row : _rows) {
        calculate_width(row, widths);
    }

    return widths;
}

void DrawTable::calculate_width(const vector<string>& row, vector<size_t>& widths) const
{
    assert(row.size() == widths.size());

    for (size_t i = 0; i < row.size(); ++i) {
        if (row[i].size() > widths[i]) {
            widths[i] = row[i].size();
        }
    }
}

string DrawTable::get_border(char c, size_t len) const
{
    string str;
    for (size_t i = 0; i < len; ++i) {
        str += c;
    }
    return str;
}

bool DrawTable::get_align(size_t i) const
{
    assert(_align.size() == _width);
    assert(i < _width);

    return _align[i];
}

void DrawTable::draw_mysql_table_border(const vector<size_t>& widths) const
{
    for (size_t i = 0; i < _width; ++i) {
        if (i == 0) {
            cout << "+";
        }
        cout << get_border('-', widths[i] + 2);
        cout << "+";
    }
    cout << endl;
}

void DrawTable::draw_row(const vector<size_t>& widths, const vector<string>& row, char del) const
{
    for (size_t i = 0; i < _width; ++i) {
        if (i == 0) {
            cout << del;
        }
        cout << " ";

        if (get_align(i)) {
            cout << right;
        } else {
            cout << left;
        }

        cout << setw(widths[i]) << row[i];
        cout << " ";
        cout << del;
    }
    cout << endl;
}

void DrawTable::draw_mysql_table() const
{
    vector<size_t> widths = get_widths();

    if (!_header.empty()) {
        draw_mysql_table_border(widths);
        draw_row(widths, _header, '|');
    }
    draw_mysql_table_border(widths);

    for (const auto& row : _rows) {
        draw_row(widths, row, '|');
    }
    draw_mysql_table_border(widths);
}

void DrawTable::draw_default() const
{
    for (const auto& row : _rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            if (i != 0)
                cout << " ";
            cout << row[i];
        }
        cout << endl;
    }
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
void error_msg_not_exist(const string& type, const string& name)
{
    error_msg(
        type +
        (type.empty() ? "" : " ") +
        "\'" + name + "\' doesn't exist"
    );
}

void error_msg(const string& str)
{
    cout << "Error: " << str << " !!" << endl;
}

void warning_msg(const string& str)
{
    cout << "Warning: " << str << " !!" << endl;
}

} // namespace jjjj222
