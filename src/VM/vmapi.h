/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "gc.h"
#include "instruction.h"
#include "opcode.h"
#include "register.h"
#include "state.h"
#include "types.h"

#define CHECK_JUMP_ADDRESS(addr) ((addr >= V->ihp) && (addr <= V->ibp))

namespace via::impl
{

static const TValue _Nil = TValue();

VIA_MAXOPTIMIZE void __seterrorstate(State *_V, const std::string &_Msg)
{
    _V->err->frame = _V->frame;
    _V->err->message = _Msg;
}

VIA_MAXOPTIMIZE void __clearerrorstate(State *_V)
{
    _V->err->frame = nullptr;
    _V->err->message = "";
}

VIA_MAXOPTIMIZE bool __haserror(State *_V)
{
    return _V->err->frame == _V->frame;
}

VIA_MAXOPTIMIZE void __setregister(State *_V, RegId _Reg, const TValue &_Val)
{
    TValue *addr = _V->ralloc->head + _Reg;
    *addr = _Val.clone();
}

VIA_MAXOPTIMIZE TValue *__getregister(State *_V, RegId _Reg)
{
    return _V->ralloc->head + _Reg;
}

VIA_MAXOPTIMIZE void __push(State *_V, const TValue &_Val)
{
    constexpr static const size_t _Stack_size = VIA_VM_STACK_SIZE / sizeof(TValue);
    if (VIA_UNLIKELY(_V->sp > _Stack_size))
    {
        __seterrorstate(_V, "stack overflow");
        return;
    }

    _V->sbp[_V->sp++] = _Val.clone();
}

VIA_MAXOPTIMIZE TValue __pop(State *_V)
{
    if (VIA_UNLIKELY(_V->sp == 0))
    {
        __seterrorstate(_V, "stack underflow");
        return _Nil.clone();
    }

    return _V->sbp[_V->sp--].clone();
}

VIA_FORCEINLINE TValue __getargument(State *VIA_RESTRICT _V, LocalId _Offset) noexcept
{
    if (_Offset >= _V->argc)
        return _Nil.clone();
    const StkPos _Stk_offset = _V->ssp + _V->argc - 1 - _Offset;
    const TValue &_Val = _V->sbp[_Stk_offset];
    return _Val.clone();
}

VIA_FORCEINLINE TValue __type(State *VIA_RESTRICT _V, const TValue &_Val) noexcept
{
    char *_Str = dupstring(std::string(ENUM_NAME(_Val.type)));
    gcaddcallback(_V, [&_Str]() { delete _Str; });
    return TValue(new TString(_V, _Str));
}

VIA_FORCEINLINE std::string __typecxxstring(State *VIA_RESTRICT _V, const TValue &_Val)
{
    TValue _Type = __type(_V, _Val);
    return std::string(_Type.val_string->ptr);
}

VIA_INLINE TValue __gettable(TTable *VIA_RESTRICT _Tbl, TableKey _Key, bool _Search_meta) noexcept
{
    auto _It = _Tbl->data.find(_Key);
    if (_It != _Tbl->data.end())
        return _It->second.clone();
    else if (_Search_meta && _Tbl->meta)
        return __gettable(_Tbl->meta, _Key, false);
    return _Nil.clone();
}

VIA_FORCEINLINE void __settable(TTable *VIA_RESTRICT _Tbl, TableKey _Key, const TValue &_Val) noexcept
{
    if (checknil(_Val))
    {
        const TValue &_Tbl_val = __gettable(_Tbl, _Key, false);
        if (!checknil(_Tbl_val))
            _Tbl->data.erase(_Key);
    }
    else
        _Tbl->data.emplace(_Key, _Val.clone());
}

VIA_FORCEINLINE TValue __typeofv(State *VIA_RESTRICT _V, const TValue &_Val)
{
    if (checktable(_Val))
    {
        TTable *_Tbl = _Val.val_table;
        const TValue &ty = __gettable(_Tbl, hashstring("__type"), true);
        if (checknil(ty))
            return __type(_V, _Val);
        return TValue(new TString(_V, ty.val_string->ptr));
    }

    return __type(_V, _Val);
}

VIA_MAXOPTIMIZE void __nativecall(State *_V, TFunction *_Callee, size_t _Argc)
{
    _Callee->caller = _V->frame;
    _Callee->ret_addr = _V->ip;
    _V->frame = _Callee;
    _V->ip = _Callee->bytecode.data();
    _V->argc = _Argc;
    _V->ssp = _V->sp;
}

VIA_MAXOPTIMIZE void __externcall(State *_V, TCFunction *_Callee, size_t _Argc)
{
    char _Buf[2 + std::numeric_limits<uintptr_t>::digits / 4 + 1];
    const void *_Addr = _Callee;
    uintptr_t _Address = reinterpret_cast<uintptr_t>(_Addr);
    _Buf[0] = '0';
    _Buf[1] = 'x';

    auto _Result = std::to_chars(_Buf + 2, _Buf + sizeof(_Buf), _Address, 16);
    if (_Result.ec != std::errc{})
        std::memset(_Buf + 2, '0', sizeof(_Buf) - 2);

    TFunction _Func(0, std::string(_Buf, _Result.ptr), _V->ip, _V->frame, {}, _Callee->error_handler, false);

    __nativecall(_V, &_Func, _Argc);
    _Callee->ptr(_V);
}

VIA_MAXOPTIMIZE void __methodcall(State *VIA_RESTRICT _V, TTable *VIA_RESTRICT _Tbl, TableKey _Key, size_t _Argc) noexcept
{
    const TValue &_Method = __gettable(_Tbl, _Key, true);
    if (checkfunction(_Method))
        __nativecall(_V, _Method.val_function, _Argc);
    else if (checkcfunction(_Method))
        __externcall(_V, _Method.val_cfunction, _Argc);
}

VIA_MAXOPTIMIZE void __call(State *_V, const TValue &_Callee, size_t _Argc)
{
    _V->calltype = CallType::CALL;

    if (checkfunction(_Callee))
        __nativecall(_V, _Callee.val_function, _Argc);
    else if (checkcfunction(_Callee))
        __externcall(_V, _Callee.val_cfunction, _Argc);
    else if (checktable(_Callee))
        __methodcall(_V, _Callee.val_table, hashstring("__call"), _Argc);
    else
        __seterrorstate(_V, std::format("attempt to call a {} value", __typecxxstring(_V, _Callee)));
}

VIA_FORCEINLINE TValue __len(State *VIA_RESTRICT _V, const TValue &_Val) noexcept
{
    if (checkstring(_Val))
        return TValue(static_cast<TNumber>(strlen(_Val.val_string->ptr)));
    else if (checktable(_Val))
    {
        TableKey _Metamethod_key = hashstring("__len");
        const TValue &_Metamethod = __gettable(_Val.val_table, _Metamethod_key, true);

        if (checknil(_Metamethod))
            return TValue(static_cast<TNumber>(_Val.val_table->data.size()));

        __call(_V, _Metamethod, 1);
        return __pop(_V);
    }

    return _Nil.clone();
}

VIA_FORCEINLINE void __nativeret(State *VIA_RESTRICT _V, size_t _Retc) noexcept
{
    std::vector<TValue> _Ret_values;
    _V->ip = _V->frame->ret_addr;
    _V->frame = _V->frame->caller;

    for (size_t i = 0; i < _Retc; i++)
    {
        TValue _Ret_val = __pop(_V);
        _Ret_values.push_back(std::move(_Ret_val));
    }

    _V->sp = _V->ssp;

    for (size_t i = 0; i < _V->argc; i++)
        __pop(_V);

    for (int i = _Retc - 1; i >= 0; i--)
        __push(_V, _Ret_values.at(i));
}

VIA_MAXOPTIMIZE TValue __getglobal(State *VIA_RESTRICT _V, kGlobId _Id) noexcept
{
    auto _It = _V->G->gtable.find(_Id);
    if (_It != _V->G->gtable.end())
        return _It->second.clone();
    return _Nil.clone();
}

VIA_INLINE TValue __tostring(State *VIA_RESTRICT _V, const TValue &_Val) noexcept
{
    if (checkstring(_Val))
        return _Val.clone();

    switch (_Val.type)
    {
    case ValueType::number:
    {
        std::string _Str = std::to_string(_Val.val_number);
        TString *_Tstr = new TString(_V, _Str.c_str());
        return TValue(_Tstr);
    }
    case ValueType::boolean:
    {
        TString *_Str = new TString(_V, _Val.val_boolean ? "true" : "false");
        return TValue(_Str);
    }
    case ValueType::table:
    {
        std::string _Str = "{";

        for (auto &_Elem : _Val.val_table->data)
        {
            _Str += __tostring(_V, _Elem.second).val_string->ptr;
            _Str += ", ";
        }

        if (_Str.back() == ' ')
            _Str += "\b\b";

        _Str += "}";

        TString *_Tstr = new TString(_V, _Str.c_str());
        return TValue(_Tstr);
    }
    case ValueType::function:
    {
        const void *_Faddr = _Val.val_function;
        std::string _Str = std::format("<function@{}>", _Faddr);
        TString *_Tstr = new TString(_V, _Str.c_str());
        return TValue(_Tstr);
    }
    case ValueType::cfunction:
    {
        // This has to be explicitly casted because function pointers be weird
        const void *_Cfaddr = _Val.val_cfunction;
        std::string _Str = std::format("<cfunction@{}>", _Cfaddr);
        TString *_Tstr = new TString(_V, _Str.c_str());
        return TValue(_Tstr);
    }
    default:
        TString *_Tstr = new TString(_V, "_Nil");
        return TValue(_Tstr);
    }

    VIA_UNREACHABLE();
    return _Nil.clone();
}

VIA_FORCEINLINE std::string __tocxxstring(State *VIA_RESTRICT _V, const TValue &_Val) noexcept
{
    TValue _Str = __tostring(_V, _Val);
    return std::string(_Str.val_string->ptr);
}

VIA_FORCEINLINE TValue __tobool(const TValue &_Val) noexcept
{
    if (checkbool(_Val))
        return _Val.clone();

    switch (_Val.type)
    {
    // Nil and Monostate is the only falsy type
    case ValueType::nil:
    case ValueType::monostate:
        return TValue(false);
    default:
        return TValue(true);
    }

    VIA_UNREACHABLE();
    return _Nil.clone();
}

VIA_FORCEINLINE bool __tocxxbool(const TValue &_Val) noexcept
{
    return __tobool(_Val).val_boolean;
}

VIA_FORCEINLINE TValue __tonumber(const TValue &_Val) noexcept
{
    if (checknumber(_Val))
        return _Val.clone();

    switch (_Val.type)
    {
    case ValueType::string:
        return TValue(std::stod(_Val.val_string->ptr));
    case ValueType::boolean:
        return TValue(_Val.val_boolean ? 1.0f : 0.0f);
    default:
        break;
    }

    return _Nil.clone();
}

template<typename T = double>
    requires std::is_arithmetic_v<T>
VIA_FORCEINLINE T __tocxxnumber(const TValue &_Val) noexcept
{
    TValue _Double = __tonumber(_Val);

    if (checknil(_Double))
        return std::numeric_limits<T>::quiet_NaN();

    return static_cast<T>(_Double.val_number);
}

VIA_FORCEINLINE void *__topointer(const TValue &_Val) noexcept
{
    switch (_Val.type)
    {
    case ValueType::cfunction:
        return _Val.val_cfunction;
    case ValueType::function:
        return _Val.val_function;
    case ValueType::table:
        return _Val.val_table;
    case ValueType::string:
        return _Val.val_string;
    default:
        return nullptr;
    }
}

VIA_MAXOPTIMIZE bool __compare(const TValue &_Val_0, const TValue &_Val_1) noexcept
{
    if (_Val_0.type != _Val_1.type)
        return false;

    switch (_Val_0.type)
    {
    case ValueType::number:
        return _Val_0.val_number == _Val_1.val_number;
    case ValueType::boolean:
        return _Val_0.val_boolean == _Val_1.val_boolean;
    case ValueType::nil:
        return true;
    case ValueType::string:
        return !std::strcmp(_Val_0.val_string->ptr, _Val_1.val_string->ptr);
    default:
        return __topointer(_Val_0) == __topointer(_Val_1);
    }

    VIA_UNREACHABLE();
    return false;
};

VIA_MAXOPTIMIZE bool __compareregisters(State *VIA_RESTRICT _V, RegId _Reg_0, RegId _Reg_1) noexcept
{
    if (_Reg_0 == _Reg_1)
        return true;

    TValue &_Val_0 = *__getregister(_V, _Reg_0);
    TValue &_Val_1 = *__getregister(_V, _Reg_1);
    if (_Val_0.type != _Val_1.type)
        return false;
    return __compare(_Val_0, _Val_1);
}

VIA_MAXOPTIMIZE TValue __getmetamethod(const TValue &_Val, OpCode _Op)
{
    if (!checktable(_Val))
        return _Nil.clone();

#define GET_METHOD(id) (__gettable(_Val.val_table, hashstring(id), true))
    switch (_Op)
    {
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

} // namespace via::impl
