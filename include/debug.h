#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <sstream>
#include <iomanip>
#include <type_traits>

//#include <array>
//#include <vector>
#include <string>

#include "macro.h"
#include "util.h"
#include "type.h"

namespace jjjj222 {

using std::cout;
using std::endl;
//using std::vector;
//using std::array;
using std::pair;
using std::tuple;
using std::string;
using std::true_type;
using std::false_type;
using std::get;
using std::stringstream;
using std::setw;
using std::right;
using std::is_array;
using std::is_integral;
using std::is_floating_point;
using std::is_pointer;
using std::is_enum;

//------------------------------------------------------------------------------
//   declare
//------------------------------------------------------------------------------
//template<typename T> class vector_2d;

template<typename T> string dump_str(const T& data);
template<typename T> string dump_str_default(const T& data);
template<class... Args> string dump_str_tuple(const tuple<Args...>& data);
template<typename T, typename D> string dump_str_pair(const pair<T, D>& data);
template<typename T> string dump_str_container(const T& container);
template<typename T> string dump_str_built_in_type(const T& data);

template<typename T> void dump_print(const T& data);
template<typename T> void dump_print_container(const T& container);
template<typename T> void dump_print_default(const T& data);
template<typename T> void dump_print_dump_str(const T& data);

//------------------------------------------------------------------------------
//   type trait
//------------------------------------------------------------------------------
//template<bool v> struct is_true : public false_type {};
//template<> struct is_true<true> : public true_type {};
//
//template<typename T> struct is_pair : public false_type {};
//template<typename T, typename D> struct is_pair<pair<T,D>> : public true_type {};
//template<typename T> struct is_string : public false_type {};
//template<> struct is_string<string> : public true_type {};
//template<typename T> struct is_c_string : public false_type {};
//template<> struct is_c_string<const char*> : public true_type {};
//template<typename... Args> struct is_tuple : public false_type {};
//template<typename... Args> struct is_tuple<tuple<Args...>> : public true_type {};
//template<typename T> struct is_container : public false_type {};
//template<typename T> struct is_container<vector<T>> : public true_type {};
////template<typename T> struct is_container<queue<T>> : public true_type {};
//template<typename T, size_t N> struct is_container<array<T, N>> : public true_type {};
//template<typename T> struct is_container<vector_2d<T>> : public true_type {};
//// TODO: support other container
//
//template<typename T> struct is_built_in_type
//    : public is_true<is_integral<T>::value || is_floating_point<T>::value> {};
//
//template<typename T> struct is_container_support_dump : public false_type {};
//template<typename T> struct is_container_support_dump<vector_2d<T>> : public true_type {};

//------------------------------------------------------------------------------
//   dump str
//------------------------------------------------------------------------------
template<typename T, typename D>
string dump_str(const T& data1, const D& data2)
{
    string s = "(";
    s += dump_str(data1);
    s += ", ";
    s += dump_str(data2);
    return s + ")";
}

template<typename T>
string dump_str_impl_is_pointer(const T& ptr, const true_type&)
{
    if (ptr == NULL)
        return "<NULL>";

    return dump_str_impl_is_c_string(ptr, is_c_string<T>());
}

template<typename T>
string dump_str_impl_is_c_string(const T& c_string, const true_type&)
{
    return string(c_string);
}

template<typename T>
string dump_str_impl_is_c_string(const T& ptr, const false_type&)
{
    return dump_str(*ptr);
}

template<typename T>
string dump_str_impl_is_pointer(const T& data, const false_type&)
{
    return dump_str_impl_is_container(data, is_container<T>());
}

template<typename T>
string dump_str(const T& data)
{
    return dump_str_impl_is_pointer(data, is_pointer<T>());
}

template<typename T>
string dump_str_impl_is_container(const T& container, const true_type&)
{
    return dump_str_container(container);
}

template<typename T>
string dump_str_impl_is_array(const T& data, const true_type&)
{
    return dump_str_array(data);
}

template<typename T>
string dump_str_impl_is_array(const T& data, const false_type&)
{
    return dump_str_impl_is_pair(data, is_pair<T>());
}

template<typename T>
string dump_str_impl_is_container(const T& data, const false_type&)
{
    return dump_str_impl_is_array(data, is_array<T>());
}

template<typename T>
string dump_str_impl_is_pair(const T& data, const true_type&)
{
    return dump_str_pair(data);
}

template<typename T>
string dump_str_impl_is_pair(const T& data, const false_type&)
{
    return dump_str_impl_is_tuple(data, is_tuple<T>());
}

template<typename T>
string dump_str_impl_is_tuple(const T& data, const true_type&)
{
    return dump_str_tuple(data);
}

template<typename T>
string dump_str_impl_is_tuple(const T& data, const false_type&)
{
    //return dump_str_impl_is_built_in_type(data, is_built_in_type<T>());
    return dump_str_impl_is_string(data, is_string<T>());
}

template<typename T>
string dump_str_impl_is_string(const T& data, const true_type&)
{
    return data;
}

template<typename T>
string dump_str_impl_is_string(const T& data, const false_type&)
{
    return dump_str_impl_is_built_in_type(data, is_built_in_type<T>());
}

template<typename T>
string dump_str_impl_is_built_in_type(const T& data, const true_type&)
{
    return dump_str_built_in_type(data);
}

template<typename T>
string dump_str_impl_is_enum(const T& data, const true_type&)
{
    return dump_str((unsigned)data);
}

template<typename T>
string dump_str_impl_is_enum(const T& data, const false_type&)
{
    return dump_str_default(data);
}

template<typename T>
string dump_str_impl_is_built_in_type(const T& data, const false_type&)
{
    return dump_str_impl_is_enum(data, is_enum<T>());
}

//------------------------------------------------------------------------------
template<typename T>
string dump_str_array(const T& data)
{
    size_t size_of_array = sizeof(data) / sizeof(data[0]);
    string s = "{";
    foreach(i, size_of_array) {
        if (i != 0) {
            s += ", ";
        }
        s += dump_str(data[i]);
    }
    return s + "}";
}

template<typename T>
string dump_str_container(const T& container)
{
    string s = "{";
    foreach_cit(it, container) {
        if (it != container.cbegin()) {
            s += ", ";
        }
        s += dump_str(*it);
    }
    return s + "}";
}

template<typename Tuple, size_t N>
struct TupleDumper {
    static string dump_tuple_str(const Tuple& data) {
        string s;
        s += TupleDumper<Tuple, N-1>::dump_tuple_str(data);
        s += ", ";
        s += dump_str(get<N-1>(data));
        return s;
    }
};

template<typename Tuple>
struct TupleDumper<Tuple, 1> {
    static string dump_tuple_str(const Tuple& data) {
        return dump_str(get<0>(data));
    }
};

template<typename... Args>
string dump_str_tuple(const tuple<Args...>& data)
{
    string s = "<";
    s += TupleDumper<decltype(data), sizeof...(Args)>::dump_tuple_str(data);
    return s + ">";
}

template<typename T, typename D>
string dump_str_pair(const pair<T, D>& data)
{
    string s = "<";
    s += dump_str(data.first);
    s += ", ";
    s += dump_str(data.second);
    return s + ">";
}

template<typename T>
string dump_str_built_in_type(const T& data)
{
    stringstream sstm;
    sstm << data;
    return sstm.str();
}

template<typename T>
string dump_str_default(const T& data)
{
    return data.dump_str();
}

//------------------------------------------------------------------------------
//   dump print
//------------------------------------------------------------------------------
template<typename T>
void dump_print_impl_is_pointer(const T& ptr, const true_type&)
{
    if (ptr == NULL) {
        dump_print(string("<NULL>"));
        return;
    }
    dump_print(*ptr);
}

template<typename T>
void dump_print_impl_is_pointer(const T& data, const false_type&)
{
    dump_print_impl_is_container(data, is_container<T>());
}

template<typename T>
void dump_print(const T& data)
{
    dump_print_impl_is_pointer(data, is_pointer<T>());
}


template<typename T>
void dump_print_impl_is_container(const T& container, const true_type&)
{
    dump_print_impl_is_container_support_dump(container, is_container_support_dump<T>());
}

template<typename T>
void dump_print_impl_is_container(const T& data, const false_type&)
{
    //dump_print_impl_is_built_in_type(data, is_built_in_type<T>());
    dump_print_impl_is_string(data, is_string<T>());
}

template<typename T>
void dump_print_impl_is_string(const T& data, const true_type&)
{
    dump_print_dump_str(data);
    //dump_print_impl_is_built_in_type(data, is_built_in_type<T>());
}

template<typename T>
void dump_print_impl_is_string(const T& data, const false_type&)
{
    dump_print_impl_is_built_in_type(data, is_built_in_type<T>());
}

template<typename T>
void dump_print_impl_is_container_support_dump(const T& container, const true_type&)
{
    container.dump();
}

template<typename T>
void dump_print_impl_is_container_support_dump(const T& container, const false_type&)
{
    dump_print_container(container);
}

template<typename T>
void dump_print_impl_is_built_in_type(const T& data, const true_type&)
{
    dump_print_dump_str(data);
}

template<typename T>
void dump_print_impl_is_enum(const T& data, const true_type&)
{
    dump_print_dump_str(data);
}

template<typename T>
void dump_print_impl_is_enum(const T& data, const false_type&)
{
    dump_print_default(data);
}

template<typename T>
void dump_print_impl_is_built_in_type(const T& data, const false_type&)
{
    dump_print_impl_is_enum(data, is_enum<T>());
}


//------------------------------------------------------------------------------
template<typename T>
void dump_print_container(const T& container)
{
    size_t digits = num_of_digit(container.size());
    size_t i = 0;
    foreach_cit(it, container) {
        cout << setw(digits) << right << i++ << ": ";
        cout << dump_str(*it) << endl;
    }
}

template<typename T>
void dump_print_dump_str(const T& data)
{
    cout << dump_str(data) << endl;
}

template<typename T>
void dump_print_default(const T& data)
{
    data.dump();
}

//------------------------------------------------------------------------------
//   dump macro
//------------------------------------------------------------------------------
#define dump_normal(var) dump_impl(#var, var)
template<typename T>
void dump_impl(const string& name, const T& data)
{
    cout << name << ": " << dump_str(data) << endl;
}

#define dump_pretty(var) dump_pretty_impl(#var, var)
template<typename T>
void dump_pretty_impl(const string& name, const T& data)
{
    cout << name << ": " << endl;
    dump_print(data);
}

} // namespace
#endif
