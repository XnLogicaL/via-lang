// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_API_H
#define _VIA_API_H

#include "common.h"
#include "gc.h"
#include "instruction.h"
#include "opcode.h"
#include "state.h"
#include "rttypes.h"
#include "vmapi.h"
#include <cmath>

VIA_NAMESPACE_BEGIN

static const TValue nil = TValue();

TValue* get_register(State*, U32 reg);
void    set_register(State*, U32 reg, const TValue& val);

void* to_pointer(const TValue& val) noexcept;
bool  is_heap(const TValue& val) noexcept;
bool  compare(const TValue& v0, const TValue& v1) noexcept;

void   push(State* VIA_RESTRICT, const TValue& val);
TValue pop(State* VIA_RESTRICT);
TValue top(State* VIA_RESTRICT);

template<typename T>
    requires std::is_arithmetic_v<T>
T           to_cxx_number(const TValue& val) noexcept;
TValue      to_string(State* VIA_RESTRICT, const TValue& val) noexcept;
std::string to_cxx_string(State* VIA_RESTRICT, const TValue& val) noexcept;
TValue      to_bool(const TValue& val) noexcept;
bool        to_cxx_bool(const TValue& val) noexcept;
TValue      to_number(const TValue& val) noexcept;

TValue get_table(TTable* VIA_RESTRICT tbl, U32 key, bool search_meta) noexcept;
void   set_table(TTable* VIA_RESTRICT tbl, U32 key, const TValue& val) noexcept;
TValue get_metamethod(const TValue& val, OpCode op);

const TValue& get_local(State* VIA_RESTRICT, U32 offset) noexcept;
const TValue& get_global(State* VIA_RESTRICT, U32 ident) noexcept;
void          set_global(State* VIA_RESTRICT, U32, const TValue&);
const TValue& get_argument(State* VIA_RESTRICT, U32) noexcept;

void native_return(State* VIA_RESTRICT, SIZE) noexcept;
void native_call(State* VIA_RESTRICT, TFunction* VIA_RESTRICT, SIZE) noexcept;
void method_call(State* VIA_RESTRICT, TTable* VIA_RESTRICT, U32, SIZE) noexcept;
void call(State* VIA_RESTRICT, const TValue&, SIZE argc) noexcept;

TValue len(State* VIA_RESTRICT, const TValue&) noexcept;
TValue type(State* VIA_RESTRICT, const TValue&) noexcept;
TValue typeofv(State* VIA_RESTRICT, const TValue&) noexcept;
void   freeze(TTable* VIA_RESTRICT tbl) noexcept;

TValue weak_primitive_cast(State* VIA_RESTRICT, const TValue& val, ValueType type);
void   strong_primitive_cast(State* VIA_RESTRICT, TValue& val, ValueType type);

VIA_NAMESPACE_END

#endif
