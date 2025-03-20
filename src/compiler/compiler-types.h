// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_TYPES_H
#define _VIA_TYPES_H

#include "ast.h"
#include "common.h"
#include "object.h"

VIA_NAMESPACE_BEGIN

template<typename T>
struct DataType;

template<>
struct DataType<std::monostate> {
    static constexpr ValueType value_type = ValueType::nil;
    static constexpr int       precedence = -1;
};

template<>
struct DataType<TInteger> {
    static constexpr ValueType value_type = ValueType::integer;
    static constexpr int       precedence = 1;
};

template<>
struct DataType<TFloat> {
    static constexpr ValueType value_type = ValueType::floating_point;
    static constexpr int       precedence = 2;
};

template<>
struct DataType<bool> {
    static constexpr ValueType value_type = ValueType::boolean;
    static constexpr int       precedence = -1;
};

template<>
struct DataType<std::string> {
    static constexpr ValueType value_type = ValueType::string;
    static constexpr int       precedence = -1;
};

template<typename Base, typename Derived>
bool is_derived_instance(Derived& derived) {
    return dynamic_cast<Base*>(&derived) != nullptr;
}

template<typename Base, typename Derived>
Derived* get_derived_instance(Base& base) {
    return dynamic_cast<Derived*>(&base);
}

template<typename Expression>
    requires std::is_base_of_v<ExprNode, Expression>
bool is_constant_expression(Expression& expression) {
    return is_derived_instance<LiteralNode>(expression);
}

template<typename Derived>
concept is_type_node_derivative = requires { std::is_base_of_v<TypeNode, Derived>; };

VIA_INLINE bool is_integral(pTypeNode& type) {
    using enum ValueType;

    if (PrimitiveNode* primitive = get_derived_instance<TypeNode, PrimitiveNode>(*type)) {
        return primitive->type == integer;
    }

    // TODO: Add aggregate type support by checking for arithmetic meta-methods
    return false;
}

VIA_INLINE bool is_floating_point(pTypeNode& type) {
    using enum ValueType;

    if (PrimitiveNode* primitive = get_derived_instance<TypeNode, PrimitiveNode>(*type)) {
        return primitive->type == floating_point;
    }

    // TODO: Add aggregate type support by checking for arithmetic meta-methods
    return false;
}

VIA_INLINE bool is_arithmetic(pTypeNode& type) {
    return is_integral(type) || is_floating_point(type);
}

VIA_INLINE bool is_compatible(pTypeNode& left, pTypeNode& right) {
    if (PrimitiveNode* primitive_left = get_derived_instance<TypeNode, PrimitiveNode>(*left)) {
        if (PrimitiveNode* primitive_right =
                get_derived_instance<TypeNode, PrimitiveNode>(*right)) {

            return primitive_left->type == primitive_right->type;
        }
    }

    return false;
}

VIA_INLINE bool is_castable(pTypeNode& from, pTypeNode& into) {
    if (PrimitiveNode* primitive_right = get_derived_instance<TypeNode, PrimitiveNode>(*into)) {
        if (get_derived_instance<TypeNode, PrimitiveNode>(*from)) {
            if (primitive_right->type == ValueType::string) {
                return true;
            }
            else if (is_arithmetic(into)) {
                return is_arithmetic(from);
            }
        }
    }

    return false;
}

VIA_INLINE bool is_castable(pTypeNode& from, ValueType to) {
    if (PrimitiveNode* primitive_left = get_derived_instance<TypeNode, PrimitiveNode>(*from)) {
        if (to == ValueType::string) {
            return true;
        }
        else if (to == ValueType::integer) {
            return primitive_left->type == ValueType::floating_point ||
                   primitive_left->type == ValueType::string;
        }
    }

    return false;
}

VIA_NAMESPACE_END

#endif
