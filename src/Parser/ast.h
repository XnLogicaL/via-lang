/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "token.h"
#include <optional>
#include <variant>
#include <vector>
#include <memory>

namespace via::Parsing::AST
{

using namespace Tokenization;

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

using TypeNode = std::variant<GenericTypeNode, UnionTypeNode, VariantTypeNode, FunctionTypeNode, TableTypeNode, OptionalTypeNode>;

struct GenericTypeNode
{
    Token name;
    std::vector<TypeNode> generics;
};

struct UnionTypeNode
{
    TypeNode *lhs;
    TypeNode *rhs;
};

struct VariantTypeNode
{
    std::vector<TypeNode> types;
};

struct FunctionTypeNode
{
    std::vector<TypeNode> input;
    std::vector<TypeNode> output;
};

struct TableTypeNode
{
    TypeNode *ktype;
    TypeNode *vtype;
};

struct OptionalTypeNode
{
    TypeNode *type;
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

using ExprNode = std::variant<LiteralExprNode, UnaryExprNode, BinaryExprNode, LambdaExprNode, CallExprNode, IndexExprNode, VarExprNode>;

struct LiteralExprNode
{
    Token value;
};

struct UnaryExprNode
{
    ExprNode *expr;
};

struct BinaryExprNode
{
    Token op;
    ExprNode *lhs;
    ExprNode *rhs;
};

struct LambdaExprNode
{
    std::vector<TypedParamNode> params;
    ScopeStmtNode *body;
};

struct CallExprNode
{
    ExprNode *callee;
    std::vector<ExprNode> args;
    std::vector<TypeNode> type_args;
};

struct IndexExprNode
{
    ExprNode *object;
    ExprNode *index;
};

struct VarExprNode
{
    Token ident;
};

// StatementNode Variants and Definitions
// ----------------------------------------

struct LocalDeclStmtNode;
struct GlobalDeclStmtNode;
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

using StmtNode = std::variant<
    LocalDeclStmtNode,
    GlobalDeclStmtNode,
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
    BreakStmtNode>;

struct TypedParamNode
{
    Token ident;
    TypeNode type;
};

struct LocalDeclStmtNode
{
    Token ident;
    TypeNode type;
    std::optional<ExprNode> value;
    bool is_const;
};

struct GlobalDeclStmtNode
{
    Token ident;
    TypeNode type;
    std::optional<ExprNode> value;
};

struct CallStmtNode
{
    ExprNode *callee;
    std::vector<ExprNode> args;
    std::vector<TypeNode> generics;
};

struct AssignStmtNode
{
    ExprNode *target;
    ExprNode *value;
};

struct WhileStmtNode
{
    ExprNode *condition;
    ScopeStmtNode *body;
};

struct ForStmtNode
{
    Token keys;
    Token values;
    ExprNode *iterator;
    ScopeStmtNode *body;
};

struct ScopeStmtNode
{
    std::vector<StmtNode> statements;
};

struct FunctionDeclStmtNode
{
    Token ident;
    std::vector<TypedParamNode> params;
    std::vector<Token> generics;
    ScopeStmtNode *body;
    bool is_global;
};

struct IfStmtNode
{
    ExprNode *condition;
    ScopeStmtNode *then_body;
    std::optional<ScopeStmtNode> else_body;
    std::vector<ElifStmtNode> elif_branches;
};

struct ElifStmtNode
{
    ExprNode *condition;
    ScopeStmtNode *body;
};

struct SwitchStmtNode
{
    ExprNode *condition;
    std::vector<CaseStmtNode> cases;
    std::optional<ScopeStmtNode> default_case;
};

struct CaseStmtNode
{
    ExprNode *value;
    ScopeStmtNode *body;
};

struct ReturnStmtNode
{
    std::vector<ExprNode> values;
};

struct StructDeclStmtNode
{
    Token ident;
    std::vector<StmtNode> declarations;
};

struct NamespaceDeclStmtNode
{
    Token ident;
    std::vector<StmtNode> declarations;
};

struct BreakStmtNode
{
};

struct ContinueStmtNode
{
};

// Root AST Node
// ---------------
struct AST
{
    std::vector<StmtNode> statements;
};

} // namespace via::Parsing::AST
