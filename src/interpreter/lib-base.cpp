// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "lib-base.h"
#include "api-impl.h"
#include "lib-utility.h"

namespace via::lib {

static value_obj nil = value_obj();

using namespace impl;

vl_libdeclfn(base_print) {
  size_t i = 0;
  while (i < V->frame->call_info.argc) {
    vl_libdeclparam(argx, i++);
    std::cout << __to_cxx_string(V, argx);
  }

  std::cout << std::flush;

  vl_libret(nil);
}

vl_libdeclfn(base_println) {
  size_t i = 0;
  while (i < V->frame->call_info.argc) {
    vl_libdeclparam(argx, i++);
    std::cout << __to_cxx_string(V, argx);
  }

  std::cout << std::endl;

  vl_libret(nil);
}

vl_libdeclfn(open_baselib) {
  V->glb->gtable.set("print", value_obj(base_print));
  V->glb->gtable.set("println", value_obj(base_println));
}

} // namespace via::lib
