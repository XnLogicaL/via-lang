//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

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
  size_t sp;
  size_t argc;
  function_obj* caller;
  instruction* pc;
  instruction* ibp;
};

struct function_obj {
  size_t ic;
  size_t upvc = 0;
  upv_obj* upvs;
  instruction* ibp;
  call_info call_data;

  function_obj() = default;
  function_obj(const function_obj& other);
  ~function_obj();
};

} // namespace via

#endif
