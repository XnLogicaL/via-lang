// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "vmapi.h"
#include "vmstr.h"
#include <cmath>

namespace via {

namespace vm {

static std::unordered_map<NativeFn, std::string> native_fn_ids{};

static std::string nativeid(NativeFn fn) {
  auto it = native_fn_ids.find(fn);
  if (it != native_fn_ids.end())
    return std::format("function {}", it->second);
  else
    return std::format("function <native@0x{:x}>", reinterpret_cast<uintptr_t>(fn));
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

Value* get_local(State* S, size_t offset) {
  CallInfo* ci = S->ci_top - 1;
  return (ci->base + offset - 1);
}

void set_local(State* VIA_RESTRICT S, size_t offset, Value&& val) {
  *get_local(S, offset) = std::move(val);
}

void set_register(State* S, uint16_t reg, Value&& val) {
  S->rf.data[reg] = std::move(val);
}

Value* get_register(State* S, uint16_t reg) {
  return &S->rf.data[reg];
}

Value get_constant(State* S, size_t index) {
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
void call_base(State* S, Closure* closure, size_t nargs) {
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

void call(State* S, Closure* closure, size_t nargs) {
  call_base<false>(S, closure, nargs);
}

void pcall(State* S, Closure* closure, size_t nargs) {
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
    return value_new(S, (int)val.data->u.str->data.size);
  else if (val.kind == VLK_ARRAY)
    return value_new(S, (int)array_size(S, val.data->u.arr));
  else if (val.kind == VLK_DICT)
    return value_new(S, (int)dict_size(S, val.data->u.dict));

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
    return val->data->u.str->data.data;
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
        (uintptr_t)to_pointer(val)
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
    const char* str = val->data->u.str->data.data;
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
    const char* str = val->data->u.str->data.data;
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

// Hashes a dictionary key using the FNV-1a hashing algorithm.
size_t dict_hash_key(const Dict* dict, const char* key) {
  constexpr size_t BASE = 2166136261u;
  constexpr size_t MOD = 16777619;

  size_t hash = BASE;

  while (*key)
    hash = (hash ^ *key++) * MOD;

  return hash % dict;
}

// Inserts a key-value pair into the hash table component of a given table_obj object.
void dict_set(const Dict* dict, const char* key, Value val) {
  size_t index = dict_hash_key(dict, key);
  if (index > dict->data_capacity) {
    // Handle relocation
  }

  Dict::HNode& node = dict->data[index];
  node.key = key;
  node.value = std::move(val);
  dict->csize.is_valid = false;
}

// Performs a look-up on the given table with a given key. Returns nullptr upon lookup failure.
Value* dict_get(const Dict* dict, const char* key) {
  size_t index = dict_hash_key(dict, key);
  if (index > dict->data_capacity) {
    return nullptr;
  }

  return &dict->data[index].value;
}

// Returns the real size_t of the hashtable component of the given table object.
size_t dict_size(const Dict* dict) {
  if (dict->csize.is_valid) {
    return dict->csize.cache;
  }

  size_t index = 0;
  for (; index < dict->data_capacity; index++) {
    Dict::HNode& obj = dict->data[index];
    if (obj.value.is_nil()) {
      break;
    }
  }

  dict->csize.cache = index;
  dict->csize.is_valid = true;

  return index;
}

// Checks if the given index is out of bounds of a given tables array component.
bool array_range_check(const Array* array, size_t index) {
  return array->data_capacity > index;
}

// Dynamically grows and relocates the array component of a given table_obj object.
void array_resize(Array* array) {
  size_t old_capacity = array->data_capacity;
  size_t new_capacity = old_capacity * 2;

  Value* old_location = array->data;
  Value* new_location = new Value[new_capacity];

  for (Value* ptr = old_location; ptr < old_location + old_capacity; ptr++) {
    size_t position = ptr - old_location;
    new_location[position] = std::move(*ptr);
  }

  array->data = new_location;
  array->data_capacity = new_capacity;

  delete[] old_location;
}

// Sets the given index of a table to a given value. Resizes the array component of the table_obj
// object if necessary.
void array_set(Array* array, size_t index, Value val) {
  if (!array_range_check(array, index)) {
    array_resize(array);
  }

  array->csize.is_valid = false;
  array->data[index] = std::move(val);
}

// Attempts to get the value at the given index of the array component of the table. Returns nullptr
// if the index is out of array capacity range.
Value* array_get(const Array* array, size_t index) {
  if (!array_range_check(array, index)) {
    return nullptr;
  }

  return &array->data[index];
}

// Returns the real size_t of the given tables array component.
size_t array_size(const Array* array) {
  if (array->csize.is_valid) {
    return array->csize.cache;
  }

  size_t size = 0;
  for (Value* ptr = array->data; ptr < array->data + array->data_capacity; ptr++) {
    if (!ptr->is_nil()) {
      size++;
    }
  }

  array->csize.cache = size;
  array->csize.is_valid = true;

  return size;
}

Instruction* label_get(State* S, size_t index) {
  return S->labels[index];
}

void label_load(State* S) {
  using enum Opcode;

  size_t index = 0;
  for (Instruction* pc = S->pc; 1; pc++) {
    if (pc->op == VOP_LBL)
      S->labels[index++] = pc;
    else if (pc->op == VOP_RET || pc->op == VOP_RETBF || pc->op == VOP_RETBT)
      break;
  }
}

Closure* create_main_function(Context& lctx) {
  Function fn;
  fn.id = "main";
  fn.line_number = 0;
  fn.code = lctx.bytecode.data();
  fn.code_size = lctx.bytecode.size();

  Callable c;
  c.type = Callable::Tag::Function;
  c.u = {.fn = fn};
  c.arity = 1;

  return new Closure(std::move(c));
}

void declare_core_lib(State* S) {
#define MAKE_VALUE(func, argc)                                                                     \
  ({                                                                                               \
    Callable c;                                                                                    \
    c.type = Callable::Tag::Native;                                                                \
    c.u = {.ntv = func};                                                                           \
    c.arity = argc;                                                                                \
    Value(new Closure(std::move(c)));                                                              \
  })

#define DECL_VALUE(name, func, arity)                                                              \
  do {                                                                                             \
    dict_set(S->globals, #name, MAKE_VALUE(func, arity));                                          \
    native_fn_ids[func] = #name;                                                                   \
  } while (0)

  static NativeFn core_print = [](State* S) -> Value {
    Value* arg0 = get_register(S, S->args);
    std::cout << arg0->to_cxx_string() << "\n";
    return Value();
  };

  static NativeFn core_error = [](State* S) -> Value {
    Value* arg0 = get_register(S, S->args);
    set_error_state(S, arg0->to_cxx_string());
    return Value();
  };

  DECL_VALUE(print, core_print, 1);
  DECL_VALUE(error, core_error, 1);

#undef MAKE_VALUE
#undef DECL_VALUE
}

} // namespace vm

} // namespace via
