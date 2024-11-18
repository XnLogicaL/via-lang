/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "token.h"
#include <optional>
#include <variant>
#include <vector>

namespace via::Parsing::AST
{

using namespace Tokenization;

// Forward declarations
struct ScopeStmtNode;
struct TypedParamStmtNode;
struct ElifStmtNode;
struct CaseStmtNode;
struct DefaultStmtNode;

// Type nodes
// ---------------

struct LiteralTypeNode;
struct UnionTypeNode;
struct VariantTypeNode;
struct FunctorTypeNode;
struct TableTypeNode;
struct OptionTypeNode;

typedef std::variant<
    LiteralTypeNode,
    UnionTypeNode,
    VariantTypeNode,
    FunctorTypeNode,
    TableTypeNode,
    OptionTypeNode
> TypeNode;

struct LiteralTypeNode
{
    Token type;
    std::vector<TypeNode*> args;
};

struct UnionTypeNode
{
    TypeNode* lhs;
    TypeNode* rhs;
};

struct VariantTypeNode
{
    std::vector<TypeNode*> types;
};

struct FunctorTypeNode
{
    std::vector<TypeNode*> input;
    std::vector<TypeNode*> output;
};

struct TableTypeNode
{
    TypeNode* type;
};

struct OptionTypeNode
{
    TypeNode* type;
};

// Expression nodes
// ---------------

struct LitExprNode;
struct UnExprNode;
struct GroupExprNode;
struct BinExprNode;
struct LambdaExprNode;
struct CallExprNode;
struct IndexExprNode;
struct IndexCallExprNode;
struct TypeCastExprNode;
struct BracketIndexExprNode;
struct BracketIndexCallExprNode;
struct IdentExprNode;

typedef std::variant<
    LitExprNode,
    UnExprNode,
    GroupExprNode,
    BinExprNode,
    LambdaExprNode,
    CallExprNode,
    IndexExprNode,
    IndexCallExprNode,
    TypeCastExprNode,
    BracketIndexExprNode,
    BracketIndexCallExprNode,
    IdentExprNode
> ExprNode;

struct LitExprNode
{
    Token val;
};

struct IdentExprNode
{
    Token val;  
};

struct UnExprNode
{
    ExprNode* expr;
};

struct GroupExprNode
{
    ExprNode* expr;
};

struct BinExprNode
{
    Token op;
    ExprNode* lhs;
    ExprNode* rhs;
};

struct LambdaExprNode
{
    std::vector<TypedParamStmtNode*> params;
    ScopeStmtNode* body;
};

struct CallExprNode
{
    ExprNode* ident;
    std::vector<ExprNode*> args;
    std::vector<TypeNode*> type_args;
};

struct IndexExprNode
{
    ExprNode* ident;
    Token index;
};

struct IndexCallExprNode
{
    ExprNode* ident;
    Token index;
    std::vector<ExprNode*> args;
    std::vector<TypeNode*> type_args;
};

struct TypeCastExprNode
{
    TypeNode* type;
    ExprNode* expr;
};

struct BracketIndexExprNode
{
    ExprNode* ident;
    ExprNode* index;
};

struct BracketIndexCallExprNode
{
    ExprNode* ident;
    ExprNode* index;
    std::vector<ExprNode*> args;
    std::vector<TypeNode*> type_args;
};

// Statement nodes
// ---------------

struct LocalDeclStmtNode;
struct GlobDeclStmtNode;
struct PropertyDeclStmtNode;
struct CallStmtNode;
struct IndexCallStmtNode;
struct IndexAssignStmtNode;
struct AssignStmtNode;
struct WhileStmtNode;
struct ForStmtNode;
struct ScopeStmtNode;
struct FuncDeclStmtNode;
struct MethodDeclStmtNode;
struct IfStmtNode;
struct ElifStmtNode;
struct SwitchStmtNode;
struct CaseStmtNode;
struct DefaultStmtNode;
struct NamespaceDeclStmtNode;
struct StructDeclStmtNode;
struct ReturnStmtNode;
struct BracketIndexAssignStmtNode;
struct BracketIndexCallStmtNode;
struct ExprIndexCallStmtNode;
struct ExprIndexAssignStmtNode;
struct BracketExprIndexCallStmtNode;
struct BracketExprIndexAssignStmtNode;

typedef std::variant<
    LocalDeclStmtNode,
    GlobDeclStmtNode,
    PropertyDeclStmtNode,
    CallStmtNode,
    IndexCallStmtNode,
    IndexAssignStmtNode,
    AssignStmtNode,
    WhileStmtNode,
    ForStmtNode,
    ScopeStmtNode,
    FuncDeclStmtNode,
    MethodDeclStmtNode,
    IfStmtNode,
    ElifStmtNode,
    SwitchStmtNode,
    CaseStmtNode,
    DefaultStmtNode,
    NamespaceDeclStmtNode,
    StructDeclStmtNode,
    ReturnStmtNode,
    BracketIndexAssignStmtNode,
    BracketIndexCallStmtNode,
    IndexCallStmtNode,
    IndexAssignStmtNode
> StmtNode;

struct TypedParamStmtNode
{
    Token ident;
    TypeNode* type;
};

struct LocalDeclStmtNode
{
    Token ident;
    TypeNode* type;
    ExprNode* val;
    bool is_const;
};

struct GlobDeclStmtNode
{
    Token ident;
    TypeNode* type;
    ExprNode* val;
};

struct CallStmtNode
{
    Token ident;
    std::vector<ExprNode*> args;
    std::vector<TypeNode*> type_args;
};

struct ReturnStmtNode
{
    std::vector<ExprNode*> vals;
};

struct IndexCallStmtNode
{
    ExprNode* ident;
    Token index;
    std::vector<ExprNode*> args;
    std::vector<TypeNode*> type_args;
};

struct AssignStmtNode
{
    Token ident;
    ExprNode* val;
};

struct IndexAssignStmtNode
{
    ExprNode* ident;
    Token index;
    ExprNode* val;
};

struct BracketIndexAssignStmtNode
{
    ExprNode* ident;
    ExprNode* index;
    ExprNode* val;
};

struct BracketIndexCallStmtNode
{
    ExprNode* ident;
    ExprNode* index;
    std::vector<ExprNode*> args;
    std::vector<TypeNode*> type_args;
};

struct PropertyDeclStmtNode
{
    Token ident;
    TypeNode* type;
    std::optional<ExprNode*> val;
    bool is_const;
};

struct WhileStmtNode
{
    ExprNode* cond;
    ScopeStmtNode* scope;
};

struct ForStmtNode
{
    Token init;
    ExprNode* cond;
    ExprNode* step;
    ScopeStmtNode* body;
};

struct IfStmtNode
{
    ExprNode* cond;
    ScopeStmtNode* body;
    std::optional<ScopeStmtNode*> else_body;
    std::vector<ElifStmtNode*> elif_bodies;
};

struct ElifStmtNode
{
    ExprNode* cond;
    ScopeStmtNode* body;
};

struct SwitchStmtNode
{
    ExprNode* cond;
    std::vector<CaseStmtNode*> cases;
    std::optional<DefaultStmtNode*> default_case;
};

struct CaseStmtNode
{
    ExprNode* value;
    ScopeStmtNode* body;
};

struct DefaultStmtNode
{
    ScopeStmtNode* body;
};

struct FuncDeclStmtNode
{
    Token ident;
    std::vector<TypedParamStmtNode*> params;
    std::vector<Token> type_params;
    ScopeStmtNode* body;
    bool is_const;
};

struct MethodDeclStmtNode : FuncDeclStmtNode
{
};

struct NamespaceDeclStmtNode
{
    Token ident;
    std::vector<PropertyDeclStmtNode*> properties;
    std::vector<FuncDeclStmtNode*> funcs;
    std::vector<Token> template_types;
};

struct StructDeclStmtNode
{
    Token ident;
    std::vector<PropertyDeclStmtNode*> properties;
    std::vector<FuncDeclStmtNode*> funcs;
    std::vector<Token> template_types;
};

struct ScopeStmtNode
{
    std::vector<StmtNode*> stmts;
};

struct AST
{
    std::vector<StmtNode*> stmts;
};

} // namespace via::Parsing::AST
