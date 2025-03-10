// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_VMAPI_H
#define _VIA_VMAPI_H

#include "common.h"
#include "gc.h"
#include "constant.h"
#include "instruction.h"
#include "opcode.h"
#include "state.h"
#include "strutils.h"
#include "rttypes.h"
#include "vaux.h"

VIA_NAMESPACE_IMPL_BEGIN

static const TValue _Nil = TValue();

VIA_INLINE_HOT void __set_error_state(State* _V, const std::string& _Msg) {
    _V->err->frame   = _V->frame;
    _V->err->message = _Msg;
}

VIA_INLINE_HOT void __clear_error_state(State* _V) {
    _V->err->frame   = nullptr;
    _V->err->message = "";
}

VIA_INLINE_HOT bool __has_error(State* _V) {
    return _V->err->frame != nullptr;
}

VIA_FORCE_INLINE bool __handle_error(State* _V) {
    TFunction* _Current_frame = _V->frame;
    TFunction* _Error_frame   = _V->frame;

    while (_Current_frame) {
        if (_Current_frame->is_error_handler) {
            _V->err->frame = _Current_frame;
            break;
        }
        _Current_frame = _Current_frame->caller;
    }

    if (!_Current_frame) {
        if (_Error_frame) {
            std::string _Error = std::format(
                "{} <frame@0x{:x}>: {}\n\n",
                _Error_frame->id,
                reinterpret_cast<uintptr_t>(_Error_frame),
                _V->err->message
            );
            std::cerr << _Error;
        }

        std::unordered_set<TFunction*> visited;

        SIZE _Idx = 0;
        while (_Error_frame && !visited.count(_Error_frame)) {
            visited.insert(_Error_frame);
            std::cerr << std::format(
                "#{} {} <frame@0x{:x}>\n",
                _Error_frame->id,
                _Idx++,
                reinterpret_cast<uintptr_t>(_Error_frame)
            );
            _Error_frame = _Error_frame->caller;
        }
    }

    return static_cast<bool>(_Current_frame);
}

VIA_INLINE_HOT void __set_register(State* _V, Operand _Reg, const TValue& _Val) {
    TValue* addr = _V->registers + _Reg;
    *addr        = _Val.clone();
}

VIA_INLINE_HOT TValue* __get_register(State* _V, Operand _Reg) {
    TValue* addr = _V->registers + _Reg;
    return addr;
}

VIA_INLINE TValue __get_constant(State* _V, SIZE _Idx) {
    if (_Idx >= _V->program.constants->size()) {
        return _Nil.clone();
    }

    return _V->program.constants->at(_Idx).clone();
}

VIA_INLINE_HOT void __push(State* _V, const TValue& _Val) {
    _V->sbp[_V->sp++] = _Val.clone();
}

VIA_INLINE_HOT TValue __pop(State* _V) {
    return _V->sbp[_V->sp--].clone();
}

VIA_INLINE_HOT TValue __get_stack(State* _V, Operand _Offset) noexcept {
    return _V->sbp[_Offset].clone();
}

VIA_INLINE_HOT void __set_stack(State* _V, Operand _Offset, TValue& _Val) noexcept {
    _V->sbp[_Offset] = _Val.clone();
}

VIA_FORCE_INLINE TValue __get_argument(State* VIA_RESTRICT _V, Operand _Offset) noexcept {
    if (_Offset >= _V->argc) {
        return _Nil.clone();
    }

    const Operand _Stk_offset = _V->ssp + _V->argc - 1 - _Offset;
    const TValue& _Val        = _V->sbp[_Stk_offset];

    return _Val.clone();
}

VIA_FORCE_INLINE TValue __type(State* VIA_RESTRICT _V, const TValue& _Val) noexcept {
    char* _Str = duplicate_string(std::string(magic_enum::enum_name(_Val.type)));
    return TValue(new TString(_V, _Str));
}

VIA_FORCE_INLINE std::string __type_cxx_string(State* VIA_RESTRICT _V, const TValue& _Val) {
    TValue _Type = __type(_V, _Val);
    return std::string(_Type.cast_ptr<TString>()->data);
}

VIA_FORCE_INLINE TValue __typeofv(State* VIA_RESTRICT _V, const TValue& _Val) {
    if (check_table(_Val)) {
        TTable*       _Tbl  = _Val.cast_ptr<TTable>();
        const TValue& _Type = __table_get(_Tbl, TValue(new TString(_V, "__type")));

        if (check_nil(_Type)) {
            return __type(_V, _Val);
        }

        return TValue(new TString(_V, _Type.cast_ptr<TString>()->data));
    }

    return __type(_V, _Val);
}

VIA_INLINE_HOT void __native_call(State* _V, TFunction* _Callee, SIZE _Argc) {
    _Callee->caller   = _V->frame;
    _Callee->ret_addr = _V->ip;
    _V->frame         = _Callee;
    _V->sibp          = _V->ibp;
    _V->siep          = _V->iep;
    _V->ip            = _Callee->bytecode;
    _V->ibp           = _Callee->bytecode;
    _V->iep           = _Callee->bytecode + _Callee->bytecode_len;
    _V->argc          = _Argc;
    _V->ssp           = _V->sp;
}

VIA_INLINE_HOT void __extern_call(State* _V, TCFunction* _Callee, SIZE _Argc) {
    char        _Buf[2 + std::numeric_limits<uintptr_t>::digits / 4 + 1];
    const void* _Addr    = _Callee;
    uintptr_t   _Address = reinterpret_cast<uintptr_t>(_Addr);
    _Buf[0]              = '0';
    _Buf[1]              = 'x';

    auto _Result = std::to_chars(_Buf + 2, _Buf + sizeof(_Buf), _Address, 16);
    if (_Result.ec != std::errc{}) {
        std::memset(_Buf + 2, '0', sizeof(_Buf) - 2);
    }

    TFunction _Func{
        .line             = 0,
        .id               = "<cfunction>",
        .is_error_handler = _Callee->is_error_handler,
        .is_vararg        = false,
        .ret_addr         = _V->ip,
        .caller           = _V->frame,
    };

    __native_call(_V, &_Func, _Argc);
    _Callee->data(_V);
}

VIA_INLINE_HOT void __call(State* _V, const TValue& _Callee, SIZE _Argc) {
    _V->calltype = CallType::CALL;

    if (check_function(_Callee)) {
        __native_call(_V, _Callee.cast_ptr<TFunction>(), _Argc);
    }
    else if (check_cfunction(_Callee)) {
        __extern_call(_V, _Callee.cast_ptr<TCFunction>(), _Argc);
    }
    else {
        __set_error_state(
            _V, std::format("attempt to call a {} value", __type_cxx_string(_V, _Callee))
        );
    }
}

VIA_FORCE_INLINE TValue __len(const TValue& _Val) noexcept {
    if (check_string(_Val)) {
        return TValue(static_cast<TInteger>(_Val.cast_ptr<TString>()->len));
    }
    else if (check_table(_Val)) {
        SIZE _Size = __table_size(_Val.cast_ptr<TTable>());
        return TValue(static_cast<TInteger>(_Size));
    }

    return _Nil.clone();
}

VIA_FORCE_INLINE void __native_return(State* VIA_RESTRICT _V, const TValue& _Ret_value) noexcept {
    __closure_close_upvalues(_V->frame);

    _V->ibp   = _V->sibp;
    _V->iep   = _V->siep;
    _V->ip    = _V->frame->ret_addr;
    _V->frame = _V->frame->caller;

    _V->sp = _V->ssp;
    _V->sp -= _V->argc;

    __push(_V, _Ret_value);
}

VIA_INLINE_HOT TValue __get_global(State* VIA_RESTRICT _V, Operand _Id) noexcept {
    std::lock_guard<std::mutex> lock(_V->G->gtable_mutex);

    auto _It = _V->G->gtable.find(_Id);
    if (_It != _V->G->gtable.end()) {
        return _It->second.clone();
    }

    return _Nil.clone();
}

VIA_FORCE_INLINE void __set_global(State* VIA_RESTRICT _V, Operand _Id, const TValue& _Val) {
    std::lock_guard<std::mutex> lock(_V->G->gtable_mutex);

    auto _It = _V->G->gtable.find(_Id);
    if (_It != _V->G->gtable.end()) {
        __set_error_state(_V, std::format("attempt to reassign global '{}'", _Id));
    }

    _V->G->gtable.emplace(_Id, _Val.clone());
}

VIA_INLINE TValue __to_string(State* VIA_RESTRICT _V, const TValue& _Val) noexcept {
    using enum ValueType;

    if (check_string(_Val)) {
        return _Val.clone();
    }

    switch (_Val.type) {
    case integer: {
        std::string _Str  = std::to_string(_Val.val_integer);
        TString*    _Tstr = new TString(_V, _Str.c_str());
        return TValue(string, _Tstr);
    }
    case floating_point: {
        std::string _Str  = std::to_string(_Val.val_floating_point);
        TString*    _Tstr = new TString(_V, _Str.c_str());
        return TValue(string, _Tstr);
    }
    case boolean: {
        TString* _Str = new TString(_V, _Val.val_boolean ? "true" : "false");
        return TValue(string, _Str);
    }
    case table:
    case function:
    case cfunction: {
        auto _Type_str = magic_enum::enum_name(_Val.type);
        auto _Final_str =
            std::format("<{}@0x{:x}>", _Type_str, reinterpret_cast<uintptr_t>(_Val.val_pointer));

        TString* _Str = new TString(_V, _Final_str.c_str());
        return TValue(string, _Str);
    }
    default:
        TString* _Tstr = new TString(_V, "nil");
        return TValue(string, _Tstr);
    }

    VIA_UNREACHABLE;
    return _Nil.clone();
}

VIA_FORCE_INLINE std::string __to_cxx_string(State* VIA_RESTRICT _V, const TValue& _Val) noexcept {
    TValue _Str = __to_string(_V, _Val);
    return std::string(_Str.cast_ptr<TString>()->data);
}

VIA_FORCE_INLINE TValue __to_bool(const TValue& _Val) noexcept {
    if (check_bool(_Val)) {
        return _Val.clone();
    }

    return TValue(_Val.type != ValueType::nil);

    VIA_UNREACHABLE;
    return _Nil.clone();
}

VIA_FORCE_INLINE bool __to_cxx_bool(const TValue& _Val) noexcept {
    return __to_bool(_Val).val_boolean;
}

VIA_FORCE_INLINE TValue __to_number(const TValue& _Val) noexcept {
    using enum ValueType;

    if (check_number(_Val)) {
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
VIA_FORCE_INLINE T __to_cxx_number(const TValue& _Val) noexcept {
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

VIA_FORCE_INLINE void* __to_pointer(const TValue& _Val) noexcept {
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

VIA_INLINE_HOT bool __compare(const TValue& _Val_0, const TValue& _Val_1) noexcept {
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

VIA_INLINE TValue
__weak_primitive_cast(State* VIA_RESTRICT _V, const TValue& _Val, ValueType _Type) {
    using enum ValueType;

    switch (_Type) {
    case integer:
    case floating_point:
        return __to_number(_Val);
    case boolean:
        return __to_bool(_Val);
    case string:
        return __to_string(_V, _Val);
    default:
        break;
    }

    return _Nil.clone();
}

VIA_INLINE void __strong_primitive_cast(State* VIA_RESTRICT _V, TValue& _Val, ValueType _Type) {
    using enum ValueType;

    switch (_Type) {
    case integer: {
        int _Num         = __to_cxx_number<int>(_Val);
        _Val.val_integer = _Num;
        break;
    }
    case floating_point: {
        float _Num = __to_cxx_number<float>(_Val);
        if (_Num != _Num) {
            goto error;
        }

        _Val.val_floating_point = _Num;
        break;
    }
    case boolean:
        _Val.val_boolean = __to_cxx_bool(_Val);
        break;
    case string: {
        TValue   _Non_owned_val = __to_string(_V, _Val);
        TString* _Owned_val     = _Val.cast_ptr<TString>();

        _Val.val_pointer = new TString(_V, _Non_owned_val.cast_ptr<TString>()->data);

        if (_Owned_val) {
            delete _Owned_val;
        }

        break;
    }
    default:
        goto error;
    }

    _Val.type = _Type;
    return;
error:
    __set_error_state(
        _V,
        std::format(
            "type '{}' is not primitive castable into type '{}'",
            magic_enum::enum_name(_Val.type),
            magic_enum::enum_name(_Type)
        )
    );
}

VIA_NAMESPACE_END

#endif
