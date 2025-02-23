// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GPL v3.           |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "gc.h"
#include "instruction.h"
#include "opcode.h"
#include "state.h"
#include "types.h"

namespace via::impl {

static const TValue _Nil = TValue();

VIA_MAXOPTIMIZE void __set_error_state(State *_V, const std::string &_Msg)
{
    _V->err->frame = _V->frame;
    _V->err->message = _Msg;
}

VIA_MAXOPTIMIZE void __clear_error_state(State *_V)
{
    _V->err->frame = nullptr;
    _V->err->message = "";
}

VIA_MAXOPTIMIZE bool __has_error(State *_V)
{
    return _V->err->frame != nullptr;
}

VIA_FORCEINLINE bool __handle_error(State *_V)
{
    TFunction *_Current_frame = _V->frame;
    TFunction *_Error_frame = _V->frame;

    while (_Current_frame) {
        if (_Current_frame->error_handler) {
            _V->err->frame = _Current_frame;
            break;
        }
        _Current_frame = _Current_frame->caller;
    }

    if (!_Current_frame) {
        if (_Error_frame) {
            std::cerr << std::format(
                             "<frame@0x{:x}>: {}\n",
                             reinterpret_cast<uintptr_t>(_Error_frame),
                             _V->err->message
                         )
                      << '\n';
        }

        std::unordered_set<TFunction *> visited;
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

VIA_MAXOPTIMIZE void __set_register(State *_V, U32 _Reg, const TValue &_Val)
{
    TValue *addr = _V->registers + _Reg;
    *addr = _Val.clone();
}

VIA_MAXOPTIMIZE TValue *__get_register(State *_V, U32 _Reg)
{
    TValue *addr = _V->registers + _Reg;
    return addr;
}

VIA_INLINE const TValue &__get_constant(State *_V, size_t _Idx)
{
    if (_Idx >= _V->program->constants->size()) {
        std::cout << _Idx << " is not a valid constant\n";
        return _Nil;
    }

    return _V->program->constants->at(_Idx);
}

VIA_MAXOPTIMIZE void __push(State *_V, const TValue &_Val)
{
    constexpr static const size_t _Stack_size = VIA_VM_STACK_SIZE / sizeof(TValue);
    if (VIA_UNLIKELY(_V->sp > _Stack_size)) {
        __set_error_state(_V, "stack overflow");
        return;
    }

    _V->sbp[_V->sp++] = _Val.clone();

    std::cout << magic_enum::enum_name(_V->sbp[_V->sp].type);
}

VIA_MAXOPTIMIZE TValue __pop(State *_V)
{
    if (VIA_UNLIKELY(_V->sp == 0)) {
        __set_error_state(_V, "stack underflow");
        return _Nil.clone();
    }

    return _V->sbp[_V->sp--].clone();
}

VIA_FORCEINLINE TValue __get_argument(State *VIA_RESTRICT _V, U32 _Offset) noexcept
{
    if (_Offset >= _V->argc) {
        return _Nil.clone();
    }

    const U32 _Stk_offset = _V->ssp + _V->argc - 1 - _Offset;
    const TValue &_Val = _V->sbp[_Stk_offset];

    return _Val.clone();
}

VIA_FORCEINLINE TValue __type(State *VIA_RESTRICT _V, const TValue &_Val) noexcept
{
    char *_Str = dup_string(std::string(magic_enum::enum_name(_Val.type)));
    return TValue(new TString(_V, _Str));
}

VIA_FORCEINLINE std::string __type_cxx_string(State *VIA_RESTRICT _V, const TValue &_Val)
{
    TValue _Type = __type(_V, _Val);
    return std::string(_Type.cast_ptr<TString>()->data);
}

VIA_INLINE TValue __get_table(TTable *VIA_RESTRICT _Tbl, U32 _Key, bool _Search_meta) noexcept
{
    auto _It = _Tbl->data.find(_Key);
    if (_It != _Tbl->data.end()) {
        return _It->second.clone();
    }
    else if (_Search_meta && _Tbl->meta) {
        return __get_table(_Tbl->meta, _Key, false);
    }

    return _Nil.clone();
}

VIA_FORCEINLINE void __set_table(TTable *VIA_RESTRICT _Tbl, U32 _Key, const TValue &_Val) noexcept
{
    if (check_nil(_Val)) {
        const TValue &_Tbl_val = __get_table(_Tbl, _Key, false);

        if (!check_nil(_Tbl_val)) {
            _Tbl->data.erase(_Key);
        }
    }
    else {
        _Tbl->data.emplace(_Key, _Val.clone());
    }
}

VIA_FORCEINLINE TValue __typeofv(State *VIA_RESTRICT _V, const TValue &_Val)
{
    if (check_table(_Val)) {
        TTable *_Tbl = _Val.cast_ptr<TTable>();
        const TValue &_Type = __get_table(_Tbl, hash_string("__type"), true);

        if (check_nil(_Type)) {
            return __type(_V, _Val);
        }

        return TValue(new TString(_V, _Type.cast_ptr<TString>()->data));
    }

    return __type(_V, _Val);
}

VIA_MAXOPTIMIZE void __native_call(State *_V, TFunction *_Callee, size_t _Argc)
{
    _Callee->caller = _V->frame;
    _Callee->ret_addr = _V->ip;
    _V->frame = _Callee;
    _V->ip = _Callee->bytecode.data();
    _V->argc = _Argc;
    _V->ssp = _V->sp;
}

VIA_MAXOPTIMIZE void __extern_call(State *_V, TCFunction *_Callee, size_t _Argc)
{
    char _Buf[2 + std::numeric_limits<uintptr_t>::digits / 4 + 1];
    const void *_Addr = _Callee;
    uintptr_t _Address = reinterpret_cast<uintptr_t>(_Addr);
    _Buf[0] = '0';
    _Buf[1] = 'x';

    auto _Result = std::to_chars(_Buf + 2, _Buf + sizeof(_Buf), _Address, 16);
    if (_Result.ec != std::errc{}) {
        std::memset(_Buf + 2, '0', sizeof(_Buf) - 2);
    }

    TFunction _Func(
        0, std::string(_Buf, _Result.ptr), _V->ip, _V->frame, {}, _Callee->error_handler, false
    );

    __native_call(_V, &_Func, _Argc);
    _Callee->data(_V);
}

VIA_MAXOPTIMIZE void __method_call(
    State *VIA_RESTRICT _V,
    TTable *VIA_RESTRICT _Tbl,
    U32 _Key,
    size_t _Argc
) noexcept
{
    const TValue &_Method = __get_table(_Tbl, _Key, true);
    if (check_function(_Method)) {
        __native_call(_V, _Method.cast_ptr<TFunction>(), _Argc);
    }
    else if (check_cfunction(_Method)) {
        __extern_call(_V, _Method.cast_ptr<TCFunction>(), _Argc);
    }
}

VIA_MAXOPTIMIZE void __call(State *_V, const TValue &_Callee, size_t _Argc)
{
    _V->calltype = CallType::CALL;

    if (check_function(_Callee)) {
        __native_call(_V, _Callee.cast_ptr<TFunction>(), _Argc);
    }
    else if (check_cfunction(_Callee)) {
        __extern_call(_V, _Callee.cast_ptr<TCFunction>(), _Argc);
    }
    else if (check_table(_Callee)) {
        __method_call(_V, _Callee.cast_ptr<TTable>(), hash_string("__call"), _Argc);
    }
    else {
        __set_error_state(
            _V, std::format("attempt to call a {} value", __type_cxx_string(_V, _Callee))
        );
    }
}

VIA_FORCEINLINE TValue __len(State *VIA_RESTRICT _V, const TValue &_Val) noexcept
{
    if (check_string(_Val)) {
        return TValue(static_cast<int>(strlen(_Val.cast_ptr<TString>()->data)));
    }
    else if (check_table(_Val)) {
        U32 _Metamethod_key = hash_string("__len");
        const TValue &_Metamethod = __get_table(_Val.cast_ptr<TTable>(), _Metamethod_key, true);

        if (check_nil(_Metamethod)) {
            return TValue(static_cast<int>(_Val.cast_ptr<TTable>()->data.size()));
        }

        __call(_V, _Metamethod, 1);
        return __pop(_V);
    }

    return _Nil.clone();
}

VIA_FORCEINLINE void __native_return(State *VIA_RESTRICT _V, size_t _Retc) noexcept
{
    std::vector<TValue> _Ret_values;
    _V->ip = _V->frame->ret_addr;
    _V->frame = _V->frame->caller;

    for (size_t i = 0; i < _Retc; i++) {
        TValue _Ret_val = __pop(_V);
        _Ret_values.push_back(std::move(_Ret_val));
    }

    _V->sp = _V->ssp;

    for (size_t i = 0; i < _V->argc; i++) {
        __pop(_V);
    }

    for (int i = _Retc - 1; i >= 0; i--) {
        __push(_V, _Ret_values.at(i));
    }
}

VIA_MAXOPTIMIZE TValue __get_global(State *VIA_RESTRICT _V, U32 _Id) noexcept
{
    std::lock_guard<std::mutex> lock(_V->G->gtable_mutex);

    auto _It = _V->G->gtable.find(_Id);
    if (_It != _V->G->gtable.end()) {
        return _It->second.clone();
    }

    return _Nil.clone();
}

VIA_FORCEINLINE void __set_global(State *VIA_RESTRICT _V, U32 _Id, const TValue &_Val)
{
    std::lock_guard<std::mutex> lock(_V->G->gtable_mutex);

    auto _It = _V->G->gtable.find(_Id);
    if (_It != _V->G->gtable.end()) {
        __set_error_state(_V, std::format("attempt to reassign global '{}'", _Id));
    }

    _V->G->gtable.emplace(_Id, _Val.clone());
}

VIA_INLINE TValue __to_string(State *VIA_RESTRICT _V, const TValue &_Val) noexcept
{
    using enum ValueType;

    if (check_string(_Val)) {
        return _Val.clone();
    }

    switch (_Val.type) {
    case integer: {
        std::string _Str = std::format("{}", _Val.val_integer);
        TString *_Tstr = new TString(_V, _Str.c_str());
        return TValue(_Tstr);
    }
    case floating_point: {
        std::string _Str = std::format("{:.2f}", _Val.val_floating_point);
        TString *_Tstr = new TString(_V, _Str.c_str());
        return TValue(_Tstr);
    }
    case boolean: {
        TString *_Str = new TString(_V, _Val.val_boolean ? "true" : "false");
        return TValue(_Str);
    }
    case table: {
        std::string _Str = "{";

        for (auto &_Elem : _Val.cast_ptr<TTable>()->data) {
            _Str += __to_string(_V, _Elem.second).cast_ptr<TString>()->data;
            _Str += ", ";
        }

        if (_Str.back() == ' ') {
            _Str += "\b\b";
        }

        _Str += "}";

        TString *_Tstr = new TString(_V, _Str.c_str());
        return TValue(_Tstr);
    }
    case function: {
        const void *_Faddr = _Val.cast_ptr<TFunction>();
        std::string _Str = std::format("<function@{}>", _Faddr);
        TString *_Tstr = new TString(_V, _Str.c_str());
        return TValue(_Tstr);
    }
    case cfunction: {
        // This has to be explicitly casted because function pointers be
        // weird
        const void *_Cfaddr = _Val.cast_ptr<TCFunction>();
        std::string _Str = std::format("<cfunction@{}>", _Cfaddr);
        TString *_Tstr = new TString(_V, _Str.c_str());
        return TValue(_Tstr);
    }
    default:
        TString *_Tstr = new TString(_V, "nil");
        return TValue(_Tstr);
    }

    VIA_UNREACHABLE;
    return _Nil.clone();
}

VIA_FORCEINLINE std::string __to_cxx_string(State *VIA_RESTRICT _V, const TValue &_Val) noexcept
{
    TValue _Str = __to_string(_V, _Val);
    return std::string(_Str.cast_ptr<TString>()->data);
}

VIA_FORCEINLINE TValue __to_bool(const TValue &_Val) noexcept
{
    if (check_bool(_Val)) {
        return _Val.clone();
    }

    return TValue(_Val.type != ValueType::nil);

    VIA_UNREACHABLE;
    return _Nil.clone();
}

VIA_FORCEINLINE bool __to_cxx_bool(const TValue &_Val) noexcept
{
    return __to_bool(_Val).val_boolean;
}

#include <charconv>
#include <string>
#include <iostream>

VIA_FORCEINLINE TValue __to_number(const TValue &_Val) noexcept
{
    using enum ValueType;

    if (check_number(_Val)) {
        return _Val.clone();
    }

    switch (_Val.type) {
    case string: {
        const std::string &str = _Val.cast_ptr<TString>()->data;
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
VIA_FORCEINLINE T __to_cxx_number(const TValue &_Val) noexcept
{
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

VIA_FORCEINLINE void *__to_pointer(const TValue &_Val) noexcept
{
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

VIA_MAXOPTIMIZE bool __compare(const TValue &_Val_0, const TValue &_Val_1) noexcept
{
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

VIA_MAXOPTIMIZE TValue __get_metamethod(const TValue &_Val, OpCode _Op)
{
    if (!check_table(_Val)) {
        return _Nil.clone();
    }

#define GET_METHOD(id) (__get_table(_Val.cast_ptr<TTable>(), hash_string(id), true))
    switch (_Op) {
    case OpCode::ADD:
        return GET_METHOD("__add");
    case OpCode::SUB:
        return GET_METHOD("__sub");
    case OpCode::MUL:
        return GET_METHOD("__mul");
    case OpCode::DIV:
        return GET_METHOD("__div");
    case OpCode::POW:
        return GET_METHOD("__pow");
    case OpCode::MOD:
        return GET_METHOD("__mod");
    case OpCode::NEG:
        return GET_METHOD("__neg");
    case OpCode::INCREMENT:
        return GET_METHOD("__inc");
    case OpCode::DECREMENT:
        return GET_METHOD("__dec");
    case OpCode::CONCAT:
        return GET_METHOD("__con");
    default:
        break;
    }

    return _Nil.clone();
#undef GET_METHOD
}

VIA_INLINE TValue __weak_primitive_cast(State *VIA_RESTRICT _V, const TValue &_Val, ValueType _Type)
{
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

VIA_INLINE void __strong_primtive_cast(State *VIA_RESTRICT _V, TValue &_Val, ValueType _Type)
{
    using enum ValueType;

    switch (_Type) {
    case integer: {
        int _Num = __to_cxx_number<int>(_Val);
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
        TValue _Non_owned_val = __to_string(_V, _Val);
        TString *_Owned_val = _Val.cast_ptr<TString>();

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

} // namespace via::impl
