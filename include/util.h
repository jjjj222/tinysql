#ifndef UTIL_H
#define UTIL_H

#include <algorithm>    // find

#include <vector>       // vector
#include <string>       // string
#include <sstream>      // stringstream
#include <utility>      // pair
#include <cassert>      // assert

#include "type.h"

namespace jjjj222 {

using std::find;
using std::vector;
using std::string;
using std::pair;
using std::stringstream;

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
template<typename T>
T num_of_digit(T n)
{
    T digit = 0;
    do {
        ++digit;
    } while (n /= 10);
    return digit;
}

template<typename T>
T str_to(const string& str)
{
    stringstream sstm(str);
    T value;
    sstm >> value;
    return value;
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
template<typename T>
void replace_all(T& container, typename T::value_type data1, typename T::value_type data2)
{
    for (auto& data : container) {
        if (data == data1) {
            data = data2;
        }
    }
} 

template<typename T>
void remove_from(T& container, typename T::value_type data)
{
    size_t end = 0;
    for (size_t i = 0; i < container.size(); ++i) {
        const auto& tmp = container[i];
        if (tmp != data) {
            if (i != end) {
                container[end] = tmp;
            }
            ++end;
        }
    }

    container.resize(end);
} 

template<typename T, typename D>
bool is_contain(const T& container, const D& data)
{
    return is_contain_is_container(container, data, is_container<D>());
}

template<typename T, typename D>
bool is_contain_is_container(const T& container, const D& c2, const true_type&)
{
    for (const auto& data : c2) {
        if (is_contain(container, data))
            return true;
    }
    return false;
}

template<typename T, typename D>
bool is_contain_is_container(const T& container, const D& data, const false_type&)
{
    return find(container.begin(), container.end(), data) != container.end();
}

template<typename T, typename D>
void add_into(T& container, const D& data)
{
    add_into_is_container(container, data, is_container<D>());
}

template<typename T, typename D>
void add_into_is_container(T& container, const D& container_2, const true_type&)
{
    for (const auto& data : container_2) {
        add_into(container, data);
    }
}

template<typename T, typename D>
void add_into_is_container(T& container, const D& data, const false_type&)
{
    container.push_back(data);
}

//------------------------------------------------------------------------------
//   
//------------------------------------------------------------------------------
template<typename T>
void delete_not_null(T& ptr)
{
    if (ptr != NULL)
        delete ptr;

    ptr = NULL;
}

template<typename T>
void delete_pos(vector<T*>& container, size_t pos)
{
    delete_not_null(container[pos]);

    for (size_t i = pos+1; i < container.size(); ++i) {
        container[i-1] = container[i];
    }
    container.resize(container.size() - 1);
}

template<typename T>
void delete_all(T& container)
{
    for (auto& data : container) {
        delete_not_null(data);
    }
}

template<typename T, typename D>
vector<pair<T, D>> vector_make_pair(const vector<T>& vec1, const vector<D>& vec2)
{
    assert(vec1.size() == vec2.size());

    vector<pair<T, D>> tmp;
    for (size_t i = 0; i < vec1.size(); ++i) {
        tmp.push_back(make_pair(vec1[i], vec2[i]));
    }

    return tmp;
}

template<typename T>
vector<vector<T>> vector_split(const vector<T>& vec, size_t num) {
    size_t count = 0;
    vector<T> tmp;
    vector<vector<T>> res;
    for (size_t i = 0; i < vec.size(); ++i) {
        tmp.push_back(vec[i]);
        ++count;
    
        if (count == num) {
            res.push_back(tmp);
            tmp.clear();
            count = 0;
        }
    }

    if (!tmp.empty()) {
        res.push_back(tmp);
    }

    return res;
}

template<typename T>
class UpdateTo
{
    public:
        UpdateTo(T* addr, const T& new_value)
            : _addr(addr), _pre_value(*addr)
        {
            *_addr = new_value;
        }

        ~UpdateTo()
        {
            *_addr = _pre_value;
        }

    private:
        T*  _addr;
        T   _pre_value;
};

//template<typename T>


} // namespace
#endif
