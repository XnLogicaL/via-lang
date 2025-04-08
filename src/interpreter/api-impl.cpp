// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "bit-utility.h"
#include "file-io.h"
#include "api-impl.h"
#include "api-aux.h"
#include "common.h"
#include "state.h"
#include "constant.h"
#include <cmath>

namespace via::impl {

void __set_error_state(state* state, const std::string& message) {
  state->err->frame = state->frame;
  state->err->message = std::move(message);
}

void __clear_error_state(state* state) {
  state->err->frame = nullptr;
  state->err->message = "";
}

bool __has_error(state* state) {
  return state->err->frame != nullptr;
}

bool __handle_error(state* state) {
  IFunction* current_frame = state->frame;
  IFunction* error_frame = state->frame;

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

    std::unordered_set<IFunction*> visited;

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

IValue __get_constant(state* state, size_t index) {
  if (index >= state->unit_ctx.constants->size()) {
    return IValue();
  }

  return state->unit_ctx.constants->at(index).clone();
}

IValue __type(const IValue& val) {
  std::string _Temp = std::string(magic_enum::enum_name(val.type));
  const char* str = _Temp.c_str();
  return IValue(new string_obj(str));
}

std::string __type_cxx_string(const IValue& val) {
  IValue _Type = __type(val);
  return std::string(_Type.val_string->data);
}

void* __to_pointer(const IValue& val) {
  switch (val.type) {
  case IValueType::cfunction:
  case IValueType::function:
  case IValueType::array:
  case IValueType::dict:
  case IValueType::string:
    // This is technically UB... too bad!
    return reinterpret_cast<void*>(val.val_string);
  default:
    return nullptr;
  }
}

void __native_call(state* state, IFunction* _Callee, size_t _Argc) {
  _Callee->call_data.caller = state->frame;
  _Callee->call_data.ibp = state->ibp;
  _Callee->call_data.pc = state->pc;
  _Callee->call_data.sp = state->sp;
  _Callee->call_data.argc = _Argc;

  state->frame = _Callee;
  state->pc = _Callee->ibp;
  state->ibp = _Callee->ibp;
}

void __extern_call(state* state, const IValue& _Callee, size_t _Argc) {
  IFunction func;
  func.call_data.caller = state->frame;
  func.call_data.ibp = state->ibp;
  func.call_data.pc = state->pc;

  // Hack to bypass destructor
  func.ibp = nullptr;
  func.upvs = nullptr;

  __native_call(state, &func, _Argc);
  _Callee.val_cfunction(state);
}

void __call(state* state, IValue& _Callee, size_t _Argc) {
  if (_Callee.is_function()) {
    __native_call(state, _Callee.val_function, _Argc);
  }
  else if (_Callee.is_cfunction()) {
    __extern_call(state, _Callee, _Argc);
  }
  else {
    __set_error_state(state, std::format("attempt to call a {} value", __type_cxx_string(_Callee)));
  }
}

IValue __length(IValue& val) {
  if (val.is_string()) {
    return IValue(static_cast<int>(val.val_string->len));
  }
  else if (val.is_array() || val.is_dict()) {
    size_t len = val.is_array() ? __array_size(val.val_array) : __dict_size(val.val_dict);
    return IValue(static_cast<int>(len));
  }

  return IValue();
}

int __length_cxx(IValue& val) {
  IValue len = __length(val);
  return len.is_nil() ? -1 : len.val_integer;
}

void __native_return(state* VIA_RESTRICT state, IValue _Ret_value) {
  __closure_close_upvalues(state->frame);

  ICallInfo _Call_info = state->frame->call_data;

  state->ibp = _Call_info.ibp;
  state->pc = _Call_info.pc + 1; // Required to prevent infinite loop
  state->frame = _Call_info.caller;

  state->sp = _Call_info.sp;
  state->sp -= _Call_info.argc;

  __push(state, std::move(_Ret_value));
}

IValue __to_string(const IValue& val) {
  using enum IValueType;

  if (val.is_string()) {
    return val.clone();
  }

  switch (val.type) {
  case integer: {
    std::string str = std::to_string(val.val_integer);
    return IValue(str.c_str());
  }
  case floating_point: {
    std::string str = std::to_string(val.val_floating_point);
    return IValue(str.c_str());
  }
  case boolean:
    return IValue(val.val_boolean ? "true" : "false");
  case array:
  case dict:
  case function:
  case cfunction: {
    auto type_str = magic_enum::enum_name(val.type);
    auto final_str = std::format("<{}@0x{:x}>", type_str, (uintptr_t)__to_pointer(val));

    return IValue(final_str.c_str());
  }
  default:
    return IValue("nil");
  }

  VIA_UNREACHABLE();
  return IValue();
}

std::string __to_cxx_string(const IValue& val) {
  IValue str = __to_string(val);
  return std::string(str.val_string->data);
}

std::string __to_literal_cxx_string(const IValue& val) {
  IValue str = __to_string(val);
  std::string str_cpy = str.val_string->data;
  return escape_string(str_cpy);
}

IValue __to_bool(const IValue& val) {
  if (val.is_bool()) {
    return val.clone();
  }

  return IValue(val.type != IValueType::nil);

  VIA_UNREACHABLE();
  return IValue();
}

bool __to_cxx_bool(const IValue& val) {
  return __to_bool(val).val_boolean;
}

IValue __to_int(state* V, const IValue& val) {
  using enum IValueType;

  if (val.is_number()) {
    return val.clone();
  }

  switch (val.type) {
  case string: {
    const std::string& str = val.val_string->data;
    if (str.empty()) {
      return IValue();
    }

    int int_result;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), int_result);
    if (ec == std::errc() && ptr == str.data() + str.size()) {
      return IValue(static_cast<float>(int_result)); // Convert to float for consistency
    }

    __set_error_state(V, "string -> integer cast failed");
    return IValue();
  }
  case boolean:
    return IValue(static_cast<int>(val.val_boolean));
  default:
    break;
  }

  return IValue();
}

IValue __to_float(state* V, const IValue& val) {
  using enum IValueType;

  if (val.is_number()) {
    return val.clone();
  }

  switch (val.type) {
  case string: {
    const std::string& str = val.val_string->data;
    if (str.empty()) {
      return IValue();
    }

    float float_result;
    auto [ptr_f, ec_f] = std::from_chars(str.data(), str.data() + str.size(), float_result);
    if (ec_f == std::errc() && ptr_f == str.data() + str.size()) {
      return IValue(float_result);
    }

    __set_error_state(V, "string -> float cast failed");
    return IValue();
  }
  case boolean:
    return IValue(static_cast<float>(val.val_boolean));
  default:
    break;
  }

  return IValue();
}

bool __compare(const IValue& val0, const IValue& val1) {
  using enum IValueType;

  if (val0.type != val1.type) {
    return false;
  }

  switch (val0.type) {
  case integer:
    return val0.val_integer == val1.val_integer;
  case floating_point:
    return val0.val_floating_point == val1.val_floating_point;
  case boolean:
    return val0.val_boolean == val1.val_boolean;
  case nil:
    return true;
  case string:
    return !std::strcmp(val0.val_string->data, val1.val_string->data);
  default:
    return __to_pointer(val0) == __to_pointer(val1);
  }

  VIA_UNREACHABLE();
  return false;
};

} // namespace via::impl
