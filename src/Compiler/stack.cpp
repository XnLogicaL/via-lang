// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "stack.h"

VIA_NAMESPACE_BEGIN

using index_query_result  = CompilerStack::index_query_result;
using find_query_result   = CompilerStack::find_query_result;
using function_stack_node = CompilerStack::function_stack_node;
using function_stack_type = CompilerStack::function_stack_type;
using symbol              = CompilerStack::symbol;

size_t CompilerStack::size() {
    return sp;
}

void CompilerStack::push(StackObject val) {
    if (sp >= capacity) {
        grow_stack();
    }

    sbp[sp++] = {val.is_const, val.is_constexpr, val.symbol, val.type->clone()};
}

StackObject CompilerStack::pop() {
    StackObject& val = sbp[sp--];
    return {val.is_const, val.is_const, val.symbol, val.type->clone()};
}

StackObject CompilerStack::top() {
    StackObject& obj = sbp[sp--];
    return {obj.is_const, obj.is_constexpr, obj.symbol, obj.type->clone()};
}

index_query_result CompilerStack::at(size_t pos) {
    if (pos > size()) {
        return std::nullopt;
    }

    StackObject& obj = sbp[pos];
    return StackObject{obj.is_const, obj.is_constexpr, obj.symbol, obj.type->clone()};
}

find_query_result CompilerStack::find_symbol(const symbol& symbol) {
    for (StackObject* stk_id = sbp; stk_id < sbp + sp; stk_id++) {
        if (stk_id->symbol == symbol) {
            return stk_id - sbp;
        }
    }

    return std::nullopt;
}

find_query_result CompilerStack::find_symbol(const StackObject& member) {
    return find_symbol(member.symbol);
}

void CompilerStack::grow_stack() {
    size_t old_capacity = capacity;
    size_t new_capacity = old_capacity * 2;

    StackObject* old_location = sbp;
    StackObject* new_location = new StackObject[new_capacity];

    for (StackObject* obj = old_location; obj < old_location + old_capacity; obj++) {
        size_t position        = obj - old_location;
        new_location[position] = std::move(*obj);
    }

    delete[] old_location;

    sbp      = new_location;
    capacity = new_capacity;
}

VIA_NAMESPACE_END
