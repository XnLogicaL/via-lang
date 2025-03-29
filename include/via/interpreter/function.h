// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_function_h
#define vl_has_header_function_h

#include "common.h"
#include "state.h"
#include "object.h"

namespace via {

struct upv_obj {
  bool is_open = true;
  bool is_valid = false;

  value_obj* value = nullptr;
  value_obj heap_value = value_obj();
};

struct call_info {
  function_obj* caller;
  instruction* pc;
  instruction* ibp;
  instruction* iep;
  size_t sp;
  size_t argc;
};

struct function_obj {
  bool is_error_handler = false;
  bool is_vararg = false;

  call_info call_info;

  instruction* ibp = nullptr;
  instruction* iep = nullptr;

  upv_obj* upvs = new upv_obj[8];
  uint32_t upv_count = 8;

  function_obj() = default;
  function_obj(const function_obj& other);
  ~function_obj();
};

struct cfunction_obj {
  void (*data)(state*) = nullptr;
  bool is_error_handler = false;
};

} // namespace via

#endif
