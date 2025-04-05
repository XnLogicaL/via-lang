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
  : ic(other.ic),
    upvc(other.upvc),
    call_data(other.call_data) {
  ibp = new instruction[ic];
  upvs = new upv_obj[upvc];

  std::memcpy(ibp, other.ibp, ic * sizeof(instruction));

  for (size_t i = 0; i < other.upvc; i++) {
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
