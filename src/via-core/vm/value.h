/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.h>
#include <via/types.h>
#include "closure.h"
#include "machine.h"
#include "sema/const_value.h"
#include "support/option.h"

namespace via {

class Value final
{
  public:
    union Union {
        i64 integer;
        f64 float_;
        bool boolean;
        char* string;
        Closure* function;
    };

    friend class ValueRef;
    friend class VirtualMachine;

    template <bool, bool>
    friend void detail::__execute(VirtualMachine*);

  public:
    static Value* construct(VirtualMachine* vm)
    {
        return construct_impl(vm, ValueKind::NIL);
    }
    static Value* construct(VirtualMachine* vm, i64 integer)
    {
        return construct_impl(vm, ValueKind::INT, {.integer = integer});
    }

    static Value* construct(VirtualMachine* vm, f64 float_)
    {
        return construct_impl(vm, ValueKind::FLOAT, {.float_ = float_});
    }

    static Value* construct(VirtualMachine* vm, bool boolean)
    {
        return construct_impl(vm, ValueKind::BOOL, {.boolean = boolean});
    }

    static Value* construct(VirtualMachine* vm, char* string)
    {
        debug::require(
            vm->get_allocator().owns(string),
            "Value construction via string requires it to be allocated by "
            "the corresponding Value::vm"
        );
        return construct_impl(vm, ValueKind::STRING, {.string = string});
    }

    static Value* construct(VirtualMachine* vm, Closure* closure)
    {
        debug::require(
            vm->get_allocator().owns(closure),
            "Value construction via closure object requires it to be allocated by "
            "the corresponding Value::vm"
        );
        return construct_impl(vm, ValueKind::FUNCTION, {.function = closure});
    }

    static Value* construct(VirtualMachine* vm, const sema::ConstValue& cv)
    {
        auto& alloc = vm->get_allocator();

        switch (cv.kind()) {
        case ValueKind::NIL:
            return construct(vm);
        case ValueKind::BOOL:
            return construct(vm, cv.value<ValueKind::BOOL>());
        case ValueKind::INT:
            return construct(vm, cv.value<ValueKind::INT>());
        case ValueKind::FLOAT:
            return construct(vm, cv.value<ValueKind::FLOAT>());
        case ValueKind::STRING: {
            auto buf = alloc.strdup(cv.value<ValueKind::STRING>().c_str());
            return construct(vm, buf);
        }
        default:
            break;
        }

        debug::unimplemented();
    }

  public:
    inline auto kind() const { return m_kind; }
    inline auto& data() { return m_data; }
    inline const auto& data() const { return m_data; }
    inline auto* context() const { return m_vm; }

    inline bool unref()
    {
        m_rc--;
        [[unlikely]] if (m_rc == 0) {
            free();
            return true;
        }
        return false;
    }

    inline void free()
    {
        switch (m_kind) {
        case ValueKind::STRING: // Trivial destruction:
            m_vm->get_allocator().free(std::bit_cast<void*>(m_data));
            break;
        case ValueKind::FUNCTION:
            m_vm->get_allocator().free<Closure>(m_data.function);
            break;
        default:
            // Trivial types don't require explicit destruction
            break;
        }

        m_kind = ValueKind::NIL;
    }

    inline Value* clone() noexcept { return construct_impl(m_vm, m_kind, m_data); }

    inline bool bool_value() const noexcept { return m_data.boolean; }
    inline i64 int_value() const noexcept { return m_data.integer; }
    inline f64 float_value() const noexcept { return m_data.float_; }
    inline char* string_value() const noexcept { return m_data.string; }
    inline Closure* function_value() const noexcept { return m_data.function; }

    inline Option<i64> as_cint() const { return nullopt; }
    inline Option<f64> as_cfloat() const { return nullopt; }
    inline bool as_cbool() const { return false; }
    inline std::string as_cstring() const
    {
        switch (m_kind) {
        case ValueKind::NIL:
            return "nil";
        case ValueKind::BOOL:
            return std::to_string(m_data.boolean);
        case ValueKind::INT:
            return std::to_string(m_data.integer);
        case ValueKind::FLOAT:
            return std::to_string(m_data.float_);
        case ValueKind::STRING:
            return m_data.string;
        case ValueKind::FUNCTION:
            return std::format(
                "{}@{}",
                m_data.function->is_native() ? "native" : "function",
                (const void*) m_data.function
            );
        }
    }

    inline Value* as_int() const { return nullptr; /* PLACEHOLDER */ }
    inline Value* as_float() const { return nullptr; /* PLACEHOLDER */ }
    inline Value* as_bool() const { return nullptr; /* PLACEHOLDER */ }
    inline Value* as_string() const { return nullptr; /* PLACEHOLDER */ }

    inline std::string to_string() const noexcept
    {
        return std::format(
            "[rc: {}, has_vm_ref: {}, {}({})]",
            m_rc,
            m_vm != nullptr,
            magic_enum::enum_name(m_kind),
            as_cstring()
        );
    }

  private:
    static inline Value*
    construct_impl(VirtualMachine* vm, ValueKind kind, Value::Union data = {})
    {
        Value* ptr = vm->get_allocator().emplace<Value>();
        ptr->m_kind = kind;
        ptr->m_data = data;
        ptr->m_vm = vm;
        return ptr;
    }

  private:
    ValueKind m_kind = ValueKind::NIL;
    Union m_data = {};
    size_t m_rc = 1; // obviously
    VirtualMachine* m_vm;
};

} // namespace via
