#ifndef TYPE_H
#define TYPE_H

#include <type_traits>

#include <vector>
#include <array>
#include <string>

namespace jjjj222 {

using std::find;
using std::vector;
using std::array;
using std::string;
using std::pair;
using std::stringstream;

using std::tuple;
using std::true_type;
using std::false_type;
using std::get;
using std::stringstream;
using std::is_integral;
using std::is_floating_point;

//------------------------------------------------------------------------------
//   declare
//------------------------------------------------------------------------------
template<typename T> class vector_2d;

//------------------------------------------------------------------------------
//   type trait
//------------------------------------------------------------------------------
template<bool v> struct is_true : public false_type {};
template<> struct is_true<true> : public true_type {};

template<typename T> struct is_pair : public false_type {};
template<typename T, typename D> struct is_pair<pair<T,D>> : public true_type {};
template<typename T> struct is_string : public false_type {};
template<> struct is_string<string> : public true_type {};
template<typename T> struct is_c_string : public false_type {};
template<> struct is_c_string<const char*> : public true_type {};
template<typename... Args> struct is_tuple : public false_type {};
template<typename... Args> struct is_tuple<tuple<Args...>> : public true_type {};
template<typename T> struct is_container : public false_type {};
template<typename T> struct is_container<vector<T>> : public true_type {};
//template<typename T> struct is_container<queue<T>> : public true_type {};
template<typename T, size_t N> struct is_container<array<T, N>> : public true_type {};
template<typename T> struct is_container<vector_2d<T>> : public true_type {};
// TODO: support other container

template<typename T> struct is_built_in_type
    : public is_true<is_integral<T>::value || is_floating_point<T>::value> {};

template<typename T> struct is_container_support_dump : public false_type {};
template<typename T> struct is_container_support_dump<vector_2d<T>> : public true_type {};


} // namespace
#endif
