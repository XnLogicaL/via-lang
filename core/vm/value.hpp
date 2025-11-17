/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <cstdint>
#include <optional>
#include <stdfloat>
#include <via/config.hpp>
#include "closure.hpp"
#include "machine.hpp"
#include "sema/const.hpp"

namespace via {

#if defined(VIA_COMPILER_GCC) || defined(VIA_COMPILER_CLANG)
using float64 = _Float64;
#else
using float64 = double;
#endif

class Value final
{
  public:
    union Union {
        int64_t integer;
        float64 float_;
        bool boolean;
        char* string;
        Closure* function;
    };

    friend class ValueRef;
    friend class VirtualMachine;

    template <bool, bool>
    friend void detail::execute(VirtualMachine*);

  public:
    static Value* create(VirtualMachine* vm);
    static Value* create(VirtualMachine* vm, int64_t integer);
    static Value* create(VirtualMachine* vm, float64 float_);
    static Value* create(VirtualMachine* vm, bool boolean);
    static Value* create(VirtualMachine* vm, char* string);
    static Value* create(VirtualMachine* vm, Closure* closure);
    static Value* create(VirtualMachine* vm, const ConstValue& cv);

  public:
    auto kind() const { return m_kind; }
    auto& data() { return m_data; }
    const auto& data() const { return m_data; }
    auto* context() const { return m_vm; }
    bool unref() noexcept;
    void free() noexcept;
    Value* clone() noexcept;
    auto bool_value() const noexcept { return m_data.boolean; }
    auto int_value() const noexcept { return m_data.integer; }
    auto float_value() const noexcept { return m_data.float_; }
    auto string_value() const noexcept { return m_data.string; }
    auto function_value() const noexcept { return m_data.function; }
    std::optional<int64_t> as_cint() const;
    std::optional<float64> as_cfloat() const;
    bool as_cbool() const;
    std::string as_cstring() const;
    Value* as_int() const;
    Value* as_float() const;
    Value* as_bool() const;
    Value* as_string() const;
    std::string to_string() const noexcept;

  private:
    static Value* create(VirtualMachine* vm, ValueKind kind, Union data = {});

  private:
    ValueKind m_kind = ValueKind::NIL;
    Union m_data = {};
    uint64_t m_rc = 1;
    VirtualMachine* m_vm;
};

} // namespace via
