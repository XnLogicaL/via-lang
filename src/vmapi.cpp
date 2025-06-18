// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "vmiapi.h"
#include "vmstr.h"
#include <cmath>

namespace via::impl {

static std::unordered_map<NativeFn, std::string> native_fn_ids{};

const InstructionData& __pcdata(const State* state, const Instruction* const pc) {
  Instruction* basepc = state->lctx.bytecode.data();
  size_t offset = pc - basepc;
  return state->lctx.bytecode_data[offset];
}

std::string __funcsig(const Callable& func) {
  return func.type == Callable::Tag::Function ? std::format("function {}", func.u.fn.id)
                                              : __nativeid(func.u.ntv);
}

std::string __nativeid(NativeFn fn) {
  auto it = native_fn_ids.find(fn);
  if (it != native_fn_ids.end())
    return std::format("function {}", it->second);
  else
    return std::format("function <native@0x{:x}>", reinterpret_cast<uintptr_t>(fn));
}

void __set_error_state(const State* state, const std::string& message) {
  const Callable& func = __current_callframe(const_cast<State*>(state))->closure->callee;
  state->err->has_error = true;
  state->err->funcsig = __funcsig(func);
  state->err->message = message;
}

void __clear_error_state(const State* state) {
  state->err->has_error = false;
}

bool __has_error(const State* state) {
  return state->err->has_error;
}

bool __handle_error(State* state) {
  CallStack* callstack = state->callstack;
  std::vector<std::string> funcsigs;

  for (int i = callstack->frames_count - 1; i >= 0; i--) {
    CallInfo* frame = &callstack->frames[i];
    if (frame->is_protected) {
      Value errv(new String(state->err->message.c_str()));
      __clear_error_state(state);
      __return(state, std::move(errv));
      return true;
    }

    funcsigs.push_back(__funcsig(frame->closure->callee));
    __pop_callframe(state);
  }

  std::ostringstream oss;
  oss << state->err->funcsig << ": " << state->err->message << "\n";

  for (size_t i = 0; const std::string& funcsig : funcsigs) {
    oss << " #" << i++ << ' ' << funcsig << "\n";
  }

  std::cout << oss.str();
  return false;
}

Value __get_constant(const State* state, size_t index) {
  if (index >= state->lctx.constants.size()) {
    return Value();
  }

  return state->lctx.constants.at(index).clone();
}

Value __type(const Value& val) {
  std::string _Temp = std::string(magic_enum::enum_name(val.type));
  const char* str = _Temp.c_str();
  return Value(new String(str));
}

std::string __type_cxx_string(const Value& val) {
  Value _Type = __type(val);
  return std::string(_Type.u.str->data);
}

void* __to_pointer(const Value& val) {
  switch (val.type) {
  case Value::Tag::Function:
  case Value::Tag::Array:
  case Value::Tag::Dict:
  case Value::Tag::String:
    // This is technically UB... too bad!
    return reinterpret_cast<void*>(val.u.str);
  default:
    return nullptr;
  }
}

CallInfo* __current_callframe(State* state) {
  CallStack* callstack = state->callstack;
  return &callstack->frames[callstack->frames_count - 1];
}

void __push_callframe(State* state, CallInfo&& frame) {
  if (state->callstack->frames_count >= 200) {
    __set_error_state(state, "Stack overflow");
    return;
  }

  CallStack* callstack = state->callstack;
  callstack->frames[callstack->frames_count++] = std::move(frame);
}

void __pop_callframe(State* state) {
  CallStack* callstack = state->callstack;
  callstack->frames_count--;
}

template<const bool IsProtected>
void __call_base(State* state, Closure* closure) {
  CallInfo cf;
  cf.is_protected = IsProtected;
  cf.closure = new Closure(*closure);

  if (closure->callee.type == Callable::Tag::Function) {
    // Functions are automatically positioned by RET instructions; no need to increment saved
    // program counter.
    cf.savedpc = state->pc;
    __push_callframe(state, std::move(cf));

    state->pc = closure->callee.u.fn.code;
  }
  else if (closure->callee.type == Callable::Tag::Native) {
    // Native functions require manual positioning as they don't increment program counter with a
    // RET instruction or similar.
    cf.savedpc = state->pc + 1;
    __push_callframe(state, std::move(cf));
    __return(state, closure->callee.u.ntv(state));
  }
}

void __call(State* state, Closure* closure) {
  __call_base<false>(state, closure);
}

void __pcall(State* state, Closure* closure) {
  __call_base<true>(state, closure);
}

void __return(State* VIA_RESTRICT state, Value&& retv) {
  CallInfo* current_frame = __current_callframe(state);
  state->pc = current_frame->savedpc;

  __set_register(state, state->ret, std::move(retv));
  __pop_callframe(state);
}

Value __length(Value& val) {
  if (val.is_string())
    return Value(static_cast<int>(val.u.str->data_size));
  else if (val.is_array() || val.is_dict()) {
    size_t len = val.is_array() ? __array_size(val.u.arr) : __dict_size(val.u.dict);
    return Value(static_cast<int>(len));
  }

  return Value();
}

int __length_cxx(Value& val) {
  Value len = __length(val);
  return len.is_nil() ? -1 : len.u.i;
}

Value __to_string(const Value& val) {
  using enum Value::Tag;

  if (val.is_string()) {
    return val.clone();
  }

  switch (val.type) {
  case Int: {
    std::string str = std::to_string(val.u.i);
    return Value(new struct String(str.c_str()));
  }
  case Float: {
    std::string str = std::to_string(val.u.f);
    return Value(new struct String(str.c_str()));
  }
  case Bool:
    return Value(new struct String(val.u.b ? "true" : "false"));
  case Array:
  case Dict: {
    auto type_str = magic_enum::enum_name(val.type);
    auto final_str = std::format("<{}@0x{:x}>", type_str, (uintptr_t)__to_pointer(val));

    return Value(new struct String(final_str.c_str()));
  }

  case Function: {
    std::string fnty = "native";
    std::string fnn = "";

    if (val.u.clsr->callee.type == Callable::Tag::Function) {
      fnty = "function ";
      fnn = val.u.clsr->callee.u.fn.id;
    }

    std::string final_str = std::format("<{}{}@0x{:x}>", fnty, fnn, (uintptr_t)val.u.clsr);
    return Value(new struct String(final_str.c_str()));
  }
  default:
    return Value(new struct String("nil"));
  }

  VIA_UNREACHABLE();
  return Value();
}

std::string __to_cxx_string(const Value& val) {
  Value str = __to_string(val);
  return std::string(str.u.str->data);
}

std::string __to_literal_cxx_string(const Value& val) {
  Value str = __to_string(val);
  std::string str_cpy = str.u.str->data;
  return ustresc(str_cpy);
}

Value __to_bool(const Value& val) {
  if (val.is_bool()) {
    return val.clone();
  }

  return Value(val.type != Value::Tag::Nil);
}

bool __to_cxx_bool(const Value& val) {
  return __to_bool(val).u.b;
}

Value __to_int(const State* V, const Value& val) {
  using enum Value::Tag;

  if (val.is_number()) {
    return val.clone();
  }

  switch (val.type) {
  case String: {
    const std::string& str = val.u.str->data;
    if (str.empty()) {
      return Value();
    }

    int int_result;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), int_result);
    if (ec == std::errc() && ptr == str.data() + str.size()) {
      return Value(int_result);
    }

    __set_error_state(V, "Failed to cast String into Int");
    return Value();
  }
  case Bool:
    return Value(static_cast<int>(val.u.b));
  default:
    break;
  }

  __set_error_state(V, std::format("Failed to cast {} into Int", magic_enum::enum_name(val.type)));
  return Value();
}

Value __to_float(const State* V, const Value& val) {
  using enum Value::Tag;

  if (val.is_number()) {
    return val.clone();
  }

  switch (val.type) {
  case String: {
    const std::string& str = val.u.str->data;
    if (str.empty()) {
      return Value();
    }

    float float_result;
    auto [ptr_f, ec_f] = std::from_chars(str.data(), str.data() + str.size(), float_result);
    if (ec_f == std::errc() && ptr_f == str.data() + str.size()) {
      return Value(float_result);
    }

    __set_error_state(V, "Failed to cast string to float");
    return Value();
  }
  case Bool:
    return Value(static_cast<float>(val.u.b));
  default:
    break;
  }

  __set_error_state(V, std::format("Failed to cast {} to float", magic_enum::enum_name(val.type)));
  return Value();
}

bool __compare(const Value& val0, const Value& val1) {
  using enum Value::Tag;

  if (val0.type != val1.type) {
    return false;
  }

  switch (val0.type) {
  case Int:
    return val0.u.i == val1.u.i;
  case Float:
    return val0.u.f == val1.u.f;
  case Bool:
    return val0.u.b == val1.u.b;
  case Nil:
    return true;
  case String:
    return !std::strcmp(val0.u.str->data, val1.u.str->data);
  default:
    return false;
  }

  VIA_UNREACHABLE();
  return false;
};


bool __compare_deep(const Value& val0, const Value& val1) {
  using enum Value::Tag;

  if (val0.type != val1.type) {
    return false;
  }

  switch (val0.type) {
  case Int:
    return val0.u.i == val1.u.i;
  case Float:
    return val0.u.f == val1.u.f;
  case Bool:
    return val0.u.b == val1.u.b;
  case Nil:
    return true;
  case String:
    return !std::strcmp(val0.u.str->data, val1.u.str->data);
  case Array: {
    if (__array_size(val0.u.arr) != __array_size(val1.u.arr)) {
      return false;
    }

    for (size_t i = 0; i < __array_size(val0.u.arr); i++) {
      Value* val = __array_get(val0.u.arr, i);
      Value* other = __array_get(val1.u.arr, i);

      if (!val->compare(*other)) {
        return false;
      }
    }

    return true;
  }
  // TODO: IMPLEMENT OTHER DATA TYPES
  default:
    return false;
  }

  VIA_UNREACHABLE();
  return false;
}

// Automatically resizes UpValue vector of closure by VIA_UPV_RESIZE_FACTOR.
void __closure_upvs_resize(Closure* closure) {
  uint32_t current_size = closure->upv_count;
  uint32_t new_size = current_size == 0 ? 8 : (current_size * 2);
  UpValue* new_location = new UpValue[new_size];

  // Check if upvalues are initialized
  if (current_size != 0) {
    for (UpValue* ptr = closure->upvs; ptr < closure->upvs + current_size; ptr++) {
      uint32_t offset = ptr - closure->upvs;
      new_location[offset] = std::move(*ptr);
    }

    delete[] closure->upvs;
  }

  // Update closure
  closure->upvs = new_location;
  closure->upv_count = new_size;
}

// Checks if a given index is within the bounds of the UpValue vector of the closure.
// Used for resizing.
bool __closure_upvs_range_check(Closure* closure, size_t index) {
  return closure->upv_count >= index;
}

// Attempts to retrieve UpValue at index <upv_id>.
// Returns nullptr if <upv_id> is out of UpValue vector bounds.
UpValue* __closure_upv_get(Closure* closure, size_t upv_id) {
  if (!__closure_upvs_range_check(closure, upv_id)) {
    return nullptr;
  }

  return &closure->upvs[upv_id];
}

// Dynamically reassigns UpValue at index <upv_id> the value <val>.
void __closure_upv_set(Closure* closure, size_t upv_id, Value& val) {
  UpValue* _Upv = __closure_upv_get(closure, upv_id);
  if (_Upv != nullptr) {
    if (_Upv->value != nullptr) {
      *_Upv->value = val.clone();
    }
    else {
      _Upv->value = &val;
    }

    _Upv->is_valid = true;
  }
}

// Loads closure bytecode by iterating over the Instruction pipeline.
// Handles sentinel/special opcodes like RET or CAPTURE while assembling closure.
void __closure_init(State* state, Closure* closure, size_t len) {
  size_t upvalues = 0;
  for (size_t i = 0; i < len; ++i) {
    if ((state->pc++)->op == Opcode::CAPTURE) {
      if (__closure_upvs_range_check(closure, upvalues)) {
        __closure_upvs_resize(closure);
      }

      operand_t idx = state->pc->b;
      Value* value;
      if (state->pc->a == 0)
        value = &__current_callframe(state)->locals[idx];
      else { // Upvalue is captured twice; automatically close it.
        UpValue* upv = &__current_callframe(state)->closure->upvs[idx];
        if (upv->is_valid && upv->is_open) {
          upv->heap_value = upv->value->clone();
          upv->value = &upv->heap_value;
          upv->is_open = false;
        }
        value = upv->value;
      }

      closure->upvs[upvalues++] = {
        .is_open = true,
        .is_valid = true,
        .value = value,
        .heap_value = Value(),
      };
    }
  }
}

// Moves upvalues of the current closure into the heap, "closing" them.
void __closure_close_upvalues(const Closure* closure) {
  // C Function replica compliance
  if (closure->upvs == nullptr) {
    return;
  }

  for (UpValue* upv = closure->upvs; upv < closure->upvs + closure->upv_count; upv++) {
    if (upv->is_valid && upv->is_open) {
      upv->heap_value = upv->value->clone();
      upv->value = &upv->heap_value;
      upv->is_open = false;
    }
  }
}

//  ================
// [ Table handling ]
//  ================

// Hashes a dictionary key using the FNV-1a hashing algorithm.
size_t __dict_hash_key(const Dict* dict, const char* key) {
  size_t hash = 2166136261u;
  while (*key) {
    hash = (hash ^ *key++) * 16777619;
  }

  return hash % dict->data_capacity;
}

// Inserts a key-value pair into the hash table component of a given table_obj object.
void __dict_set(const Dict* dict, const char* key, Value val) {
  size_t index = __dict_hash_key(dict, key);
  if (index > dict->data_capacity) {
    // Handle relocation
  }

  Dict::HNode& node = dict->data[index];
  node.key = key;
  node.value = std::move(val);
  dict->csize.is_valid = false;
}

// Performs a look-up on the given table with a given key. Returns nullptr upon lookup failure.
Value* __dict_get(const Dict* dict, const char* key) {
  size_t index = __dict_hash_key(dict, key);
  if (index > dict->data_capacity) {
    return nullptr;
  }

  return &dict->data[index].value;
}

// Returns the real size_t of the hashtable component of the given table object.
size_t __dict_size(const Dict* dict) {
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
bool __array_range_check(const Array* array, size_t index) {
  return array->data_capacity > index;
}

// Dynamically grows and relocates the array component of a given table_obj object.
void __array_resize(Array* array) {
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
void __array_set(Array* array, size_t index, Value val) {
  if (!__array_range_check(array, index)) {
    __array_resize(array);
  }

  array->csize.is_valid = false;
  array->data[index] = std::move(val);
}

// Attempts to get the value at the given index of the array component of the table. Returns nullptr
// if the index is out of array capacity range.
Value* __array_get(const Array* array, size_t index) {
  if (!__array_range_check(array, index)) {
    return nullptr;
  }

  return &array->data[index];
}

// Returns the real size_t of the given tables array component.
size_t __array_size(const Array* array) {
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

// ==========================================================
// Label handling
void __label_allocate(State* state, size_t count) {
  state->labels = new Instruction*[count];
}

void __label_deallocate(State* state) {
  if (state->labels) {
    delete[] state->labels;
    state->labels = nullptr;
  }
}

Instruction* __label_get(const State* state, size_t index) {
  return state->labels[index];
}

void __label_load(const State* state) {
  using enum Opcode;

  size_t index = 0;
  for (Instruction* pc = state->pc; 1; pc++) {
    if (pc->op == Opcode::LBL) {
      state->labels[index++] = pc;
    }
    else if (pc->op == RET || pc->op == RETBF || pc->op == RETBT) {
      break;
    }
  }
}

// ==========================================================
// Stack handling
void __push(State* state, Value&& val) {
  CallInfo* frame = __current_callframe(state);
  frame->locals[frame->locals_size++] = std::move(val);
}

void __drop(State* state) {
  CallInfo* frame = __current_callframe(state);
  Value dropped = std::move(frame->locals[frame->locals_size--]); // Thanks RAII
  dropped.reset();
}

Value* __get_local(State* VIA_RESTRICT state, size_t offset) {
  CallInfo* frame = __current_callframe(state);
  return &frame->locals[offset];
}

void __set_local(State* VIA_RESTRICT state, size_t offset, Value&& val) {
  CallInfo* frame = __current_callframe(state);
  frame->locals[offset] = std::move(val);
}

// ==========================================================
// Register handling
void __register_allocate(State* state) {
  state->spill_registers = new SpillRegFile();
}

void __register_deallocate(const State* state) {
  delete state->spill_registers;
}

void __set_register(const State* state, operand_t reg, Value&& val) {
  if VIA_LIKELY (reg < REGISTER_STACK_COUNT) {
    state->stack_registers.registers[reg] = std::move(val);
  }
  else {
    const operand_t offset = reg - REGISTER_STACK_COUNT;
    state->spill_registers->registers[offset] = std::move(val);
  }
}

Value* __get_register(const State* state, operand_t reg) {
  if VIA_LIKELY ((reg & 0xFF) == reg) {
    return &state->stack_registers.registers[reg];
  }
  else {
    const operand_t offset = reg - REGISTER_STACK_COUNT;
    return &state->spill_registers->registers[offset];
  }
}

//  ========================
// [ Main function handling ]
//  ========================
Closure* __create_main_function(Context& lctx) {
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

void __declare_core_lib(State* state) {
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
    __dict_set(state->globals, #name, MAKE_VALUE(func, arity));                                    \
    native_fn_ids[func] = #name;                                                                   \
  } while (0)

  static NativeFn core_print = [](State* state) -> Value {
    Value* arg0 = __get_register(state, state->args);
    std::cout << arg0->to_cxx_string() << "\n";
    return Value();
  };

  static NativeFn core_error = [](State* state) -> Value {
    Value* arg0 = __get_register(state, state->args);
    __set_error_state(state, arg0->to_cxx_string());
    return Value();
  };

  DECL_VALUE(__print, core_print, 1);
  DECL_VALUE(__error, core_error, 1);

#undef MAKE_VALUE
#undef DECL_VALUE
} // namespace via::impl

} // namespace via::impl
