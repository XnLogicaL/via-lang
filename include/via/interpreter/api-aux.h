//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

// TODO: UPDATE DOCUMENTATION IN THIS FILE
#ifndef VIA_HAS_HEADER_VAUX_H
#define VIA_HAS_HEADER_VAUX_H

#include "memory-utility.h"
#include "common.h"
#include "function.h"

//  ===========
// [ api-aux.h ]
//  ===========
namespace via::impl {
//  ==================
// [ Closure handling ]
//  ==================

// Automatically resizes upv_obj vector of closure by VIA_UPV_RESIZE_FACTOR.
VIA_IMPLEMENTATION void __closure_upvs_resize(function_obj* closure) {
  uint32_t current_size = closure->upvc;
  uint32_t new_size = current_size == 0 ? 8 : (current_size * 2);
  upv_obj* new_location = new upv_obj[new_size];

  // Check if upvalues are initialized
  if (current_size != 0) {
    // Move upvalues to new location
    for (upv_obj* ptr = closure->upvs; ptr < closure->upvs + current_size; ptr++) {
      uint32_t offset = ptr - closure->upvs;
      new_location[offset] = std::move(*ptr);
    }

    // Free old location
    delete[] closure->upvs;
  }

  // Update closure
  closure->upvs = new_location;
  closure->upvc = new_size;
}

// Checks if a given index is within the bounds of the upv_obj vector of the closure.
// Used for resizing.
VIA_IMPLEMENTATION bool __closure_upvs_range_check(function_obj* closure, size_t index) {
  return closure->upvc >= index;
}

// Attempts to retrieve upv_obj at index <upv_id>.
// Returns nullptr if <upv_id> is out of upv_obj vector bounds.
VIA_IMPLEMENTATION upv_obj* __closure_upv_get(function_obj* closure, size_t upv_id) {
  if (!__closure_upvs_range_check(closure, upv_id)) {
    return nullptr;
  }

  return &closure->upvs[upv_id];
}

// Dynamically reassigns upv_obj at index <upv_id> the value <val>.
VIA_IMPLEMENTATION void __closure_upv_set(function_obj* closure, size_t upv_id, value_obj& val) {
  upv_obj* _Upv = __closure_upv_get(closure, upv_id);
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

// Loads closure bytecode by iterating over the instruction pipeline.
// Handles sentinel/special opcodes like RET or CAPTURE while assembling closure.
VIA_IMPLEMENTATION void __closure_bytecode_load(state* state, function_obj* closure, size_t len) {
  // Skip NEWCLSR instruction
  state->pc++;

  // Copy instructions from PC
  instruction buffer[len];
  for (size_t i = 0; i < len; ++i) {
    buffer[i] = *(state->pc++);
  }

  closure->ibp = new instruction[len];
  closure->ic = len;

  std::memcpy(closure->ibp, buffer, len * sizeof(instruction));

  // Capture upvalues from the current stack frame
  for (value_obj* _Stk_id = state->sbp + state->sp - 1; _Stk_id >= state->sbp; --_Stk_id) {
    size_t _Pos = _Stk_id - state->sbp;

    if (__closure_upvs_range_check(closure, _Pos)) {
      __closure_upvs_resize(closure); // must grow upvs to _Pos + 1 or more
    }

    closure->upvs[_Pos] = {
      .is_open = true,
      .value = _Stk_id,
    };
  }
}

#if VIA_COMPILER == C_CLANG
#pragma clang diagnostic pop
#endif

// Moves upvalues of the current closure into the heap, "closing" them.
VIA_IMPLEMENTATION void __closure_close_upvalues(function_obj* closure) {
  // C Function replica compliance
  if (closure->upvs == nullptr) {
    return;
  }

  for (upv_obj* upv = closure->upvs; upv < closure->upvs + closure->upvc; upv++) {
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
VIA_IMPLEMENTATION size_t __dict_hash_key(const dict_obj* dict, const char* key) {
  size_t hash = 2166136261u;
  while (*key) {
    hash = (hash ^ *key++) * 16777619;
  }

  return hash % dict->capacity;
}

// Inserts a key-value pair into the hash table component of a given table_obj object.
VIA_IMPLEMENTATION void __dict_set(dict_obj* dict, const char* key, value_obj val) {
  size_t index = __dict_hash_key(dict, key);
  if (index > dict->capacity) {
    // Handle relocation
  }

  hash_node& node = dict->data[index];
  node.key = key;
  node.value = val.move();

  dict->size_cache_valid = false;
}

// Performs a look-up on the given table with a given key. Returns nullptr upon lookup failure.
VIA_IMPLEMENTATION value_obj* __dict_get(const dict_obj* dict, const char* key) {
  size_t index = __dict_hash_key(dict, key);
  if (index > dict->capacity) {
    return nullptr;
  }

  return &dict->data[index].value;
}

// Returns the real size_t of the hashtable component of the given table object.
VIA_IMPLEMENTATION size_t __dict_size(const dict_obj* dict) {
  if (dict->size_cache_valid) {
    return dict->size_cache;
  }

  size_t index = 0;
  for (; index < dict->capacity; index++) {
    hash_node& obj = dict->data[index];
    if (obj.value.is_nil()) {
      break;
    }
  }

  dict->size_cache = index;
  dict->size_cache_valid = true;

  return index;
}

// Checks if the given index is out of bounds of a given tables array component.
VIA_IMPLEMENTATION bool __array_range_check(const array_obj* array, size_t index) {
  return array->capacity > index;
}

// Dynamically grows and relocates the array component of a given table_obj object.
VIA_IMPLEMENTATION void __array_resize(array_obj* array) {
  size_t old_capacity = array->capacity;
  size_t new_capacity = old_capacity * 2;

  value_obj* old_location = array->data;
  value_obj* new_location = new value_obj[new_capacity];

  for (value_obj* ptr = old_location; ptr < old_location + old_capacity; ptr++) {
    size_t position = ptr - old_location;
    new_location[position] = std::move(*ptr);
  }

  array->data = new_location;
  array->capacity = new_capacity;

  delete[] old_location;
}

// Sets the given index of a table to a given value. Resizes the array component of the table_obj
// object if necessary.
VIA_IMPLEMENTATION void __array_set(array_obj* array, size_t index, value_obj val) {
  if (!__array_range_check(array, index)) {
    __array_resize(array);
  }

  array->size_cache_valid = false;
  array->data[index] = val.move();
}

// Attempts to get the value at the given index of the array component of the table. Returns nullptr
// if the index is out of array capacity range.
VIA_IMPLEMENTATION value_obj* __array_get(const array_obj* array, size_t index) {
  if (!__array_range_check(array, index)) {
    return nullptr;
  }

  return &array->data[index];
}

// Returns the real size_t of the given tables array component.
VIA_IMPLEMENTATION size_t __array_size(const array_obj* array) {
  if (array->size_cache_valid) {
    return array->size_cache;
  }

  size_t size = 0;
  for (value_obj* ptr = array->data; ptr < array->data + array->capacity; ptr++) {
    if (!ptr->is_nil()) {
      size++;
    }
  }

  array->size_cache = size;
  array->size_cache_valid = true;

  return size;
}

// ==========================================================
// Label handling
VIA_IMPLEMENTATION void __label_allocate(state* state, size_t count) {
  state->labels = new instruction*[count];
}

VIA_IMPLEMENTATION void __label_deallocate(state* state) {
  if (state->labels) {
    delete[] state->labels;
    state->labels = nullptr;
  }
}

VIA_IMPLEMENTATION instruction* __label_get(state* state, size_t index) {
  return state->labels[index];
}

VIA_IMPLEMENTATION void __label_load(state* state) {
  size_t index = 0;
  for (instruction* pc = state->ibp; 1; pc++) {
    if (pc->op == opcode::LBL) {
      state->labels[index++] = pc;
    }
    else if (pc->op == opcode::EXIT) {
      break;
    }
  }
}

// ==========================================================
// Stack handling
VIA_IMPLEMENTATION void __stack_allocate(state* state) {
  state->sbp = new value_obj[VIA_VMSTACKSIZE];
}

VIA_IMPLEMENTATION void __stack_deallocate(state* state) {
  delete[] state->sbp;
}

VIA_IMPLEMENTATION void __push(state* state, value_obj val) {
  state->sbp[state->sp++] = std::move(val);
}

VIA_IMPLEMENTATION value_obj __pop(state* state) {
  return state->sbp[state->sp--].move();
}

VIA_IMPLEMENTATION void __drop(state* state) {
  value_obj& dropped = state->sbp[state->sp--];
  dropped.reset();
}

VIA_IMPLEMENTATION value_obj& __get_stack(state* state, size_t offset) {
  return state->sbp[offset];
}

VIA_IMPLEMENTATION void __set_stack(state* state, size_t offset, value_obj val) {
  state->sbp[offset] = val.move();
}

VIA_IMPLEMENTATION value_obj __get_argument(state* VIA_RESTRICT state, size_t offset) {
  if (offset >= state->frame->call_data.argc) {
    return value_obj();
  }

  // Compute stack offset in reverse order
  const operand_t stk_offset = state->frame->call_data.sp - state->frame->call_data.argc + offset;
  const value_obj& val = state->sbp[stk_offset];

  return val.clone();
}

// ==========================================================
// Register handling
VIA_IMPLEMENTATION void __register_allocate(state* state) {
  state->spill_registers = new spill_registers_t();
}

VIA_IMPLEMENTATION void __register_deallocate(state* state) {
  delete state->spill_registers;
}

VIA_OPTIMIZE void __set_register(state* state, operand_t reg, value_obj val) {
  if VIA_LIKELY (reg < VIA_STK_REGISTERS) {
    state->stack_registers.registers[reg] = val.move();
  }
  else {
    const operand_t offset = reg - VIA_STK_REGISTERS;
    state->spill_registers->registers[offset] = val.move();
  }
}

VIA_OPTIMIZE value_obj* __get_register(state* state, operand_t reg) {
  if VIA_LIKELY ((reg & 0xFF) == reg) {
    return &state->stack_registers.registers[reg];
  }
  else {
    const operand_t offset = reg - VIA_STK_REGISTERS;
    return &state->spill_registers->registers[offset];
  }
}

} // namespace via::impl

#endif
