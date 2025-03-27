// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_LIBUTILS_H
#define _VIA_LIBUTILS_H

#include "common.h"

#define VIA_LIB_ASSERT(cond, message)                                                              \
  if (!(cond)) {                                                                                   \
    impl::__set_error_state(V, message);                                                           \
    return;                                                                                        \
  }

#define VIA_LIB_DECL_FUNCTION(id)       void id(State* V)
#define VIA_LIB_DECL_PARAMETER(id, idx) const TValue& id = impl::__get_argument(V, idx);
#define VIA_LIB_RETURN(val)             impl::__native_return(V, val);

#define VIA_LIB_WRAP_CFPTR(ptr)            (TValue(ValueType::cfunction, new TCFunction(ptr)))
#define VIA_LIB_WRAP_PRIM(val)             (TValue(val))
#define VIA_LIB_MAP_EMPLACE(map, key, val) map.emplace(hash_string_custom(key), val);

#endif
