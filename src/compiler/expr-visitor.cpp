// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "state.h"
#include "bit-utility.h"
#include "stack.h"
#include "string-utility.h"
#include "compiler-types.h"
#include "visitor.h"

// ==========================================================================================
// visitor_expr.cpp
//
// This file is a part of the first compiler stage (0), and is used to compile expressions.
// It defines the `ExpressionVisitor::visit` function overloads.
//
// All visitor functions have common parameters: (NodeType& node, Operand dst).
// - NodeType& node: AST node object.
// - Operand dst: The destination register in which the expression lives until externally free'd.
//  - Note: This also implies that `dst` is not owned.
//
// Visitor functions compile each type of expression node by first converting it into
// corresponding opcode(s), and then determining the operands via the built-in node parameters.
//
// - LiteralNode compilation:
//  This node only emits `LOAD` opcodes, and is considered a constant expression.
//  It first checks for primitive data types within the node, and emits corresponding bytecode based
//  on that. If it finds complex data types like strings, tables, etc., it loads them into the
//  constant table and emits a `LOADK` instruction with the corresponding constant id.
//
// - SymbolNode compilation:
//  This node represents a "symbol" that is either a local, global, argument or upvalue.
//  It first checks the stack for the symbol, if found, emits a `GETSTACK` instruction with the
//  stack id of the symbol. After that, it checks for upvalues, if found emits `GETUPVALUE`. Next,
//  it checks for arguments by traversing the parameters of the top function in
//  `program::test_stack::function_stack` and looking for the symbol. If found, emits
//  `GETARGUMENT`. Finally, looks for the variable in the global scope by querying
//  `program::globals` and if found emits GETGLOBAL. If all of these queries fail, throws a
//  "Use of undeclared variable" compilation error.
//
// - UnaryNode compilation:
//  This node emits a NEG instruction onto the inner expression.
//
// - GroupNode compilation:
//  Compiles the inner expression into dst.
//
// - CallNode compilation:
//  This node represents a function call expression, which first loads the arguments onto the stack
//  (LIFO), loads the callee object, and calls it. And finally, emits a POP instruction to retrieve
//  the return value.
//
// - IndexNode compilation:
//  This node represents a member access, which could follow either of these patterns:
//    -> Direct table member access: table.index
//      This pattern compiles into a GETTABLE instruction that uses the hashed version of the index.
//    -> Expressional table access: table[index]
//      This pattern first compiles the index expression, then casts it into a string, and finally
//      uses it as a table index.
//    -> Object member access: object::index
//      This pattern is by far the most complex pattern with multiple arbitriary checks in order to
//      ensure access validity, conventional integrity, and type safety. It first checks if the
//      index value is a symbol or not to make the guarantee that aggregate types can only have
//      symbols as keys. After that, it searches for the field name inside the aggregate type, if
//      not found, throws a "Aggregate object has no field named ..." compilation error. And the
//      final check, which checks for private member access patterns (object::_index) which by
//      convention tell the compiler that members prefixed with '_' are private members of the
//      object. Upon failure, throws a "Private member access into object..." warning.
//
//
// ==================================================================================================
VIA_NAMESPACE_BEGIN

using enum OpCode;
using enum OutputSeverity;

void ExprVisitor::visit(LiteralNode& literal_node, Operand dst) {
    using enum ValueType;

    if (int* integer_value = std::get_if<int>(&literal_node.value)) {
        uint32_t final_value = *integer_value;
        auto     operands    = reinterpret_u32_as_2u16(final_value);

        program.bytecode->emit(LOADINT, {dst, operands.l, operands.r});
    }
    else if (float* float_value = std::get_if<float>(&literal_node.value)) {
        uint32_t final_value = std::bit_cast<uint32_t>(*float_value);
        auto     operands    = reinterpret_u32_as_2u16(final_value);

        program.bytecode->emit(LOADFLOAT, {dst, operands.l, operands.r});
    }
    else if (bool* bool_value = std::get_if<bool>(&literal_node.value)) {
        program.bytecode->emit(*bool_value ? LOADTRUE : LOADFALSE, {dst});
    }
    else {
        const TValue& constant    = construct_constant(literal_node);
        const Operand constant_id = program.constants->push_constant(constant);

        program.bytecode->emit(LOADK, {dst, constant_id});
    }
}

void ExprVisitor::visit(SymbolNode& variable_node, Operand dst) {
    Token var_id = variable_node.identifier;

    std::string            symbol = var_id.lexeme;
    std::optional<Operand> stk_id = program.test_stack->find_symbol(var_id.lexeme);

    if (stk_id.has_value()) {
        auto& current_closure = program.test_stack->function_stack.top();

        if (current_closure.upvalues > stk_id.value()) {
            program.bytecode->emit(GETUPVALUE, {dst, stk_id.value()}, symbol);
        }
        else {
            program.bytecode->emit(GETSTACK, {dst, stk_id.value()}, symbol);
        }
    }
    else if (program.globals->was_declared(symbol)) {
        uint32_t symbol_hash = hash_string_custom(symbol.c_str());
        auto     operands    = reinterpret_u32_as_2u16(symbol_hash);

        program.bytecode->emit(GETGLOBAL, {dst, operands.l, operands.r}, symbol);
    }
    else if (!program.test_stack->function_stack.empty()) {
        uint16_t index = 0;
        auto&    top   = program.test_stack->function_stack.top();

        for (const auto& parameter : top.parameters) {
            if (parameter.identifier.lexeme == symbol) {
                program.bytecode->emit(GETARGUMENT, {dst, index});
                return;
            }

            ++index;
        }
    }
    else {
        compiler_error(var_id, std::format("Use of undeclared variable '{}'", var_id.lexeme));
    }
}

void ExprVisitor::visit(UnaryNode& unary_node, Operand dst) {
    unary_node.accept(*this, dst);
    program.bytecode->emit(NEG, {dst});
}

void ExprVisitor::visit(GroupNode& group_node, Operand dst) {
    group_node.accept(*this, dst);
}

void ExprVisitor::visit(CallNode& call_node, Operand dst) {
    Operand argc       = call_node.arguments.size();
    Operand callee_reg = allocator.allocate_register();

    call_node.callee->accept(*this, callee_reg);

    for (const pExprNode& argument : call_node.arguments) {
        Operand argument_reg = allocator.allocate_register();

        argument->accept(*this, argument_reg);
        program.bytecode->emit(PUSH, {argument_reg});
        allocator.free_register(argument_reg);
    }

    program.bytecode->emit(CALL, {callee_reg, argc});
    program.bytecode->emit(POP, {dst});
    allocator.free_register(callee_reg);
}

void ExprVisitor::visit(IndexNode& index_node, Operand dst) {
    Operand obj_reg   = allocator.allocate_register();
    Operand index_reg = allocator.allocate_register();

    index_node.object->accept(*this, obj_reg);
    index_node.index->accept(*this, index_reg);

    pTypeNode object_type = index_node.object->infer_type(program);
    pTypeNode index_type  = index_node.index->infer_type(program);

    // Validate index type
    if (auto* primitive = get_derived_instance<TypeNode, PrimitiveNode>(*index_type)) {
        if (primitive->type != ValueType::string && primitive->type != ValueType::integer) {
            compiler_error(
                index_node.index->begin,
                index_node.index->end,
                "Index type must be a string or integer"
            );
            return;
        }
    }

    // Handle different object types
    if (auto* primitive = get_derived_instance<TypeNode, PrimitiveNode>(*object_type)) {
        switch (primitive->type) {
        case ValueType::string:
            program.bytecode->emit(GETSTRING, {dst, obj_reg, index_reg});
            break;
        case ValueType::table:
            program.bytecode->emit(GETTABLE, {dst, obj_reg, index_reg});
            break;
        default:
            compiler_error(
                index_node.object->begin,
                index_node.object->end,
                "Expression type is not subscriptable"
            );
        }
    }
}

void ExprVisitor::visit(BinaryNode& binary_node, Operand dst) {
    using enum TokenType;
    using OpCodeId = std::underlying_type_t<OpCode>;

    static const std::unordered_map<TokenType, OpCode> operator_map = {
        {TokenType::OP_ADD, OpCode::ADD},
        {TokenType::OP_SUB, OpCode::SUB},
        {TokenType::OP_MUL, OpCode::MUL},
        {TokenType::OP_DIV, OpCode::DIV},
        {TokenType::OP_EXP, OpCode::POW},
        {TokenType::OP_MOD, OpCode::MOD},
        {TokenType::OP_EQ, OpCode::EQUAL},
        {TokenType::OP_NEQ, OpCode::NOTEQUAL},
        {TokenType::OP_LT, OpCode::LESS},
        {TokenType::OP_GT, OpCode::GREATER},
        {TokenType::OP_LEQ, OpCode::LESSOREQUAL},
        {TokenType::OP_GEQ, OpCode::GREATEROREQUAL},
        {TokenType::KW_AND, OpCode::AND},
        {TokenType::KW_OR, OpCode::OR},
    };

    pExprNode& p_lhs = binary_node.lhs_expression;
    pExprNode& p_rhs = binary_node.rhs_expression;

    ExprNode& lhs = *p_lhs;
    ExprNode& rhs = *p_rhs;

    auto it = operator_map.find(binary_node.op.type);
    if (it == operator_map.end()) {
        compiler_error(
            binary_node.op, std::format("Unknown binary operator '{}'", binary_node.op.lexeme)
        );
        return;
    }

    pTypeNode left_type  = p_lhs->infer_type(program);
    pTypeNode right_type = p_rhs->infer_type(program);

    CHECK_TYPE_INFERENCE_FAILURE(left_type, binary_node.lhs_expression);
    CHECK_TYPE_INFERENCE_FAILURE(right_type, binary_node.rhs_expression);

    if (!is_compatible(left_type, right_type)) {
        compiler_error(
            binary_node.begin, binary_node.end, "Binary operation on incompatible types"
        );
        return;
    }

    const OpCode   base_opcode    = it->second;
    const OpCodeId base_opcode_id = static_cast<OpCodeId>(base_opcode);
    OpCodeId       opcode_id      = base_opcode_id;

    if (is_constant_expression(rhs)) {
        LiteralNode& literal = dynamic_cast<LiteralNode&>(rhs);

        if (base_opcode == DIV) {
            if (TInteger* int_val = std::get_if<TInteger>(&literal.value)) {
                if (*int_val == 0) {
                    goto division_by_zero;
                }
            }

            if (TFloat* float_val = std::get_if<TFloat>(&literal.value)) {
                if (*float_val == 0.0f) {
                    goto division_by_zero;
                }
            }

            goto good_division;

        division_by_zero:
            compiler_error(literal.value_token, "Explicit division by zero");
            return;

        good_division:
        }

        lhs.accept(*this, dst);

        if (base_opcode == AND || base_opcode == OR) {
            bool is_rhs_falsy = ({
                bool* rhs_bool = std::get_if<bool>(&literal.value);
                auto* rhs_nil  = std::get_if<std::monostate>(&literal.value);
                rhs_bool != nullptr ? !(*rhs_bool) : rhs_nil != nullptr;
            });

            if (base_opcode == AND && is_rhs_falsy) {
                program.bytecode->emit(LOADFALSE, {dst});
            }

            if (base_opcode == OR && !is_rhs_falsy) {
                program.bytecode->emit(LOADTRUE, {dst});
            }

            return;
        }

        if (int* int_value = std::get_if<int>(&literal.value)) {
            OpCode   opcode      = static_cast<OpCode>(opcode_id + 2); // OPINT
            uint32_t final_value = *int_value;
            auto     operands    = reinterpret_u32_as_2u16(final_value);

            program.bytecode->emit(opcode, {dst, operands.l, operands.r});
        }
        else if (float* float_value = std::get_if<float>(&literal.value)) {
            OpCode   opcode      = static_cast<OpCode>(opcode_id + 3); // OPFLOAT
            uint32_t final_value = std::bit_cast<uint32_t>(*float_value);
            auto     operands    = reinterpret_u32_as_2u16(final_value);

            program.bytecode->emit(opcode, {dst, operands.l, operands.r});
        }
        else {
            OpCode  opcode          = static_cast<OpCode>(opcode_id + 1); // OPK
            TValue  right_const_val = construct_constant(dynamic_cast<LiteralNode&>(rhs));
            Operand right_const_id  = program.constants->push_constant(right_const_val);

            program.bytecode->emit(opcode, {dst, right_const_id});
        }
    }
    else {
        Operand reg = allocator.allocate_register();

        if (rhs.precedence() > lhs.precedence()) {
            rhs.accept(*this, dst);
            lhs.accept(*this, reg);
        }
        else {
            lhs.accept(*this, dst);
            rhs.accept(*this, reg);
        }

        program.bytecode->emit(base_opcode, {dst, reg});
        allocator.free_register(reg);
    }
}

void ExprVisitor::visit(TypeCastNode& type_cast, Operand dst) {
    pTypeNode left_type = type_cast.expression->infer_type(program);

    CHECK_TYPE_INFERENCE_FAILURE(left_type, type_cast.expression);

    if (!is_castable(left_type, type_cast.type)) {
        compiler_error(
            type_cast.expression->begin,
            type_cast.expression->end,
            std::format("Cannot cast expression into type '{}'", type_cast.type->to_string_x())
        );
    }

    Operand temp = allocator.allocate_register();
    type_cast.expression->accept(*this, temp);

    if (PrimitiveNode* primitive = get_derived_instance<TypeNode, PrimitiveNode>(*type_cast.type)) {
        if (primitive->type == ValueType::integer) {
            program.bytecode->emit(TOINT, {dst, temp});
        }
        else if (primitive->type == ValueType::floating_point) {
            program.bytecode->emit(TOFLOAT, {dst, temp});
        }
        else if (primitive->type == ValueType::string) {
            program.bytecode->emit(TOSTRING, {dst, temp});
        }
    }

    allocator.free_register(temp);
}

VIA_NAMESPACE_END
