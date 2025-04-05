// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef VIA_HAS_HEADER_FUNCTION_H
#define VIA_HAS_HEADER_FUNCTION_H

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
  size_t sp;
  size_t argc;
};

struct function_obj {
  size_t ic;
  size_t upvc;

  instruction* ibp;
  upv_obj* upvs;

  call_info call_data;

  function_obj()
    : upvc(8),
      upvs(new upv_obj[upvc]) {}

  function_obj(const function_obj& other);
  ~function_obj();
};

} // namespace via

#endif
