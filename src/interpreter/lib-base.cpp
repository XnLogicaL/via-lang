// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "lib-base.h"
#include "api-impl.h"
#include "lib-utility.h"

namespace via::lib {

static value_obj nil = value_obj();

using namespace impl;

VIA_LIBDECLFN(base_print) {
  size_t i = 0;
  while (i < V->frame->call_data.argc) {
    VIA_LIBDECLPARAM(argx, i++);
    std::cout << __to_cxx_string(argx);
  }

  std::cout << std::endl;

  VIA_LIBRET(nil);
}

VIA_LIBDECLFN(open_baselib) {
  V->glb->gtable.set("print", value_obj(base_print));
}

} // namespace via::lib
