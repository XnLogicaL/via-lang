// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_VMAPI_H
#define _VIA_VMAPI_H

#include "common.h"
#include "constant.h"
#include "instruction.h"
#include "opcode.h"
#include "state.h"
#include "string-utility.h"
#include "api-aux.h"

VIA_NAMESPACE_IMPL_BEGIN

static const TValue _Nil = TValue();

VIA_INLINE_HOT void __set_error_state(State* _State, const std::string& _Msg) {
    _State->err->frame   = _State->frame;
    _State->err->message = std::string(_Msg);
}

VIA_INLINE_HOT void __clear_error_state(State* _State) {
    _State->err->frame   = nullptr;
    _State->err->message = "";
}

VIA_INLINE_HOT bool __has_error(State* _State) {
    return _State->err->frame != nullptr;
}

VIA_FORCE_INLINE bool __handle_error(State* _State) {
    TFunction* _Current_frame = _State->frame;
    TFunction* _Error_frame   = _State->frame;

    while (_Current_frame) {
        if (_Current_frame->is_error_handler) {
            _State->err->frame = _Current_frame;
            break;
        }
        _Current_frame = _Current_frame->caller;
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

        std::unordered_set<TFunction*> visited;

        size_t _Idx = 0;
        while (_Error_frame && !visited.count(_Error_frame)) {
            visited.insert(_Error_frame);
            std::cerr << std::format(
                "#{} <frame@0x{:x}>\n", _Idx++, reinterpret_cast<uintptr_t>(_Error_frame)
            );
            _Error_frame = _Error_frame->caller;
        }
    }

    return static_cast<bool>(_Current_frame);
}

VIA_INLINE_HOT void __set_register(State* _State, Operand _Reg, const TValue& _Val) {
    TValue* addr = _State->registers + _Reg;
    *addr        = _Val.clone();
}

VIA_INLINE_HOT TValue* __get_register(State* _State, Operand _Reg) {
    TValue* addr = _State->registers + _Reg;
    return addr;
}

VIA_INLINE TValue __get_constant(State* _State, size_t _Idx) {
    if (_Idx >= _State->program.constants->size()) {
        return _Nil.clone();
    }

    return _State->program.constants->at(_Idx).clone();
}

VIA_INLINE_HOT void __push(State* _State, const TValue& _Val) {
    _State->sbp[_State->sp++] = _Val.clone();
}

VIA_INLINE_HOT TValue __pop(State* _State) {
    return _State->sbp[_State->sp--].clone();
}

VIA_INLINE_HOT void __drop(State* _State) {
    TValue& _Dropped_val = _State->sbp[_State->sp--];
    _Dropped_val.reset();
}

VIA_INLINE_HOT const TValue& __get_stack(State* _State, size_t _Offset) {
    return _State->sbp[_Offset];
}

VIA_INLINE_HOT void __set_stack(State* _State, size_t _Offset, TValue& _Val) {
    _State->sbp[_Offset] = _Val.clone();
}

VIA_FORCE_INLINE TValue __get_argument(State* VIA_RESTRICT _State, size_t _Offset) {
    if (_Offset >= _State->argc) {
        return _Nil.clone();
    }

    const Operand _Stk_offset = _State->ssp + _State->argc - 1 - _Offset;
    const TValue& _Val        = _State->sbp[_Stk_offset];

    return _Val.clone();
}

VIA_FORCE_INLINE TValue __type(State* VIA_RESTRICT _State, const TValue& _Val) {
    std::string _Temp = std::string(magic_enum::enum_name(_Val.type));
    const char* _Str  = _Temp.c_str();
    return TValue(ValueType::string, new TString(_State, _Str));
}

VIA_FORCE_INLINE std::string __type_cxx_string(State* VIA_RESTRICT _State, const TValue& _Val) {
    TValue _Type = __type(_State, _Val);
    return std::string(_Type.cast_ptr<TString>()->data);
}

VIA_FORCE_INLINE TValue __typeofv(State* VIA_RESTRICT _State, const TValue& _Val) {
    if (_Val.is_table()) {
        const TTable* _Tbl  = _Val.cast_ptr<TTable>();
        const TValue& _Type = __table_get(_Tbl, TValue(new TString(_State, "__type")));

        if (_Type.is_nil()) {
            return __type(_State, _Val);
        }

        return TValue(new TString(_State, _Type.cast_ptr<TString>()->data));
    }

    return __type(_State, _Val);
}

VIA_INLINE_HOT void __native_call(State* _State, TFunction* _Callee, size_t _Argc) {
    _Callee->caller   = _State->frame;
    _Callee->ret_addr = _State->ip;
    _State->frame     = _Callee;
    _State->sibp      = _State->ibp;
    _State->siep      = _State->iep;
    _State->ip        = _Callee->bytecode;
    _State->ibp       = _Callee->bytecode;
    _State->iep       = _Callee->bytecode + _Callee->bytecode_len;
    _State->argc      = _Argc;
    _State->ssp       = _State->sp;
}

VIA_INLINE_HOT void __extern_call(State* _State, TCFunction* _Callee, size_t _Argc) {
    char        _Buf[2 + std::numeric_limits<uintptr_t>::digits / 4 + 1];
    const void* _Addr    = _Callee;
    uintptr_t   _Address = reinterpret_cast<uintptr_t>(_Addr);
    _Buf[0]              = '0';
    _Buf[1]              = 'x';

    auto _Result = std::to_chars(_Buf + 2, _Buf + sizeof(_Buf), _Address, 16);
    if (_Result.ec != std::errc{}) {
        std::memset(_Buf + 2, '0', sizeof(_Buf) - 2);
    }

    TFunction _Func(_Callee->is_error_handler, false, _State->ip, _State->frame);

    __native_call(_State, &_Func, _Argc);
    _Callee->data(_State);
}

VIA_INLINE_HOT void __call(State* _State, TValue& _Callee, size_t _Argc) {
    _State->calltype = CallType::CALL;

    if (_Callee.is_function()) {
        __native_call(_State, _Callee.cast_ptr<TFunction>(), _Argc);
    }
    else if (_Callee.is_cfunction()) {
        __extern_call(_State, _Callee.cast_ptr<TCFunction>(), _Argc);
    }
    else {
        __set_error_state(
            _State, std::format("attempt to call a {} value", __type_cxx_string(_State, _Callee))
        );
    }
}

VIA_FORCE_INLINE TValue __len(TValue& _Val) {
    if (_Val.is_string()) {
        return TValue(static_cast<TInteger>(_Val.cast_ptr<TString>()->len));
    }
    else if (_Val.is_table()) {
        size_t _Size = __table_size(_Val.cast_ptr<TTable>());
        return TValue(static_cast<TInteger>(_Size));
    }

    return _Nil.clone();
}

VIA_FORCE_INLINE void __native_return(State* VIA_RESTRICT _State, const TValue& _Ret_value) {
    __closure_close_upvalues(_State->frame);

    _State->ibp   = _State->sibp;
    _State->iep   = _State->siep;
    _State->ip    = _State->frame->ret_addr;
    _State->frame = _State->frame->caller;

    _State->sp = _State->ssp;
    _State->sp -= _State->argc;

    __push(_State, _Ret_value);
}

VIA_INLINE_HOT TValue __get_global(State* VIA_RESTRICT _State, Operand _Id) {
    std::lock_guard<std::mutex> lock(_State->G->gtable_mutex);

    auto _It = _State->G->gtable.find(_Id);
    if (_It != _State->G->gtable.end()) {
        return _It->second.clone();
    }

    return _Nil.clone();
}

VIA_FORCE_INLINE void __set_global(State* VIA_RESTRICT _State, Operand _Id, const TValue& _Val) {
    std::lock_guard<std::mutex> lock(_State->G->gtable_mutex);

    auto _It = _State->G->gtable.find(_Id);
    if (_It != _State->G->gtable.end()) {
        __set_error_state(_State, std::format("attempt to reassign global '{}'", _Id));
    }

    _State->G->gtable.emplace(_Id, _Val.clone());
}

VIA_INLINE TValue __to_string(State* VIA_RESTRICT _State, const TValue& _Val) {
    using enum ValueType;

    if (_Val.is_string()) {
        return _Val.clone();
    }

    switch (_Val.type) {
    case integer: {
        std::string _Str  = std::to_string(_Val.val_integer);
        TString*    _Tstr = new TString(_State, _Str.c_str());
        return TValue(string, _Tstr);
    }
    case floating_point: {
        std::string _Str  = std::to_string(_Val.val_floating_point);
        TString*    _Tstr = new TString(_State, _Str.c_str());
        return TValue(string, _Tstr);
    }
    case boolean: {
        TString* _Str = new TString(_State, _Val.val_boolean ? "true" : "false");
        return TValue(string, _Str);
    }
    case table:
    case function:
    case cfunction: {
        auto _Type_str = magic_enum::enum_name(_Val.type);
        auto _Final_str =
            std::format("<{}@0x{:x}>", _Type_str, reinterpret_cast<uintptr_t>(_Val.val_pointer));

        TString* _Str = new TString(_State, _Final_str.c_str());
        return TValue(string, _Str);
    }
    default:
        TString* _Tstr = new TString(_State, "nil");
        return TValue(string, _Tstr);
    }

    VIA_UNREACHABLE;
    return _Nil.clone();
}

VIA_FORCE_INLINE std::string __to_cxx_string(State* VIA_RESTRICT _State, const TValue& _Val) {
    TValue _Str = __to_string(_State, _Val);
    return std::string(_Str.cast_ptr<TString>()->data);
}

VIA_FORCE_INLINE TValue __to_bool(const TValue& _Val) {
    if (_Val.is_bool()) {
        return _Val.clone();
    }

    return TValue(_Val.type != ValueType::nil);

    VIA_UNREACHABLE;
    return _Nil.clone();
}

VIA_FORCE_INLINE bool __to_cxx_bool(const TValue& _Val) {
    return __to_bool(_Val).val_boolean;
}

VIA_FORCE_INLINE TValue __to_number(const TValue& _Val) {
    using enum ValueType;

    if (_Val.is_number()) {
        return _Val.clone();
    }

    switch (_Val.type) {
    case string: {
        const std::string& str = _Val.cast_ptr<TString>()->data;
        if (str.empty()) {
            return _Nil.clone();
        }

        int int_result;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), int_result);
        if (ec == std::errc() && ptr == str.data() + str.size()) {
            return TValue(static_cast<float>(int_result)); // Convert to float for consistency
        }

        float float_result;
        auto [ptr_f, ec_f] = std::from_chars(str.data(), str.data() + str.size(), float_result);
        if (ec_f == std::errc() && ptr_f == str.data() + str.size()) {
            return TValue(float_result);
        }

        return _Nil.clone();
    }
    case boolean:
        return TValue(static_cast<int>(_Val.val_boolean));
    default:
        break;
    }

    return _Nil.clone();
}

template<typename T>
    requires std::is_arithmetic_v<T>
VIA_FORCE_INLINE T __to_cxx_number(const TValue& _Val) {
    TValue _Number = __to_number(_Val);

    if (check_nil(_Number)) {
        if constexpr (std::is_floating_point_v<T>) {
            return std::numeric_limits<T>::quiet_NaN();
        }
        else {
            return 0;
        }
    }

    if (check_integer(_Number)) {
        return static_cast<T>(_Number.val_integer);
    }
    if (check_floating_point(_Number)) {
        return static_cast<T>(_Number.val_floating_point);
    }

    if constexpr (std::is_floating_point_v<T>) {
        return std::numeric_limits<T>::quiet_NaN();
    }
    else {
        return 0;
    }
}

VIA_FORCE_INLINE void* __to_pointer(const TValue& _Val) {
    switch (_Val.type) {
    case ValueType::cfunction:
    case ValueType::function:
    case ValueType::table:
    case ValueType::string:
        return _Val.val_pointer;
    default:
        return nullptr;
    }
}

VIA_INLINE_HOT bool __compare(const TValue& _Val_0, const TValue& _Val_1) {
    using enum ValueType;

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
        return !std::strcmp(_Val_0.cast_ptr<TString>()->data, _Val_1.cast_ptr<TString>()->data);
    default:
        return __to_pointer(_Val_0) == __to_pointer(_Val_1);
    }

    VIA_UNREACHABLE;
    return false;
};

VIA_NAMESPACE_END

#endif
