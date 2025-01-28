/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "arena.hpp"
#include "token.h"
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>
#include "format_vec.h"

namespace via
{

enum class DeclarationType
{
    Local,
    Global,
    Property,
    Meta
};

struct Pragma
{
    Token body;
    std::vector<Token> arguments;

    std::string to_string()
    {
        return std::format(
            "Pragma(body: {}, arguments: {})", body.to_string(), format_vector<Token>(arguments, [](const Token tok) { return tok.to_string(); })
        );
    }
};

struct StatementModifiers
{
    bool is_const;
    bool is_strict;

    std::string to_string()
    {
        return std::format("[is_const: {}, is_strict: {}]", is_const, is_strict);
    }
};

// Forward declarations
struct ScopeStmtNode;
struct TypedParamNode;
struct ElifStmtNode;
struct CaseStmtNode;
struct DefaultStmtNode;

// TypeNode Variants and Definitions
// -----------------------------------

struct GenericTypeNode;
struct UnionTypeNode;
struct VariantTypeNode;
struct FunctionTypeNode;
struct TableTypeNode;
struct OptionalTypeNode;
struct TypeNode;

struct GenericTypeNode
{
    Token name;
    std::vector<TypeNode> generics;

    std::string to_string()
    {
        return "GenericTypeNode";
    }
};

struct UnionTypeNode
{
    TypeNode *lhs;
    TypeNode *rhs;

    std::string to_string()
    {
        return "UnionTypeNode";
    }
};

struct VariantTypeNode
{
    std::vector<TypeNode> types;

    std::string to_string()
    {
        return "VariantTypeNode";
    }
};

struct FunctionTypeNode
{
    std::vector<TypeNode> input;
    std::vector<TypeNode> output;

    std::string to_string()
    {
        return "FunctionTypeNode";
    }
};

struct TableTypeNode
{
    TypeNode *ktype;
    TypeNode *vtype;

    std::string to_string()
    {
        return "TableTypeNode";
    }
};

struct OptionalTypeNode
{
    TypeNode *type;

    std::string to_string()
    {
        return "OptionalTypeNode";
    }
};

struct TypeNode
{
    std::variant<GenericTypeNode, UnionTypeNode, VariantTypeNode, FunctionTypeNode, TableTypeNode, OptionalTypeNode> type;
    std::string to_string()
    {
        return std::visit([](auto &&type_) { return type_.to_string(); }, type);
    }
};

// ExpressionNode Variants and Definitions
// ----------------------------------------

struct LiteralExprNode;
struct UnaryExprNode;
struct BinaryExprNode;
struct LambdaExprNode;
struct CallExprNode;
struct IndexExprNode;
struct VarExprNode;
struct IncExprNode;
struct DecExprNode;
struct TypeExprNode;
struct TypeofExprNode;
struct TableExprNode;
struct TypeCastExprNode;
struct ExprNode;

struct LiteralExprNode
{
    Token value;

    std::string to_string()
    {
        return std::format("LiteralExprNode(value: {})", value.to_string());
    }
};

struct UnaryExprNode
{
    ExprNode *expr;

    std::string to_string()
    {
        return "UnaryExprNode";
    }
};

struct BinaryExprNode
{
    Token op;
    ExprNode *lhs;
    ExprNode *rhs;

    std::string to_string()
    {
        return "BinaryExprNode";
    }
};

struct LambdaExprNode
{
    std::vector<TypedParamNode> params;
    ScopeStmtNode *body;

    std::string to_string()
    {
        return "LambdaExprNode";
    }
};

struct CallExprNode
{
    ExprNode *callee;
    std::vector<ExprNode *> args;
    std::vector<TypeNode *> type_args;

    std::string to_string()
    {
        return "CallExprNode";
    }
};

struct IndexExprNode
{
    ExprNode *object;
    ExprNode *index;

    std::string to_string()
    {
        return "IndexExprNode";
    }
};

struct VarExprNode
{
    Token ident;

    std::string to_string()
    {
        return "VarExprNode";
    }
};

struct IncExprNode
{
    ExprNode *expr;

    std::string to_string()
    {
        return "IncExprNode";
    }
};

struct DecExprNode
{
    ExprNode *expr;

    std::string to_string()
    {
        return "DecExprNode";
    }
};
// type(expr)
struct TypeExprNode
{
    ExprNode *expr;

    std::string to_string()
    {
        return "TypeExprNode";
    }
};

// typeof(expr)
struct TypeofExprNode
{
    ExprNode *expr;

    std::string to_string()
    {
        return "IndexExprNode";
    }
};

// {KVPair...}
struct TableExprNode
{
    struct KVPair
    {
        ExprNode *key;
        ExprNode *val;
    };

    std::vector<KVPair> values;
    std::string to_string()
    {
        return "TableExprNode";
    }
};

struct TypeCastExprNode
{
    ExprNode *expr;
    TypeNode *type;

    std::string to_string()
    {
        return "TypeCastExprNode";
    }
};

struct ExprNode
{
    std::variant<
        LiteralExprNode,
        UnaryExprNode,
        BinaryExprNode,
        LambdaExprNode,
        CallExprNode,
        IndexExprNode,
        VarExprNode,
        IncExprNode,
        DecExprNode,
        TypeExprNode,
        TypeofExprNode,
        TypeCastExprNode,
        TableExprNode>
        expr;

    std::string to_string()
    {
        return std::visit([](auto &&expr_) { return expr_.to_string(); }, expr);
    }
};

// StatementNode Variants and Definitions
// ----------------------------------------

struct VariableDeclStmtNode;
struct CallStmtNode;
struct AssignStmtNode;
struct WhileStmtNode;
struct ForStmtNode;
struct ScopeStmtNode;
struct FunctionDeclStmtNode;
struct IfStmtNode;
struct SwitchStmtNode;
struct ReturnStmtNode;
struct StructDeclStmtNode;
struct NamespaceDeclStmtNode;
struct ContinueStmtNode;
struct BreakStmtNode;
struct StmtNode;

struct TypedParamNode
{
    Token ident;
    TypeNode type;
    StatementModifiers modifiers;

    std::string to_string()
    {
        return std::format("TypedParamNode( ident: {}, type: {}, modifiers: {})", ident.to_string(), "<type>", modifiers.to_string());
    }
};

struct VariableDeclStmtNode
{
    Token ident;
    TypeNode type;
    std::optional<ExprNode> value;
    StatementModifiers modifiers;
    DeclarationType decl_type;

    std::string to_string()
    {
        return std::format(
            "VariableDeclStmtNode( ident: {}, type: {}, value: {}, modifiers: {})",
            ident.to_string(),
            type.to_string(),
            value.has_value() ? value->to_string() : "Nil",
            modifiers.to_string()
        );
    }
};

struct CallStmtNode
{
    ExprNode *callee;
    std::vector<ExprNode *> args;
    std::vector<TypeNode *> generics;

    std::string to_string()
    {
        return std::format(
            "CallStmtNode( callee: {}, args: {}, generics: {})",
            callee->to_string(),
            format_vector<ExprNode *>(args, [](ExprNode *expr) { return expr->to_string(); }),
            format_vector<TypeNode *>(generics, [](TypeNode *type) { return type->to_string(); })
        );
    }
};

struct AssignStmtNode
{
    ExprNode *target;
    ExprNode *value;

    std::string to_string()
    {
        return std::format("AssignStmtNode(target: {}, value: {})", target->to_string(), value->to_string());
    }
};

struct WhileStmtNode
{
    ExprNode *condition;
    ScopeStmtNode *body;

    std::string to_string()
    {
        return std::format("WhileStmtNode(condition: {})", condition->to_string());
    }
};

struct ScopeStmtNode
{
    std::vector<StmtNode> statements;
    std::string to_string()
    {
        return "ScopeStmtNode";
    }
};

struct ForStmtNode
{
    Token keys;
    Token values;
    ExprNode *iterator;
    ScopeStmtNode *body;

    std::string to_string()
    {
        return std::format(
            "ForStmtNode(keys: {}, values: {}, iterator: {}, body: {})",
            keys.to_string(),
            values.to_string(),
            iterator->to_string(),
            body->to_string()
        );
    }
};

struct FunctionDeclStmtNode
{
    Token ident;
    std::vector<TypedParamNode> params;
    std::vector<Token> generics;
    std::optional<TypeNode> return_type;
    ScopeStmtNode *body;
    StatementModifiers modifiers;
    DeclarationType decl_type;

    std::string to_string()
    {
        return std::format(
            "FunctionDeclStmtNodeident: {}, params: {}, generics: {}, return_type: {}, body: {}, modifiers: {})",
            ident.to_string(),
            format_vector<TypedParamNode>(params, [](TypedParamNode param) { return " " + param.to_string(); }),
            format_vector<Token>(generics, [](Token tok) { return " " + tok.to_string(); }),
            return_type.has_value() ? return_type->to_string() : "Unknown",
            body->to_string(),
            modifiers.to_string()
        );
    }
};

struct ElifStmtNode
{
    ExprNode *condition;
    ScopeStmtNode *body;

    std::string to_string()
    {
        return std::format("ElifStmtNode(condition: {}, body: {})", condition->to_string(), body->to_string());
    }
};

struct IfStmtNode
{
    ExprNode *condition;
    ScopeStmtNode *then_body;
    std::optional<ScopeStmtNode> else_body;
    std::vector<ElifStmtNode> elif_branches;

    std::string to_string()
    {
        return std::format(
            "IfStmtNode(condition: {}, body: {}, else_body: {}, elseif_bodies: {})",
            condition->to_string(),
            then_body->to_string(),
            else_body.has_value() ? else_body->to_string() : "None",
            format_vector<ElifStmtNode>(elif_branches, [](ElifStmtNode elif) { return elif.to_string(); })
        );
    }
};

struct CaseStmtNode
{
    ExprNode *value;
    ScopeStmtNode *body;

    std::string to_string()
    {
        return std::format("CaseStmtNode(value: {}, body: {})", value->to_string(), body->to_string());
    }
};

struct SwitchStmtNode
{
    ExprNode *condition;
    std::vector<CaseStmtNode> cases;
    std::optional<ScopeStmtNode> default_case;

    std::string to_string()
    {
        return std::format(
            "SwitchStmtNode(condition: {}, cases: {}, default: {})",
            condition->to_string(),
            format_vector<CaseStmtNode>(cases, [](CaseStmtNode case_stmt) { return case_stmt.to_string(); }),
            default_case.has_value() ? default_case->to_string() : "None"
        );
    }
};

struct ReturnStmtNode
{
    std::vector<ExprNode> values;

    std::string to_string()
    {
        return std::format("ReturnStmtNode( values: {})", format_vector<ExprNode>(values, [](ExprNode expr) { return expr.to_string(); }));
    }
};

struct StructDeclStmtNode
{
    Token ident;
    DeclarationType decl_type;
    std::vector<StmtNode> declarations;

    std::string to_string()
    {
        return std::format("StructDeclStmtNode(ident: {})", ident.value);
    }
};

struct NamespaceDeclStmtNode
{
    Token ident;
    DeclarationType decl_type;
    std::vector<StmtNode> declarations;

    std::string to_string()
    {
        return std::format("NamespaceDeclStmt(ident: {})", ident.value);
    }
};

struct BreakStmtNode
{
    std::string to_string()
    {
        return "BreakStmtNode";
    }
};

struct ContinueStmtNode
{
    std::string to_string()
    {
        return "ContinueStmtNode";
    }
};

struct StmtNode
{
    std::variant<
        VariableDeclStmtNode,
        CallStmtNode,
        AssignStmtNode,
        WhileStmtNode,
        ForStmtNode,
        ScopeStmtNode,
        FunctionDeclStmtNode,
        IfStmtNode,
        SwitchStmtNode,
        ReturnStmtNode,
        StructDeclStmtNode,
        NamespaceDeclStmtNode,
        ContinueStmtNode,
        BreakStmtNode>
        stmt;

    std::string to_string()
    {
        return std::visit([](auto &&stmt_) { return stmt_.to_string(); }, stmt);
    }
};

// Root AST Node
// ---------------
struct AbstractSyntaxTree
{
    ArenaAllocator allocator;
    std::vector<StmtNode> statements;

    AbstractSyntaxTree(const size_t alloc_size)
        : allocator(alloc_size)
    {
    }

    std::string to_string()
    {
        std::ostringstream oss;

        for (StmtNode stmt : statements)
            oss << stmt.to_string() << '\n';

        return oss.str();
    }
};

} // namespace via
