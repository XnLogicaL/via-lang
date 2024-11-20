/* This file is a part of the via programming language at
 * https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "Utils/modifiable_once.h"
#include "common.h"
#include "instruction.h"

namespace via::VM
{

struct viaState;

using __via_instruction = Compilation::viaInstruction;

using viaNumber = double;
using Bool = bool;
using String = char *;
using Nil = std::nullptr_t;
using Ptr = uintptr_t;
using CFunc = void (*)(viaState *);

struct viaValue; // Forward declaration
struct viaTable; // Forward declaration

struct Func
{
    const __via_instruction *addr;
};

// Tagged union that holds a table key
struct viaTableKey
{
    enum class __type : uint8_t
    {
        viaNumber,
        String
    };

    __type type;
    union
    {
        viaNumber num;
        String str;
    };

    viaTableKey(double n)
        : type(__type::viaNumber)
        , num(n)
    {
    }
    viaTableKey(const char *c)
        : type(__type::String)
        , str(const_cast<char *>(c))
    {
    }
};

// Tagged union that holds a primitive via value
struct viaValue
{
    enum class __type : uint8_t
    {
        viaNumber,
        Bool,
        String,
        Nil,
        Ptr,
        Func,
        CFunc,
        viaTable,
    };

    // clang-format off
    viaValue() : type(__type::Nil), nil(nullptr) {}
    viaValue(const String &c) : type(__type::String), str(strdup(c)) {}
    explicit viaValue(const viaNumber &d) : type(__type::viaNumber), num(d) {}
    explicit viaValue(const Bool &b) : type(__type::Bool), boole(b) {}
    explicit viaValue(const Ptr &p) : type(__type::Ptr), ptr(p) {}
    explicit viaValue(const Func &f) : type(__type::Func), fun(new Func(f)) {}
    explicit viaValue(CFunc cf) : type(__type::CFunc), cfun(CFunc(cf)) {}

    viaValue(const viaValue &other);
    viaValue &operator=(const viaValue &other);
    explicit viaValue(const viaTable &t);
    // clang-format on

    ~viaValue()
    {
        if (type == __type::String)
        {
            std::free(str);
        }
    };

    bool _const;
    __type type;

    union
    {
        viaNumber num;
        Bool boole;
        String str;
        Nil nil;
        Ptr ptr;
        Func *fun;
        CFunc cfun;
        viaTable *tbl;
    };

private:
    void cleanup();
};

struct viaTable
{
    struct __hash
    {
        std::size_t operator()(const viaTableKey &) const;
    };

    struct __eq
    {
        bool operator()(const viaTableKey &, const viaTableKey &) const;
    };

    viaTable *meta;
    util::modifiable_once<bool> frozen;
    std::unordered_map<viaTableKey, viaValue, __hash, __eq> data;

    viaTable()
        : meta(nullptr)
        , frozen(false)
        , data({})
    {
    }
};

using viaValueType = viaValue::__type;

} // namespace via::VM
