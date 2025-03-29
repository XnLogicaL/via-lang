// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "function.h"

namespace via {

function_obj::~function_obj() {
  delete[] upvs;
  delete[] ibp;
}

function_obj::function_obj(const function_obj& other)
  : is_error_handler(other.is_error_handler),
    is_vararg(other.is_vararg),
    call_info(other.call_info),
    upv_count(other.upv_count) {
  size_t bytecode_len = iep - ibp;

  ibp = new instruction[bytecode_len];
  upvs = new upv_obj[upv_count];

  std::memcpy(ibp, other.ibp, bytecode_len);

  for (size_t i = 0; i < other.upv_count; i++) {
    const upv_obj& upv = other.upvs[i];
    if (!upv.is_valid) {
      continue;
    }

    upvs[i] = {
      .is_open = upv.is_open,
      .is_valid = true,
      .value = upv.is_open ? upv.value : &upvs[i].heap_value,
      .heap_value = upv.heap_value.clone(),
    };
  }
}

} // namespace via
