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
#include "machine.h"
#include "sema/const_value.h"
#include "support/option.h"

namespace via {

class Closure;
class Value final
{
  public:
    enum class Kind
    {
        NIL,
        INT,
        FLOAT,
        BOOL,
        STRING,
        CLOSURE,
    };

    union Union {
        i64 integer;
        f64 float_;
        bool boolean;
        char* string;
        Closure* closure;
    };

    friend class ValueRef;
    friend class VirtualMachine;

    template <bool, bool>
    friend void execute_impl(VirtualMachine*);

  public:
    static Value* construct(VirtualMachine* vm) { return construct_impl(vm, Kind::NIL); }
    static Value* construct(VirtualMachine* vm, i64 integer)
    {
        return construct_impl(vm, Kind::INT, {.integer = integer});
    }

    static Value* construct(VirtualMachine* vm, f64 float_)
    {
        return construct_impl(vm, Kind::FLOAT, {.float_ = float_});
    }

    static Value* construct(VirtualMachine* vm, bool boolean)
    {
        return construct_impl(vm, Kind::BOOL, {.boolean = boolean});
    }

    static Value* construct(VirtualMachine* vm, char* string)
    {
        debug::require(
            vm->get_allocator().owns(string),
            "Value construction via string requires it to be allocated by "
            "the corresponding Value::vm"
        );
        return construct_impl(vm, Kind::STRING, {.string = string});
    }

    static Value* construct(VirtualMachine* vm, Closure* closure)
    {
        debug::require(
            vm->get_allocator().owns(closure),
            "Value construction via closure object requires it to be allocated by "
            "the corresponding Value::vm"
        );
        return construct_impl(vm, Kind::CLOSURE, {.closure = closure});
    }

    static Value* construct(VirtualMachine* vm, const sema::ConstValue& cv)
    {
        using enum sema::ConstValue::Kind;

        auto& alloc = vm->get_allocator();

        switch (cv.kind()) {
            case NIL:
                return construct(vm);
            case BOOL:
                return construct(vm, cv.value<BOOL>());
            case INT:
                return construct(vm, cv.value<INT>());
            case FLOAT:
                return construct(vm, cv.value<FLOAT>());
            case STRING: {
                auto buf = alloc.strdup(cv.value<STRING>().c_str());
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
    inline auto* context() const { return vm; }

    inline void free()
    {
        switch (m_kind) {
            case Kind::STRING:
            case Kind::CLOSURE:
                vm->get_allocator().free(std::bit_cast<void*>(m_data));
                break;
            default:
                // Trivial types don't require explicit destruction
                break;
        }

        m_kind = Kind::NIL;
    }

    inline Value* clone() { return construct_impl(vm, m_kind, m_data); }

    // Totally safe access methods
    inline i64 get_int() const { return m_data.integer; }
    inline f64 get_float() const { return m_data.float_; }
    inline bool get_bool() const { return m_data.boolean; }
    inline char* get_string() const { return m_data.string; }

    inline Option<i64> as_cint() const { return nullopt; /* PLACEHOLDER */ }
    inline Option<f64> as_cfloat() const { return nullopt; /* PLACEHOLDER */ }
    inline bool as_cbool() const { return false; /* PLACEHOLDER */ }
    inline char* as_cstring() const { return nullptr; /* PLACEHOLDER */ }

    inline Value* as_int() const { return nullptr; /* PLACEHOLDER */ }
    inline Value* as_float() const { return nullptr; /* PLACEHOLDER */ }
    inline Value* as_bool() const { return nullptr; /* PLACEHOLDER */ }
    inline Value* as_string() const { return nullptr; /* PLACEHOLDER */ }

  private:
    static inline Value* construct_impl(VirtualMachine* vm, Value::Kind kind, Value::Union data = {})
    {
        Value* ptr = vm->get_allocator().emplace<Value>();
        ptr->m_kind = kind;
        ptr->m_data = data;
        ptr->vm = vm;
        return ptr;
    }

  private:
    Kind m_kind = Kind::NIL;
    Union m_data = {};
    usize m_rc = 0;
    VirtualMachine* vm = nullptr;
};

} // namespace via
