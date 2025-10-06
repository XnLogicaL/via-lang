/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "value.h"
#include "debug.h"
#include "sema/const_value.h"
#include "support/memory.h"

via::Value* via::Value::create(VirtualMachine* vm)
{
    return construct_impl(vm, ValueKind::NIL);
}

via::Value* via::Value::create(VirtualMachine* vm, int64_t integer)
{
    return construct_impl(vm, ValueKind::INT, {.integer = integer});
}

via::Value* via::Value::create(VirtualMachine* vm, double_t float_)
{
    return construct_impl(vm, ValueKind::FLOAT, {.float_ = float_});
}

via::Value* via::Value::create(VirtualMachine* vm, bool boolean)
{
    return construct_impl(vm, ValueKind::BOOL, {.boolean = boolean});
}

via::Value* via::Value::create(VirtualMachine* vm, char* string)
{
    debug::require(
        vm->allocator().owns(string),
        "Value construction via string requires it to be allocated by "
        "the corresponding Value::vm"
    );
    return construct_impl(vm, ValueKind::STRING, {.string = string});
}

via::Value* via::Value::create(VirtualMachine* vm, Closure* closure)
{
    debug::require(
        vm->allocator().owns(closure),
        "Value construction via closure object requires it to be allocated by "
        "the corresponding Value::vm"
    );
    return construct_impl(vm, ValueKind::FUNCTION, {.function = closure});
}

via::Value* via::Value::create(VirtualMachine* vm, const sema::ConstValue& cv)
{
    auto& alloc = vm->allocator();

    switch (cv.kind()) {
    case ValueKind::NIL:
        return create(vm);
    case ValueKind::BOOL:
        return create(vm, cv.value<ValueKind::BOOL>());
    case ValueKind::INT:
        return create(vm, cv.value<ValueKind::INT>());
    case ValueKind::FLOAT:
        return create(vm, cv.value<ValueKind::FLOAT>());
    case ValueKind::STRING: {
        auto buf = alloc.strdup(cv.value<ValueKind::STRING>().c_str());
        return create(vm, buf);
    }
    default:
        break;
    }

    debug::unimplemented();
}

bool via::Value::unref() noexcept
{
    m_rc--;
    [[unlikely]] if (m_rc == 0) {
        free();
        return true;
    }
    return false;
}

void via::Value::free() noexcept
{
    switch (m_kind) {
    case ValueKind::STRING:
    case ValueKind::FUNCTION:
        m_vm->allocator().free(std::bit_cast<void*>(m_data));
        break;
    default:
        // Trivial types don't require explicit destruction
        break;
    }

    m_kind = ValueKind::NIL;
}

via::Value* via::Value::clone() noexcept
{
    return construct_impl(m_vm, m_kind, m_data);
}

std::optional<int64_t> via::Value::as_cint() const
{
    switch (m_kind) {
    case ValueKind::FLOAT:
        return (int64_t) float_value();
    case ValueKind::BOOL:
        return (int64_t) bool_value();
    case ValueKind::INT:
        return int_value();
    default:
        return std::nullopt;
    }
}

std::optional<double_t> via::Value::as_cfloat() const
{
    switch (m_kind) {
    case ValueKind::INT:
        return (double_t) int_value();
    case ValueKind::FLOAT:
        return float_value();
    default:
        return std::nullopt;
    }
}

bool via::Value::as_cbool() const
{
    switch (m_kind) {
    case ValueKind::NIL:
        return false;
    case ValueKind::BOOL:
        return bool_value();
    default:
        return true;
    }
}

std::string via::Value::as_cstring() const
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
    debug::unimplemented();
}

via::Value* via::Value::as_int() const
{
    auto val = as_cint();
    return val.has_value() ? create(m_vm, *val) : nullptr;
}

via::Value* via::Value::as_float() const
{
    auto val = as_cfloat();
    return val.has_value() ? create(m_vm, *val) : nullptr;
}

via::Value* via::Value::as_bool() const
{
    return Value::create(m_vm, as_cbool());
}

via::Value* via::Value::as_string() const
{
    auto& alloc = m_vm->allocator();
    auto val = as_cstring();
    return Value::create(m_vm, alloc.strdup(val.c_str()));
}

std::string via::Value::to_string() const noexcept
{
    return std::format(
        "[rc: {}, has_vm_ref: {}, {}({})]",
        m_rc,
        m_vm != nullptr,
        via::to_string(m_kind),
        as_cstring()
    );
}

via::Value*
via::Value::construct_impl(VirtualMachine* vm, ValueKind kind, Value::Union data)
{
    Value* ptr = vm->allocator().emplace<Value>();
    ptr->m_kind = kind;
    ptr->m_data = data;
    ptr->m_vm = vm;
    return ptr;
}
