// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _vl_vmapi_h
#define _vl_vmapi_h

#include "common.h"
#include "constant.h"
#include "instruction.h"
#include "opcode.h"
#include "state.h"
#include "string-utility.h"
#include "api-aux.h"

namespace via::impl {

static const value_obj _Nil = value_obj();

vl_optimize void __set_error_state(State* _State, const std::string& _Msg) {
  _State->err->frame = _State->frame;
  _State->err->message = std::string(_Msg);
}

vl_optimize void __clear_error_state(State* _State) {
  _State->err->frame = nullptr;
  _State->err->message = "";
}

vl_optimize bool __has_error(State* _State) {
  return _State->err->frame != nullptr;
}

vl_forceinline bool __handle_error(State* _State) {
  tfunction* _Current_frame = _State->frame;
  tfunction* _Error_frame = _State->frame;

  while (_Current_frame) {
    if (_Current_frame->is_error_handler) {
      _State->err->frame = _Current_frame;
      break;
    }
    _Current_frame = _Current_frame->call_info.caller;
  }

  if (!_Current_frame) {
    if (_Error_frame) {
      std::string _Error = std::format(
        "error at <frame@0x{:x}>: {}\n\n",
        reinterpret_cast<uintptr_t>(_Error_frame),
        _State->err->message
      );
      std::cerr << _Error;
    }

    std::unordered_set<tfunction*> visited;

    size_t _Idx = 0;
    while (_Error_frame && !visited.count(_Error_frame)) {
      visited.insert(_Error_frame);
      std::cerr << std::format(
        "#{} <frame@0x{:x}>\n", _Idx++, reinterpret_cast<uintptr_t>(_Error_frame)
      );
      _Error_frame = _Error_frame->call_info.caller;
    }
  }

  return static_cast<bool>(_Current_frame);
}

vl_inline value_obj __get_constant(State* _State, size_t _Idx) {
  if (_Idx >= _State->unit_ctx.constants->size()) {
    return _Nil.clone();
  }

  return _State->unit_ctx.constants->at(_Idx).clone();
}

vl_forceinline value_obj __type(State* vl_restrict _State, const value_obj& _Val) {
  std::string _Temp = std::string(magic_enum::enum_name(_Val.type));
  const char* _Str = _Temp.c_str();
  return value_obj(value_type::string, new string_obj(_State, _Str));
}

vl_forceinline std::string __type_cxx_string(State* vl_restrict _State, const value_obj& _Val) {
  value_obj _Type = __type(_State, _Val);
  return std::string(_Type.cast_ptr<string_obj>()->data);
}

vl_forceinline value_obj __typeofv(State* vl_restrict _State, const value_obj& _Val) {
  if (_Val.is_table()) {
    const table_obj* _Tbl = _Val.cast_ptr<table_obj>();
    const value_obj& _Type = __table_get(_Tbl, value_obj(new string_obj(_State, "__type")));

    if (_Type.is_nil()) {
      return __type(_State, _Val);
    }

    return value_obj(new string_obj(_State, _Type.cast_ptr<string_obj>()->data));
  }

  return __type(_State, _Val);
}

vl_optimize void __native_call(State* _State, tfunction* _Callee, size_t _Argc) {
  _Callee->call_info.caller = _State->frame;
  _Callee->call_info.ibp = _State->ibp;
  _Callee->call_info.iep = _State->iep;
  _Callee->call_info.pc = _State->pc;
  _Callee->call_info.sp = _State->sp;
  _Callee->call_info.argc = _Argc;

  _State->frame = _Callee;
  _State->pc = _Callee->ibp;
  _State->ibp = _Callee->ibp;
  _State->iep = _Callee->iep;
}

vl_optimize void __extern_call(State* _State, tcfunction* _Callee, size_t _Argc) {
  tfunction _Func;
  _Func.is_error_handler = _Callee->is_error_handler;
  _Func.call_info.caller = _State->frame;
  _Func.call_info.ibp = _State->ibp;
  _Func.call_info.iep = _State->iep;
  _Func.call_info.pc = _State->pc;

  __native_call(_State, &_Func, _Argc);
  _Callee->data(_State);
}

vl_optimize void __call(State* _State, value_obj& _Callee, size_t _Argc) {
  if (_Callee.is_function()) {
    __native_call(_State, _Callee.cast_ptr<tfunction>(), _Argc);
  }
  else if (_Callee.is_cfunction()) {
    __extern_call(_State, _Callee.cast_ptr<tcfunction>(), _Argc);
  }
  else {
    __set_error_state(
      _State, std::format("attempt to call a {} value", __type_cxx_string(_State, _Callee))
    );
  }
}

vl_forceinline value_obj __len(value_obj& _Val) {
  if (_Val.is_string()) {
    return value_obj(static_cast<TInteger>(_Val.cast_ptr<string_obj>()->len));
  }
  else if (_Val.is_table()) {
    size_t _Size = __table_size(_Val.cast_ptr<table_obj>());
    return value_obj(static_cast<TInteger>(_Size));
  }

  return _Nil.clone();
}

vl_forceinline void __native_return(State* vl_restrict _State, const value_obj& _Ret_value) {
  __closure_close_upvalues(_State->frame);

  call_info _Call_info = _State->frame->call_info;

  _State->ibp = _Call_info.ibp;
  _State->iep = _Call_info.iep;
  _State->pc = _Call_info.pc;
  _State->frame = _Call_info.caller;

  _State->sp = _Call_info.sp;
  _State->sp -= _Call_info.argc;

  __push(_State, _Ret_value);
}

vl_optimize value_obj __get_global(State* vl_restrict _State, uint32_t _Id) {
  std::lock_guard<std::mutex> lock(_State->G->gtable_mutex);

  auto _It = _State->G->gtable.find(_Id);
  if (_It != _State->G->gtable.end()) {
    return _It->second.clone();
  }

  return _Nil.clone();
}

vl_forceinline void __set_global(State* vl_restrict _State, uint32_t _Id, const value_obj& _Val) {
  std::lock_guard<std::mutex> lock(_State->G->gtable_mutex);

  auto _It = _State->G->gtable.find(_Id);
  if (_It != _State->G->gtable.end()) {
    __set_error_state(_State, std::format("attempt to reassign global '{}'", _Id));
  }

  _State->G->gtable.emplace(_Id, _Val.clone());
}

vl_inline value_obj __to_string(State* vl_restrict _State, const value_obj& _Val) {
  using enum value_type;

  if (_Val.is_string()) {
    return _Val.clone();
  }

  switch (_Val.type) {
  case integer: {
    std::string _Str = std::to_string(_Val.val_integer);
    string_obj* _Tstr = new string_obj(_State, _Str.c_str());
    return value_obj(string, _Tstr);
  }
  case floating_point: {
    std::string _Str = std::to_string(_Val.val_floating_point);
    string_obj* _Tstr = new string_obj(_State, _Str.c_str());
    return value_obj(string, _Tstr);
  }
  case boolean: {
    string_obj* _Str = new string_obj(_State, _Val.val_boolean ? "true" : "false");
    return value_obj(string, _Str);
  }
  case table:
  case function:
  case cfunction: {
    auto _Type_str = magic_enum::enum_name(_Val.type);
    auto _Final_str =
      std::format("<{}@0x{:x}>", _Type_str, reinterpret_cast<uintptr_t>(_Val.val_pointer));

    string_obj* _Str = new string_obj(_State, _Final_str.c_str());
    return value_obj(string, _Str);
  }
  default:
    string_obj* _Tstr = new string_obj(_State, "nil");
    return value_obj(string, _Tstr);
  }

  vl_unreachable;
  return _Nil.clone();
}

vl_forceinline std::string __to_cxx_string(State* vl_restrict _State, const value_obj& _Val) {
  value_obj _Str = __to_string(_State, _Val);
  return std::string(_Str.cast_ptr<string_obj>()->data);
}

vl_forceinline value_obj __to_bool(const value_obj& _Val) {
  if (_Val.is_bool()) {
    return _Val.clone();
  }

  return value_obj(_Val.type != value_type::nil);

  vl_unreachable;
  return _Nil.clone();
}

vl_forceinline bool __to_cxx_bool(const value_obj& _Val) {
  return __to_bool(_Val).val_boolean;
}

vl_forceinline value_obj __to_int(State* V, const value_obj& _Val) {
  using enum value_type;

  if (_Val.is_number()) {
    return _Val.clone();
  }

  switch (_Val.type) {
  case string: {
    const std::string& str = _Val.cast_ptr<string_obj>()->data;
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
    return value_obj(static_cast<TInteger>(_Val.val_boolean));
  default:
    break;
  }

  return _Nil.clone();
}

vl_forceinline value_obj __to_float(State* V, const value_obj& _Val) {
  using enum value_type;

  if (_Val.is_number()) {
    return _Val.clone();
  }

  switch (_Val.type) {
  case string: {
    const std::string& str = _Val.cast_ptr<string_obj>()->data;
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
    return value_obj(static_cast<TFloat>(_Val.val_boolean));
  default:
    break;
  }

  return _Nil.clone();
}

vl_forceinline void* __to_pointer(const value_obj& _Val) {
  switch (_Val.type) {
  case value_type::cfunction:
  case value_type::function:
  case value_type::table:
  case value_type::string:
    return _Val.val_pointer;
  default:
    return nullptr;
  }
}

vl_optimize bool __compare(const value_obj& _Val_0, const value_obj& _Val_1) {
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
