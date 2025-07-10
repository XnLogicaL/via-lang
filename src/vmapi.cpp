// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "vmapi.h"
#include "vmstr.h"
#include <cmath>

namespace via {

static Map<NativeFn, String> native_fn_ids{};

static String nativeid(NativeFn fn) {
  auto it = native_fn_ids.find(fn);
  if (it != native_fn_ids.end())
    return std::format("function {}", it->second);
  else
    return std::format("function <native@0x{:x}>", reinterpret_cast<uptr>(fn));
}

static std::string funcsig(const Closure* func) {
  return func->native ? nativeid(func->u.nat) : std::format("function {}", func->u.fun->id);
}

void push(State* S, Value&& val) {
  *(S->top++) = std::move(val);
}

void pop(State* S) {
  S->top--;
}

Value* get_local(State* S, usize offset) {
  CallInfo* ci = S->ci_top - 1;
  return (ci->base + offset - 1);
}

void set_local(State* VIA_RESTRICT S, usize offset, Value&& val) {
  *get_local(S, offset) = std::move(val);
}

void set_register(State* S, u16 reg, Value&& val) {
  S->rf.data[reg] = std::move(val);
}

Value* get_register(State* S, u16 reg) {
  return &S->rf.data[reg];
}

Value get_constant(State* S, usize index) {
  if (index >= S->H.consts.size)
    return nil;

  return value_clone(S, &S->H.consts.data[index]);
}

const char* type(Value* val) {
  switch (val->kind) {
  case VLK_NIL:
    return "nil";
  case VLK_INT:
    return "int";
  case VLK_FLOAT:
    return "float";
  case VLK_BOOLEAN:
    return "boolean";
  case VLK_ARRAY:
    return "array";
  case VLK_DICT:
    return "dict";
  case VLK_STRING:
    return "string";
  case VLK_FUNCTION:
    return "function";
  default:
    break;
  }

  VIA_UNREACHABLE();
}

const void* to_pointer(Value* val) {
  switch (val->kind) {
  case VLK_FUNCTION:
  case VLK_ARRAY:
  case VLK_DICT:
  case VLK_STRING:
    // This is technically UB... too bad!
    return reinterpret_cast<void*>(val->data->u.str);
  default:
    return NULL;
  }
}

template<const bool IsProtected>
void call_base(State* S, Closure* closure, usize nargs) {
  CallInfo ci;
  ci.base = S->top;
  ci.nargs = nargs;
  ci.protect = IsProtected;
  ci.closure = new Closure(*closure);

  if (closure->native) {
    // Native functions require manual positioning as they don't increment program counter with a
    // RET instruction or similar.
    ci.savedpc = S->pc + 1;
    *(S->ci_top++) = std::move(ci);
    ret(S, closure->u.nat(S));
  }
  else {
    // Functions are automatically positioned by RET instructions; no need to increment saved
    // program counter.
    ci.savedpc = S->pc;

    *(S->ci_top++) = std::move(ci);
    S->pc = closure->u.fun->code;
  }
}

void call(State* S, Closure* closure, usize nargs) {
  call_base<false>(S, closure, nargs);
}

void pcall(State* S, Closure* closure, usize nargs) {
  call_base<true>(S, closure, nargs);
}

void ret(State* S, Value&& retv) {
  CallInfo* ci = --S->ci_top;

  S->top = ci->base;
  S->pc = ci->savedpc;

  push(S, std::move(retv));
}

Value length(State* S, Value& val) {
  if (val.kind == VLK_STRING)
    return value_new(S, (int)val.data->u.str->size);

  return nil;
}

const char* to_string(State* S, Value* val) {
#define ALLOC(str)                                                                                 \
  ({                                                                                               \
    char* buf = (char*)S->ator.alloc_bytes(str.size() + 1);                                        \
    (void)std::strcpy(buf, str.c_str());                                                           \
    buf;                                                                                           \
  })

  switch (val->kind) {
  case VLK_STRING:
    return val->data->u.str->data;
  case VLK_INT:
    return ALLOC(std::to_string(val->data->u.i));
  case VLK_FLOAT:
    return ALLOC(std::to_string(val->data->u.f));
  case VLK_BOOLEAN:
    return val->data->u.b ? "true" : "false";
  case VLK_ARRAY:
  case VLK_DICT:
  case VLK_FUNCTION:
    // clang-format off
    return ALLOC(
      std::format(
        "<{}@0x{:x}>",
        type(val),
        (uptr)to_pointer(val)
      )
    ); // clang-format on

  default:
    return "nil";
  }


  VIA_UNREACHABLE();
#undef ALLOC
}

bool to_bool(State*, Value* val) {
  if (val->kind == VLK_BOOLEAN)
    return val->data->u.b;

  return val->kind != VLK_NIL;
}

int to_int(State* S, Value* val) {
  switch (val->kind) {
  case VLK_INT:
    return val->data->u.i;
  case VLK_FLOAT:
    return val->data->u.f;
  case VLK_BOOLEAN:
    return val->data->u.b;
  case VLK_STRING: {
    int res;
    const char* str = val->data->u.str->data;
    if (sscanf(str, "%d", &res) != 0)
      return res;

    error(S, "could not cast string into int");
    return -1;
  }
  default:
    break;
  }

  errorf(S, "could not cast {} into int", type(val));
  return -1;
}

float to_float(State* S, Value* val) {
  switch (val->kind) {
  case VLK_INT:
    return val->data->u.i;
  case VLK_FLOAT:
    return val->data->u.f;
  case VLK_BOOLEAN:
    return val->data->u.b;
  case VLK_STRING: {
    float res;
    const char* str = val->data->u.str->data;
    if (sscanf(str, "%f", &res) != 0)
      return res;

    error(S, "could not cast string into float");
    return -1;
  }
  default:
    break;
  }

  errorf(S, "could not cast {} into float", type(val));
  return -1;
}

} // namespace via
