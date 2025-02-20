// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "arena.h"
#include "token.h"
#include "common.h"
#include "format_vec.h"
#include "types.h"

namespace via {

struct ASTNode {
    virtual ~ASTNode() = default;
    virtual std::string to_string()
    {
        return "ASTNode";
    };
};

struct ExprNode : public ASTNode {};
struct StmtNode : public ASTNode {};
struct TypeNode : public ASTNode {};

struct LiteralNode : public ExprNode {
    ValueType type;
    union value {
        int integer;
        bool boolean;
        double fraction;
        std::string string;
    };

    std::string get_value_string()
    {
        using enum ValueType;

        switch (type) {
        case nil:
            return "nil";
        case 
        }
    }

    std::string to_string() override
    {
        return std::format("Literal<'{}'>", get_value_string());
    }
};

struct VariableNode :
        public
            ExprNode
            {
                std::string identifier;
                std::string to_string() override
                {
                    return std::format("Variable<'{}'>", identifier);
                }
            };

            struct UnaryNode : public ExprNode {
                std::unique_ptr<ExprNode> expression;
                std::string to_string() override
                {
                    return std::format("Unary<{}>", expression->to_string());
                }
            };

            struct BinaryNode : public ExprNode {
                Token op;
                std::unique_ptr<ExprNode> lhs_expression;
                std::unique_ptr<ExprNode> rhs_expression;

                std::string to_string() override
                {
                    return std::format(
                        "Binary<{} '{}' {}>",
                        lhs_expression->to_string(),
                        op.lexeme,
                        rhs_expression->to_string()
                    );
                }
            };

            struct AbstractSyntaxTree {
                std::vector<StmtNode> statements;
            };

        } // namespace via
