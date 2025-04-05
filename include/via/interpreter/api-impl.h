// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef VIA_HAS_HEADER_VMAPI_H
#define VIA_HAS_HEADER_VMAPI_H

#include "common.h"
#include "constant.h"
#include "instruction.h"
#include "opcode.h"
#include "state.h"
#include "string-utility.h"
#include "api-aux.h"

namespace via::impl {

static const value_obj _Nil = value_obj();

VIA_IMPLEMENTATION void __set_error_state(state* state, const std::string& message) {
  state->err->frame = state->frame;
  state->err->message = std::string(message);
}

VIA_IMPLEMENTATION void __clear_error_state(state* state) {
  state->err->frame = nullptr;
  state->err->message = "";
}

VIA_IMPLEMENTATION bool __has_error(state* state) {
  return state->err->frame != nullptr;
}

VIA_IMPLEMENTATION bool __handle_error(state* state) {
  function_obj* current_frame = state->frame;
  function_obj* error_frame = state->frame;

  while (current_frame) {
    if (/* TODO */ false) {
      state->err->frame = current_frame;
      break;
    }
    current_frame = current_frame->call_data.caller;
  }

  if (!current_frame) {
    if (error_frame) {
      std::string _Error = std::format(
        "error at <frame@0x{:x}>: {}\n\n",
        reinterpret_cast<uintptr_t>(error_frame),
        state->err->message
      );
      std::cerr << _Error;
    }

    std::unordered_set<function_obj*> visited;

    size_t index = 0;
    while (error_frame && !visited.count(error_frame)) {
      visited.insert(error_frame);
      std::cerr << std::format(
        "#{} <frame@0x{:x}>\n", index++, reinterpret_cast<uintptr_t>(error_frame)
      );
      error_frame = error_frame->call_data.caller;
    }
  }

  return static_cast<bool>(current_frame);
}

VIA_IMPLEMENTATION value_obj __get_constant(state* state, size_t index) {
  if (index >= state->unit_ctx.constants->size()) {
    return _Nil.clone();
  }

  return state->unit_ctx.constants->at(index).clone();
}

VIA_IMPLEMENTATION value_obj __type(const value_obj& val) {
  std::string _Temp = std::string(magic_enum::enum_name(val.type));
  const char* str = _Temp.c_str();
  return value_obj(value_type::string, new string_obj(str));
}

VIA_IMPLEMENTATION std::string __type_cxx_string(const value_obj& val) {
  value_obj _Type = __type(val);
  return std::string(_Type.cast_ptr<string_obj>()->data);
}

VIA_IMPLEMENTATION void __native_call(state* state, function_obj* _Callee, size_t _Argc) {
  _Callee->call_data.caller = state->frame;
  _Callee->call_data.ibp = state->ibp;
  _Callee->call_data.pc = state->pc;
  _Callee->call_data.sp = state->sp;
  _Callee->call_data.argc = _Argc;

  state->frame = _Callee;
  state->pc = _Callee->ibp;
  state->ibp = _Callee->ibp;
}

VIA_IMPLEMENTATION void __extern_call(state* state, const value_obj& _Callee, size_t _Argc) {
  function_obj func;
  func.call_data.caller = state->frame;
  func.call_data.ibp = state->ibp;
  func.call_data.pc = state->pc;

  // Hack to bypass destructor
  func.ibp = nullptr;
  func.upvs = nullptr;

  __native_call(state, &func, _Argc);
  _Callee.cast_ptr<cfunction_t>()(state);
}

VIA_IMPLEMENTATION void __call(state* state, value_obj& _Callee, size_t _Argc) {
  if (_Callee.is_function()) {
    __native_call(state, _Callee.cast_ptr<function_obj>(), _Argc);
  }
  else if (_Callee.is_cfunction()) {
    __extern_call(state, _Callee, _Argc);
  }
  else {
    __set_error_state(state, std::format("attempt to call a {} value", __type_cxx_string(_Callee)));
  }
}

VIA_IMPLEMENTATION value_obj __len(value_obj& val) {
  if (val.is_string()) {
    return value_obj(static_cast<TInteger>(val.cast_ptr<string_obj>()->len));
  }
  else if (val.is_table()) {
    size_t _Size = __table_size(val.cast_ptr<table_obj>());
    return value_obj(static_cast<TInteger>(_Size));
  }

  return _Nil.clone();
}

VIA_IMPLEMENTATION void __native_return(state* VIA_RESTRICT state, const value_obj& _Ret_value) {
  __closure_close_upvalues(state->frame);

  call_info _Call_info = state->frame->call_data;

  state->ibp = _Call_info.ibp;
  state->pc = _Call_info.pc + 1; // Required to prevent infinite loop
  state->frame = _Call_info.caller;

  state->sp = _Call_info.sp;
  state->sp -= _Call_info.argc;

  __push(state, _Ret_value);
}

VIA_IMPLEMENTATION value_obj __to_string(const value_obj& val) {
  using enum value_type;

  if (val.is_string()) {
    return val.clone();
  }

  switch (val.type) {
  case integer: {
    std::string str = std::to_string(val.val_integer);
    return value_obj(str.c_str());
  }
  case floating_point: {
    std::string str = std::to_string(val.val_floating_point);
    return value_obj(str.c_str());
  }
  case boolean:
    return value_obj(val.val_boolean ? "true" : "false");
  case table:
  case function:
  case cfunction: {
    auto type_str = magic_enum::enum_name(val.type);
    auto final_str =
      std::format("<{}@0x{:x}>", type_str, reinterpret_cast<uintptr_t>(val.val_pointer));

    return value_obj(final_str.c_str());
  }
  default:
    return value_obj("nil");
  }

  VIA_UNREACHABLE;
  return _Nil.clone();
}

VIA_IMPLEMENTATION std::string __to_cxx_string(const value_obj& val) {
  value_obj str = __to_string(val);
  return std::string(str.cast_ptr<string_obj>()->data);
}

VIA_IMPLEMENTATION std::string __to_literal_cxx_string(const value_obj& val) {
  value_obj str = __to_string(val);
  std::string str_cpy = str.cast_ptr<string_obj>()->data;
  return escape_string(str_cpy);
}

VIA_IMPLEMENTATION value_obj __to_bool(const value_obj& val) {
  if (val.is_bool()) {
    return val.clone();
  }

  return value_obj(val.type != value_type::nil);

  VIA_UNREACHABLE;
  return _Nil.clone();
}

VIA_IMPLEMENTATION bool __to_cxx_bool(const value_obj& val) {
  return __to_bool(val).val_boolean;
}

VIA_IMPLEMENTATION value_obj __to_int(state* V, const value_obj& val) {
  using enum value_type;

  if (val.is_number()) {
    return val.clone();
  }

  switch (val.type) {
  case string: {
    const std::string& str = val.cast_ptr<string_obj>()->data;
    if (str.empty()) {
      return _Nil.clone();
    }

    int int_result;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), int_result);
    if (ec == std::errc() && ptr == str.data() + str.size()) {
      return value_obj(static_cast<float>(int_result)); // Convert to float for consistency
    }

    __set_error_state(V, "string -> integer cast failed");
    return _Nil.clone();
  }
  case boolean:
    return value_obj(static_cast<TInteger>(val.val_boolean));
  default:
    break;
  }

  return _Nil.clone();
}

VIA_IMPLEMENTATION value_obj __to_float(state* V, const value_obj& val) {
  using enum value_type;

  if (val.is_number()) {
    return val.clone();
  }

  switch (val.type) {
  case string: {
    const std::string& str = val.cast_ptr<string_obj>()->data;
    if (str.empty()) {
      return _Nil.clone();
    }

    float float_result;
    auto [ptr_f, ec_f] = std::from_chars(str.data(), str.data() + str.size(), float_result);
    if (ec_f == std::errc() && ptr_f == str.data() + str.size()) {
      return value_obj(float_result);
    }

    __set_error_state(V, "string -> float cast failed");
    return _Nil.clone();
  }
  case boolean:
    return value_obj(static_cast<TFloat>(val.val_boolean));
  default:
    break;
  }

  return _Nil.clone();
}

VIA_IMPLEMENTATION void* __to_pointer(const value_obj& val) {
  switch (val.type) {
  case value_type::cfunction:
  case value_type::function:
  case value_type::table:
  case value_type::string:
    return val.val_pointer;
  default:
    return nullptr;
  }
}

VIA_IMPLEMENTATION bool __compare(const value_obj& _Val_0, const value_obj& _Val_1) {
  using enum value_type;

  if (_Val_0.type != _Val_1.type) {
    return false;
  }

  switch (_Val_0.type) {
  case integer:
    return _Val_0.val_integer == _Val_1.val_integer;
  case floating_point:
    return _Val_0.val_floating_point == _Val_1.val_floating_point;
  case boolean:
    return _Val_0.val_boolean == _Val_1.val_boolean;
  case nil:
    return true;
  case string:
    return !std::strcmp(_Val_0.cast_ptr<string_obj>()->data, _Val_1.cast_ptr<string_obj>()->data);
  default:
    return __to_pointer(_Val_0) == __to_pointer(_Val_1);
  }

  VIA_UNREACHABLE;
  return false;
};

} // namespace via::impl

#endif
