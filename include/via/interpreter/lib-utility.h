// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_libutils_h
#define vl_has_header_libutils_h

#include "common.h"

#define vl_libassert(cond, message)                                                                \
  if (!(cond)) {                                                                                   \
    impl::__set_error_state(V, message);                                                           \
    return;                                                                                        \
  }

#define vl_libdeclfn(id)         void id(state* V)
#define vl_libdeclparam(id, idx) const value_obj& id = impl::__get_argument(V, idx);
#define vl_libret(val)           impl::__native_return(V, val);

#define vl_libwrapcfptr(ptr)         (value_obj(value_type::cfunction, new cfunction_obj(ptr)))
#define vl_libwrapprim(val)          (value_obj(val))
#define vl_libmapempl(map, key, val) map.emplace(hash_string_custom(key), val);

#endif
