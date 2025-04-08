//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "lib-base.h"
#include "api-impl.h"
#include "lib-utility.h"

namespace via::lib {

using namespace impl;

VIA_LIBDECLFN(base_print) {
  size_t i = 0;
  while (i < V->frame->call_data.argc) {
    VIA_LIBDECLPARAM(argx, i++);
    std::cout << __to_cxx_string(argx);
  }

  std::cout << "\n";

  VIA_LIBRET(IValue());
}

VIA_LIBDECLFN(open_baselib) {
  V->glb->gtable.set("print", IValue(base_print));
}

} // namespace via::lib
