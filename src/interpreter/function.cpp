//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "function.h"

namespace via {

IFunction::~IFunction() {
  delete[] upvs;
  delete[] ibp;
}

IFunction::IFunction(const IFunction& other)
  : ic(other.ic),
    upvc(other.upvc),
    call_data(other.call_data) {
  ibp = new Instruction[ic];
  upvs = new IUpValue[upvc];

  std::memcpy(ibp, other.ibp, ic * sizeof(Instruction));

  for (size_t i = 0; i < other.upvc; i++) {
    const IUpValue& upv = other.upvs[i];
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
