// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "stack.h"

VIA_NAMESPACE_BEGIN

void CompilerStack::push(StackObject val) {
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

U64 CompilerStack::size() {
    return sp;
}

std::optional<StackObject> CompilerStack::at(SIZE pos) {
    if (pos > size()) {
        return std::nullopt;
    }

    StackObject& obj = sbp[pos];
    return StackObject{obj.is_const, obj.is_constexpr, obj.symbol, obj.type->clone()};
}

std::optional<Operand> CompilerStack::find_symbol(const std::string& symbol) {
    for (StackObject* stk_id = sbp; stk_id < sbp + sp; stk_id++) {
        if (stk_id->symbol == symbol) {
            return stk_id - sbp;
        }
    }

    return std::nullopt;
}

std::optional<Operand> CompilerStack::find_symbol(const StackObject& member) {
    return find_symbol(member.symbol);
}

VIA_NAMESPACE_END
