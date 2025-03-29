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

vl_libdeclfn(base_error) {
  vl_libdeclparam(arg0, 0);
  __set_error_state(V, __to_cxx_string(V, arg0));
  vl_libret(nil);
}

vl_libdeclfn(base_assert) {
  vl_libdeclparam(arg0, 0);
  vl_libdeclparam(arg1, 1);

  if (!__to_cxx_bool(arg0)) {
    std::string err_cxx_str = std::format("assertion failed: {}", __to_cxx_string(V, arg1));

    value_obj err_val(new string_obj(V, err_cxx_str.c_str()));
    value_obj err_fn = vl_libwrapcfptr(base_error);

    __push(V, err_val.clone());
    __call(V, err_fn, 1);
  }

  vl_libret(nil);
}

vl_libdeclfn(open_baselib) {
  std::unordered_map<uint32_t, value_obj> base_properties;

  vl_libmapempl(base_properties, "print", vl_libwrapcfptr(base_print));
  vl_libmapempl(base_properties, "println", vl_libwrapcfptr(base_println));
  vl_libmapempl(base_properties, "error", vl_libwrapcfptr(base_error));
  vl_libmapempl(base_properties, "assert", vl_libwrapcfptr(base_assert));

  for (const auto& [ident, val] : base_properties) {
    __set_global(V, ident, val);
  }
}

} // namespace via::lib
