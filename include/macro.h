#ifndef MACRO_H
#define MACRO_H

//------------------------------------------------------------------------------
//   macro
//------------------------------------------------------------------------------

#define foreach_cit(it, container) for (auto it = container.cbegin(); it != container.cend(); ++it)
#define foreach_it(it, container) for (auto it = container.begin(); it != container.end(); ++it)
#define foreach_i(i, array) for (size_t i = 0; i < array.size(); ++i)
#define foreach_rev_i(i, array) for (size_t i = array.size(); i-- > 0;)
#define foreach(i, n) for (size_t i = 0; i < n; ++i)
#define foreach_rev(i, n) for (size_t i = n; i-- > 0;)

#endif
