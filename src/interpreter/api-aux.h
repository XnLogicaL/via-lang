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

static IValue nil;

//  ==================
// [ Closure handling ]
//  ==================

// Automatically resizes IUpValue vector of closure by VIA_UPV_RESIZE_FACTOR.
VIA_IMPLEMENTATION void __closure_upvs_resize(IFunction* closure) {
  uint32_t current_size = closure->upvc;
  uint32_t new_size = current_size == 0 ? 8 : (current_size * 2);
  IUpValue* new_location = new IUpValue[new_size];

  // Check if upvalues are initialized
  if (current_size != 0) {
    // Move upvalues to new location
    for (IUpValue* ptr = closure->upvs; ptr < closure->upvs + current_size; ptr++) {
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

// Checks if a given index is within the bounds of the IUpValue vector of the closure.
// Used for resizing.
VIA_IMPLEMENTATION bool __closure_upvs_range_check(IFunction* closure, size_t index) {
  return closure->upvc >= index;
}

// Attempts to retrieve IUpValue at index <upv_id>.
// Returns nullptr if <upv_id> is out of IUpValue vector bounds.
VIA_IMPLEMENTATION IUpValue* __closure_upv_get(IFunction* closure, size_t upv_id) {
  if (!__closure_upvs_range_check(closure, upv_id)) {
    return nullptr;
  }

  return &closure->upvs[upv_id];
}

// Dynamically reassigns IUpValue at index <upv_id> the value <val>.
VIA_IMPLEMENTATION void __closure_upv_set(IFunction* closure, size_t upv_id, IValue& val) {
  IUpValue* _Upv = __closure_upv_get(closure, upv_id);
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
VIA_IMPLEMENTATION void __closure_bytecode_load(IState* state, IFunction* closure, size_t len) {
  // Skip NEWCLSR instruction
  state->pc++;

  // Copy instructions from PC
#if VIA_COMPILER == C_MSVC // MSVC does not support VLA's
#include <malloc.h>        // For _alloca
  Instruction* buffer = static_cast<Instruction*>(_alloca(len * sizeof(Instruction)));
#else
  Instruction buffer[len];
#endif
  for (size_t i = 0; i < len; ++i) {
    buffer[i] = *(state->pc++);
  }

  closure->ibp = new Instruction[len];
  closure->ic = len;

  std::memcpy(closure->ibp, buffer, len * sizeof(Instruction));

  // Capture upvalues from the current stack frame
  for (IValue* _Stk_id = state->sbp + state->sp - 1; _Stk_id >= state->sbp; --_Stk_id) {
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
VIA_IMPLEMENTATION void __closure_close_upvalues(const IFunction* closure) {
  // C IFunction replica compliance
  if (closure->upvs == nullptr) {
    return;
  }

  for (IUpValue* upv = closure->upvs; upv < closure->upvs + closure->upvc; upv++) {
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
VIA_IMPLEMENTATION size_t __dict_hash_key(const IDict* dict, const char* key) {
  size_t hash = 2166136261u;
  while (*key) {
    hash = (hash ^ *key++) * 16777619;
  }

  return hash % dict->capacity;
}

// Inserts a key-value pair into the hash table component of a given table_obj object.
VIA_IMPLEMENTATION void __dict_set(const IDict* dict, const char* key, IValue val) {
  size_t index = __dict_hash_key(dict, key);
  if (index > dict->capacity) {
    // Handle relocation
  }

  IHashNode& node = dict->data[index];
  node.key = key;
  node.value = std::move(val);

  dict->size_cache_valid = false;
}

// Performs a look-up on the given table with a given key. Returns nullptr upon lookup failure.
VIA_IMPLEMENTATION IValue* __dict_get(const IDict* dict, const char* key) {
  size_t index = __dict_hash_key(dict, key);
  if (index > dict->capacity) {
    return &nil;
  }

  return &dict->data[index].value;
}

// Returns the real size_t of the hashtable component of the given table object.
VIA_IMPLEMENTATION size_t __dict_size(const IDict* dict) {
  if (dict->size_cache_valid) {
    return dict->size_cache;
  }

  size_t index = 0;
  for (; index < dict->capacity; index++) {
    IHashNode& obj = dict->data[index];
    if (obj.value.is_nil()) {
      break;
    }
  }

  dict->size_cache = index;
  dict->size_cache_valid = true;

  return index;
}

// Checks if the given index is out of bounds of a given tables array component.
VIA_IMPLEMENTATION bool __array_range_check(const IArray* array, size_t index) {
  return array->capacity > index;
}

// Dynamically grows and relocates the array component of a given table_obj object.
VIA_IMPLEMENTATION void __array_resize(IArray* array) {
  size_t old_capacity = array->capacity;
  size_t new_capacity = old_capacity * 2;

  IValue* old_location = array->data;
  IValue* new_location = new IValue[new_capacity];

  for (IValue* ptr = old_location; ptr < old_location + old_capacity; ptr++) {
    size_t position = ptr - old_location;
    new_location[position] = std::move(*ptr);
  }

  array->data = new_location;
  array->capacity = new_capacity;

  delete[] old_location;
}

// Sets the given index of a table to a given value. Resizes the array component of the table_obj
// object if necessary.
VIA_IMPLEMENTATION void __array_set(IArray* array, size_t index, IValue val) {
  if (!__array_range_check(array, index)) {
    __array_resize(array);
  }

  array->size_cache_valid = false;
  array->data[index] = std::move(val);
}

// Attempts to get the value at the given index of the array component of the table. Returns nullptr
// if the index is out of array capacity range.
VIA_IMPLEMENTATION IValue* __array_get(const IArray* array, size_t index) {
  if (!__array_range_check(array, index)) {
    return &nil;
  }

  return &array->data[index];
}

// Returns the real size_t of the given tables array component.
VIA_IMPLEMENTATION size_t __array_size(const IArray* array) {
  if (array->size_cache_valid) {
    return array->size_cache;
  }

  size_t size = 0;
  for (IValue* ptr = array->data; ptr < array->data + array->capacity; ptr++) {
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
VIA_IMPLEMENTATION void __label_allocate(IState* state, size_t count) {
  state->labels = new Instruction*[count];
}

VIA_IMPLEMENTATION void __label_deallocate(IState* state) {
  if (state->labels) {
    delete[] state->labels;
    state->labels = nullptr;
  }
}

VIA_IMPLEMENTATION Instruction* __label_get(const IState* state, size_t index) {
  return state->labels[index];
}

VIA_IMPLEMENTATION void __label_load(const IState* state) {
  size_t index = 0;
  for (Instruction* pc = state->ibp; 1; pc++) {
    if (pc->op == IOpCode::LBL) {
      state->labels[index++] = pc;
    }
    else if (pc->op == IOpCode::EXIT) {
      break;
    }
  }
}

// ==========================================================
// Stack handling
VIA_IMPLEMENTATION void __stack_allocate(IState* state) {
  state->sbp = new IValue[VIA_VMSTACKSIZE];
}

VIA_IMPLEMENTATION void __stack_deallocate(const IState* state) {
  delete[] state->sbp;
}

VIA_IMPLEMENTATION void __push(IState* state, IValue&& val) {
  state->sbp[state->sp++] = std::move(val);
}

VIA_IMPLEMENTATION IValue __pop(IState* state) {
  return std::move(state->sbp[state->sp--]);
}

VIA_IMPLEMENTATION void __drop(IState* state) {
  IValue& dropped = state->sbp[state->sp--];
  dropped.reset();
}

VIA_IMPLEMENTATION IValue* __get_stack(const IState* state, size_t offset) {
  return &state->sbp[offset];
}

VIA_IMPLEMENTATION void __set_stack(const IState* state, size_t offset, IValue&& val) {
  state->sbp[offset] = std::move(val);
}

VIA_IMPLEMENTATION IValue* __get_local(const IState* VIA_RESTRICT state, size_t offset) {
  ICallInfo call_info = state->frame->call_data;
  size_t final_offset = call_info.sp + call_info.argc + offset;
  return &state->sbp[final_offset];
}

VIA_IMPLEMENTATION void __set_local(const IState* VIA_RESTRICT state, size_t offset, IValue&& val) {
  ICallInfo call_info = state->frame->call_data;
  size_t final_offset = call_info.sp + call_info.argc + offset;
  state->sbp[final_offset] = std::move(val);
}

VIA_IMPLEMENTATION IValue __get_argument(const IState* VIA_RESTRICT state, size_t offset) {
  if (offset >= state->frame->call_data.argc) {
    return IValue();
  }

  // Compute stack offset in reverse order
  const operand_t stk_offset = state->frame->call_data.sp - state->frame->call_data.argc + offset;
  const IValue& val = state->sbp[stk_offset];

  return val.clone();
}

// ==========================================================
// Register handling
VIA_IMPLEMENTATION void __register_allocate(IState* state) {
  state->spill_registers = new spill_registers_t();
}

VIA_IMPLEMENTATION void __register_deallocate(const IState* state) {
  delete state->spill_registers;
}

VIA_OPTIMIZE void __set_register(const IState* state, operand_t reg, IValue val) {
  if VIA_LIKELY (reg < VIA_STK_REGISTERS) {
    state->stack_registers.registers[reg] = std::move(val);
  }
  else {
    const operand_t offset = reg - VIA_STK_REGISTERS;
    state->spill_registers->registers[offset] = std::move(val);
  }
}

VIA_OPTIMIZE IValue* __get_register(const IState* state, operand_t reg) {
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
