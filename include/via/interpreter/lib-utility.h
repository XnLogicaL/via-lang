//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#ifndef VIA_HAS_HEADER_LIBUTILS_H
#define VIA_HAS_HEADER_LIBUTILS_H

#include "common.h"

#define VIA_LIBASSERT(cond, message)                                                               \
  if (!(cond)) {                                                                                   \
    impl::__set_error_state(V, message);                                                           \
    return;                                                                                        \
  }

#define VIA_LIBDECLFN(id)         void id(state* V)
#define VIA_LIBDECLPARAM(id, idx) const IValue& id = impl::__get_argument(V, idx);
#define VIA_LIBRET(val)           impl::__native_return(V, val);

#endif
