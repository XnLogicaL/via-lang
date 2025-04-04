// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_vmapi_h
#define vl_has_header_vmapi_h

#include "common.h"
#include "constant.h"
#include "instruction.h"
#include "opcode.h"
#include "state.h"
#include "string-utility.h"
#include "api-aux.h"

namespace via::impl {

static const value_obj _Nil = value_obj();

vl_implement void __set_error_state(state* state, const std::string& message) {
  state->err->frame = state->frame;
  state->err->message = std::string(message);
}

vl_implement void __clear_error_state(state* state) {
  state->err->frame = nullptr;
  state->err->message = "";
}

vl_implement bool __has_error(state* state) {
  return state->err->frame != nullptr;
}

vl_implement bool __handle_error(state* state) {
  function_obj* current_frame = state->frame;
  function_obj* error_frame = state->frame;

  while (current_frame) {
    if (/* TODO */ false) {
      state->err->frame = current_frame;
      break;
    }
    current_frame = current_frame->call_info.caller;
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
      error_frame = error_frame->call_info.caller;
    }
  }

  return static_cast<bool>(current_frame);
}

vl_implement value_obj __get_constant(state* state, size_t index) {
  if (index >= state->unit_ctx.constants->size()) {
    return _Nil.clone();
  }

  return state->unit_ctx.constants->at(index).clone();
}

vl_implement value_obj __type(state* vl_restrict state, const value_obj& val) {
  std::string _Temp = std::string(magic_enum::enum_name(val.type));
  const char* _Str = _Temp.c_str();
  return value_obj(value_type::string, new string_obj(state, _Str));
}

vl_implement std::string __type_cxx_string(state* vl_restrict state, const value_obj& val) {
  value_obj _Type = __type(state, val);
  return std::string(_Type.cast_ptr<string_obj>()->data);
}

vl_implement value_obj __typeofv(state* vl_restrict state, const value_obj& val) {
  if (val.is_table()) {
    const table_obj* table = val.cast_ptr<table_obj>();
    const value_obj& _Type = __table_get(table, value_obj(new string_obj(state, "__type")));

    if (_Type.is_nil()) {
      return __type(state, val);
    }

    return value_obj(new string_obj(state, _Type.cast_ptr<string_obj>()->data));
  }

  return __type(state, val);
}

vl_implement void __native_call(state* state, function_obj* _Callee, size_t _Argc) {
  _Callee->call_info.caller = state->frame;
  _Callee->call_info.ibp = state->ibp;
  _Callee->call_info.iep = state->iep;
  _Callee->call_info.pc = state->pc;
  _Callee->call_info.sp = state->sp;
  _Callee->call_info.argc = _Argc;

  state->frame = _Callee;
  state->pc = _Callee->ibp;
  state->ibp = _Callee->ibp;
  state->iep = _Callee->iep;
}

vl_implement void __extern_call(state* state, const value_obj& _Callee, size_t _Argc) {
  function_obj _Func;
  _Func.call_info.caller = state->frame;
  _Func.call_info.ibp = state->ibp;
  _Func.call_info.iep = state->iep;
  _Func.call_info.pc = state->pc;

  __native_call(state, &_Func, _Argc);
  _Callee.cast_ptr<cfunction_t>()(state);
}

vl_implement void __call(state* state, value_obj& _Callee, size_t _Argc) {
  if (_Callee.is_function()) {
    __native_call(state, _Callee.cast_ptr<function_obj>(), _Argc);
  }
  else if (_Callee.is_cfunction()) {
    __extern_call(state, _Callee, _Argc);
  }
  else {
    __set_error_state(
      state, std::format("attempt to call a {} value", __type_cxx_string(state, _Callee))
    );
  }
}

vl_implement value_obj __len(value_obj& val) {
  if (val.is_string()) {
    return value_obj(static_cast<TInteger>(val.cast_ptr<string_obj>()->len));
  }
  else if (val.is_table()) {
    size_t _Size = __table_size(val.cast_ptr<table_obj>());
    return value_obj(static_cast<TInteger>(_Size));
  }

  return _Nil.clone();
}

vl_implement void __native_return(state* vl_restrict state, const value_obj& _Ret_value) {
  __closure_close_upvalues(state->frame);

  call_info _Call_info = state->frame->call_info;

  state->ibp = _Call_info.ibp;
  state->iep = _Call_info.iep;
  state->pc = _Call_info.pc;
  state->frame = _Call_info.caller;

  state->sp = _Call_info.sp;
  state->sp -= _Call_info.argc;

  __push(state, _Ret_value);
}

vl_implement value_obj __to_string(state* vl_restrict state, const value_obj& val) {
  using enum value_type;

  if (val.is_string()) {
    return val.clone();
  }

  switch (val.type) {
  case integer: {
    std::string _Str = std::to_string(val.val_integer);
    string_obj* _Tstr = new string_obj(state, _Str.c_str());
    return value_obj(string, _Tstr);
  }
  case floating_point: {
    std::string _Str = std::to_string(val.val_floating_point);
    string_obj* _Tstr = new string_obj(state, _Str.c_str());
    return value_obj(string, _Tstr);
  }
  case boolean: {
    string_obj* _Str = new string_obj(state, val.val_boolean ? "true" : "false");
    return value_obj(string, _Str);
  }
  case table:
  case function:
  case cfunction: {
    auto _Type_str = magic_enum::enum_name(val.type);
    auto _Final_str =
      std::format("<{}@0x{:x}>", _Type_str, reinterpret_cast<uintptr_t>(val.val_pointer));

    string_obj* _Str = new string_obj(state, _Final_str.c_str());
    return value_obj(string, _Str);
  }
  default:
    string_obj* _Tstr = new string_obj(state, "nil");
    return value_obj(string, _Tstr);
  }

  vl_unreachable;
  return _Nil.clone();
}

vl_implement std::string __to_cxx_string(state* vl_restrict state, const value_obj& val) {
  value_obj _Str = __to_string(state, val);
  return std::string(_Str.cast_ptr<string_obj>()->data);
}

vl_implement value_obj __to_bool(const value_obj& val) {
  if (val.is_bool()) {
    return val.clone();
  }

  return value_obj(val.type != value_type::nil);

  vl_unreachable;
  return _Nil.clone();
}

vl_implement bool __to_cxx_bool(const value_obj& val) {
  return __to_bool(val).val_boolean;
}

vl_implement value_obj __to_int(state* V, const value_obj& val) {
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

vl_implement value_obj __to_float(state* V, const value_obj& val) {
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

vl_implement void* __to_pointer(const value_obj& val) {
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

vl_implement bool __compare(const value_obj& _Val_0, const value_obj& _Val_1) {
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

  vl_unreachable;
  return false;
};

} // namespace via::impl

#endif
