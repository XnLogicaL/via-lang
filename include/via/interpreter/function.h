//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#ifndef VIA_HAS_HEADER_FUNCTION_H
#define VIA_HAS_HEADER_FUNCTION_H

#include "common.h"
#include "state.h"
#include "object.h"

namespace via {

struct IUpValue {
  bool is_open = true;
  bool is_valid = false;

  IValue* value = nullptr;
  IValue heap_value = IValue();
};

struct ICallInfo {
  size_t sp;
  size_t argc;
  IFunction* caller;
  Instruction* pc;
  Instruction* ibp;
};

struct IFunction {
  size_t ic;
  size_t upvc = 0;
  IUpValue* upvs;
  Instruction* ibp;
  ICallInfo call_data;

  IFunction() = default;
  IFunction(const IFunction& other);
  ~IFunction();
};

} // namespace via

#endif
