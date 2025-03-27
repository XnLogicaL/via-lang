// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_FUNCTION_H
#define _VIA_FUNCTION_H

#include "common.h"
#include "state.h"
#include "object.h"

VIA_NAMESPACE_BEGIN

struct UpValue {
  bool is_open  = true;
  bool is_valid = false;

  TValue* value      = nullptr;
  TValue  heap_value = TValue();
};

struct CallInfo {
  TFunction*   caller;
  Instruction* ip;
  Instruction* ibp;
  Instruction* iep;
  size_t       sp;
  size_t       argc;
};

struct TFunction {
  bool is_error_handler = false;
  bool is_vararg        = false;

  CallInfo call_info;

  Instruction* ibp = nullptr;
  Instruction* iep = nullptr;

  UpValue* upvs      = new UpValue[8];
  uint32_t upv_count = 8;

  VIA_DEFAULT_CONSTRUCTOR(TFunction);
  VIA_CUSTOM_DESTRUCTOR(TFunction);

  TFunction(const TFunction& other);
};

struct TCFunction {
  void (*data)(State*)  = nullptr;
  bool is_error_handler = false;
};

VIA_NAMESPACE_END

#endif
