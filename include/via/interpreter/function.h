// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _vl_function_h
#define _vl_function_h

#include "common.h"
#include "state.h"
#include "object.h"

namespace via {

struct upvalue {
  bool is_open = true;
  bool is_valid = false;

  value_obj* value = nullptr;
  value_obj heap_value = value_obj();
};

struct call_info {
  tfunction* caller;
  instruction* pc;
  instruction* ibp;
  instruction* iep;
  size_t sp;
  size_t argc;
};

struct tfunction {
  bool is_error_handler = false;
  bool is_vararg = false;

  call_info call_info;

  instruction* ibp = nullptr;
  instruction* iep = nullptr;

  upvalue* upvs = new upvalue[8];
  uint32_t upv_count = 8;

  tfunction() = default;
  tfunction(const tfunction& other);
  ~tfunction();
};

struct tcfunction {
  void (*data)(State*) = nullptr;
  bool is_error_handler = false;
};

} // namespace via

#endif
