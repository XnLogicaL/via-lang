// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "bit-utility.h"
#include "file-io.h"
#include "api-impl.h"
#include "common.h"
#include "state.h"
#include "constant.h"
#include "csize.h"
#include "tdict.h"
#include "tarray.h"
#include "bytecode.h"
#include <cmath>

namespace via::impl {

void __set_error_state(const State* state, const std::string& message) {
  state->err->frame = state->callstack->frames + state->callstack->frames_count;
  state->err->message = std::move(message);
}

void __clear_error_state(const State* state) {
  state->err->frame = nullptr;
  state->err->message = "";
}

bool __has_error(const State* state) {
  return state->err->frame != nullptr;
}

bool __handle_error(const State* state) {
  CallStack* callstack = state->callstack;
  CallFrame* frames = callstack->frames;
  const size_t frame_count = callstack->frames_count;
  const CallFrame* error_frame = state->err->frame;

  auto get_callable_string = [](const Callable& callee) -> std::string {
    return callee.type == Callable::Tag::Function
      ? std::string(callee.u.fn->id)
      : std::format("<nativefn@0x{:x}>", reinterpret_cast<uintptr_t>(callee.u.ntv));
  };

  std::vector<CallFrame*> visited;

  // Traverse call frames from most recent to oldest
  for (size_t i = frame_count; i-- > 0;) {
    CallFrame* frame = &frames[i];
    bool can_handle_error = false; // TODO: Check if this frame can handle the error
    if (can_handle_error) {
      break;
    }
    visited.push_back(frame);
  }

  // Error can be handled
  if (!visited.empty() && visited.back() != &frames[0]) {
    // TODO: Actual error recovery here (e.g., jump to handler)
    return true;
  }
  else {
    // Unhandled error, print traceback
    std::ostringstream oss;
    oss << get_callable_string(error_frame->closure->callee) << ": " << state->err->message << "\n";
    oss << "callstack begin\n";

    for (size_t i = visited.size(); i-- > 0;) {
      const CallFrame* visited_frame = visited[i];
      const Callable& callee = visited_frame->closure->callee;
      oss << "  #" << visited.size() - i - 1 << " function " << get_callable_string(callee) << "\n";
    }

    oss << "callstack end\n";
    std::cout << oss.str();
    return false;
  }
}

Value __get_constant(const State* state, size_t index) {
  if (index >= state->unit_ctx.constants->size()) {
    return Value();
  }

  return state->unit_ctx.constants->at(index).clone();
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

CallFrame* __current_callframe(State* state) {
  CallStack* callstack = state->callstack;
  return &callstack->frames[callstack->frames_count - 1];
}

void __push_callframe(State* state, CallFrame&& frame) {
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

void __call(State* state, Closure* closure) {
  CallFrame frame;
  frame.closure = closure;
  frame.savedpc = state->pc;

  __push_callframe(state, std::move(frame));

  if (closure->callee.type == Callable::Tag::Function) {
    state->pc = closure->callee.u.fn->code;
  }
  else {
    closure->callee.u.ntv(state, closure); // Function should return on its own.
  }
}

void __return(State* VIA_RESTRICT state, Value&& retv) {
  CallFrame* current_frame = __current_callframe(state);
  state->pc = current_frame->savedpc;
  state->ret = std::move(retv);

  __closure_close_upvalues(current_frame->closure);
  __pop_callframe(state);
}

Value __length(Value& val) {
  if (val.is_string()) {
    return Value(static_cast<int>(val.u.str->data_size));
  }
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
    return Value(str.c_str());
  }
  case Float: {
    std::string str = std::to_string(val.u.f);
    return Value(str.c_str());
  }
  case Bool:
    return Value(val.u.b ? "true" : "false");
  case Array:
  case Dict: {
    auto type_str = magic_enum::enum_name(val.type);
    auto final_str = std::format("<{}@0x{:x}>", type_str, (uintptr_t)__to_pointer(val));

    return Value(final_str.c_str());
  }

  case Function: {
    std::string fnty = "native";
    std::string fnn = "";

    if (val.u.clsr->callee.type == Callable::Tag::Function) {
      fnty = "function ";
      fnn = val.u.clsr->callee.u.fn->id;
    }

    std::string final_str = std::format("<{}{}@0x{:x}>", fnty, fnn, (uintptr_t)val.u.clsr);
    return Value(final_str.c_str());
  }
  default:
    return Value("nil");
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
  return escape_string(str_cpy);
}

Value __to_bool(const Value& val) {
  if (val.is_bool()) {
    return val.clone();
  }

  return Value(val.type != Value::Tag::Nil, true);
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

    __set_error_state(V, "String -> Int cast failed");
    return Value();
  }
  case Bool:
    return Value(static_cast<int>(val.u.b));
  default:
    break;
  }

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

    __set_error_state(V, "String -> float cast failed");
    return Value();
  }
  case Bool:
    return Value(static_cast<float>(val.u.b));
  default:
    break;
  }

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
    return __to_pointer(val0) == __to_pointer(val1);
  }

  VIA_UNREACHABLE();
  return false;
};


// Automatically resizes UpValue vector of closure by VIA_UPV_RESIZE_FACTOR.
void __closure_upvs_resize(Closure* closure) {
  uint32_t current_size = closure->upv_count;
  uint32_t new_size = current_size == 0 ? 8 : (current_size * 2);
  UpValue* new_location = new UpValue[new_size];

  // Check if upvalues are initialized
  if (current_size != 0) {
    // Move upvalues to new location
    for (UpValue* ptr = closure->upvs; ptr < closure->upvs + current_size; ptr++) {
      uint32_t offset = ptr - closure->upvs;
      new_location[offset] = std::move(*ptr);
    }

    // Free old location
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

#if VIA_COMPILER == C_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvla-cxx-extension"
#endif

// Loads closure bytecode by iterating over the Instruction pipeline.
// Handles sentinel/special opcodes like RET or CAPTURE while assembling closure.
void __closure_bytecode_load(State* state, Closure* closure, size_t len) {
  // Skip CLOSURE instruction
  state->pc++;

  // Copy instructions from PC
#if VIA_COMPILER == C_MSVC // MSVC does not support VLA's
#include <malloc.h>        // For _alloca
  Instruction* buffer = static_cast<Instruction*>(_alloca(len * sizeof(Instruction)));
#else
  Instruction buffer[len];
#endif
  size_t upvalues = 0;
  for (size_t i = 0; i < len; ++i) {
    if (state->pc->op == Opcode::CAPTURE) {
      if (__closure_upvs_range_check(closure, upvalues++)) {
        __closure_upvs_resize(closure);
      }

      operand_t idx = state->pc->operand1;
      Value* value;
      if (state->pc->operand0 == 0) { // Capture local
        value = &__current_callframe(state)->locals[idx];
      }
      else { // Upvalue is captured twice; automatically close it.
        UpValue* upv = &__current_callframe(state)->closure->upvs[idx];
        if (upv->is_valid && upv->is_open) {
          upv->heap_value = upv->value->clone();
          upv->value = &upv->heap_value;
          upv->is_open = false;
        }
        value = upv->value;
      }

      closure->upvs[upvalues] = {
        .is_open = true,
        .is_valid = true,
        .value = value,
        .heap_value = Value(),
      };

      continue;
    }

    buffer[i] = *(state->pc++);
  }

  Function* fn = closure->callee.u.fn;
  // Means the function was default initialized, therefore manual allocation is needed.
  if (fn->code == nullptr) {
    fn->code = new Instruction[len];
    fn->code_size = len;
  }

  std::memcpy(fn->code, buffer, len * sizeof(Instruction));
}

#if VIA_COMPILER == C_CLANG
#pragma clang diagnostic pop
#endif

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
    else if (pc->op == RET || pc->op == RET0 || pc->op == RET1) {
      break;
    }
  }
}

// ==========================================================
// Stack handling
void __push(State* state, Value&& val) {
  CallFrame* frame = __current_callframe(state);
  frame->locals[frame->locals_size++] = std::move(val);
}

void __drop(State* state) {
  CallFrame* frame = __current_callframe(state);
  [[maybe_unused]] Value dropped = std::move(frame->locals[frame->locals_size--]); // Thanks RAII
}

Value* __get_local(State* VIA_RESTRICT state, size_t offset) {
  CallFrame* frame = __current_callframe(state);
  return &frame->locals[offset];
}

void __set_local(State* VIA_RESTRICT state, size_t offset, Value&& val) {
  CallFrame* frame = __current_callframe(state);
  frame->locals[offset] = std::move(val);
}

// ==========================================================
// Register handling
void __register_allocate(State* state) {
  state->spill_registers = new HeapRegHolder();
}

void __register_deallocate(const State* state) {
  delete state->spill_registers;
}

void __set_register(const State* state, operand_t reg, Value&& val) {
  if VIA_LIKELY (reg < VIA_STK_REGISTERS) {
    state->stack_registers.registers[reg] = std::move(val);
  }
  else {
    const operand_t offset = reg - VIA_STK_REGISTERS;
    state->spill_registers->registers[offset] = std::move(val);
  }
}

Value* __get_register(const State* state, operand_t reg) {
  if VIA_LIKELY ((reg & 0xFF) == reg) {
    return &state->stack_registers.registers[reg];
  }
  else {
    const operand_t offset = reg - VIA_STK_REGISTERS;
    return &state->spill_registers->registers[offset];
  }
}

//  ========================
// [ Main function handling ]
//  ========================
Closure* __create_main_function(BytecodeHolder& holder) {
  auto& raw = holder.get();

  Function* fn = new Function(raw.size());
  fn->id = "main";
  fn->line_number = 0;

  for (size_t i = 0; const Bytecode& data : raw) {
    fn->code[i++] = data.instruct;
  }

  Closure* main = new Closure;
  main->callee = Callable(fn, 0);

  return main;
}

} // namespace via::impl
