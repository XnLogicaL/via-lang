#ifndef VIA_AST_H
#define VIA_AST_H

#include "common.h"
#include "token.h"

namespace via
{

namespace Parsing
{

namespace AST
{

using namespace Tokenization;

// Type nodes
// ---------------

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

typedef std::variant<
    LiteralTypeNode,
    UnionTypeNode,
    VariantTypeNode,
    FunctorTypeNode,
    TableTypeNode
> TypeNode;

// Expression nodes
// ---------------

struct LitExprNode
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
    Token ident;
    std::vector<ExprNode*> args;
    std::vector<TypeNode*> type_args;
};

struct IndexExprNode
{
    Token ident;
    Token index;
};

struct IndexCallExprNode
{
    Token ident;
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
    Token ident;
    ExprNode* index;
};

struct BracketIndexCallExprNode
{
    Token ident;
    ExprNode* index;
    std::vector<ExprNode*> args;
    std::vector<TypeNode*> type_args;
};

struct ExprIndexExprNode
{
    ExprNode* expr;
    Token index;  
};

struct ExprIndexCallExprNode
{
    ExprNode* expr;
    Token index;
    std::vector<ExprNode*> args;
    std::vector<TypeNode*> type_args;
};

struct BracketExprIndexExprNode
{
    ExprNode* expr;
    ExprNode* index;
};

struct BracketExprIndexCallExprNode
{
    ExprNode* expr;
    ExprNode* index;
    std::vector<ExprNode*> args;
    std::vector<TypeNode*> type_args;
};

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
    ExprIndexExprNode,
    ExprIndexCallExprNode,
    BracketExprIndexExprNode,
    BracketExprIndexCallExprNode
> ExprNode;

// Statement nodes
// ---------------

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
    Token ident;
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
    Token ident;
    Token index;
    ExprNode* val;
};

struct BracketIndexAssignStmtNode
{
    Token ident;
    ExprNode* index;
    ExprNode* val;
};

struct BracketIndexCallStmtNode
{
    Token ident;
    ExprNode* index;
    std::vector<ExprNode*> args;
    std::vector<TypeNode*> type_args;
};

struct ExprIndexCallStmtNode
{
    ExprNode* expr;
    Token index;
    std::vector<ExprNode*> args;
    std::vector<TypeNode*> type_args;
};

struct ExprIndexAssignStmtNode
{
    ExprNode* expr;
    Token index;
    ExprNode* val;
};

struct BracketExprIndexCallStmtNode
{
    ExprNode* expr;
    ExprNode* index;
    std::vector<ExprNode*> args;
    std::vector<TypeNode*> type_args;
};

struct BracketExprIndexAssignStmtNode
{
    ExprNode* expr;
    ExprNode* index;
    ExprNode* val;
};

struct PropertyDeclStmtNode
{
    Token ident;
    TypeNode* type;
    std::optional<ExprNode*> val;
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
    std::vector<TypedParamStmtNode> params;
    std::vector<Token> type_params;
    ScopeStmtNode* body;
    bool is_lambda;
};

struct MethodDeclStmtNode : public FuncDeclStmtNode
{
    MethodDeclStmtNode()
    {
        auto ty = LiteralTypeNode{
            .type = Token(TokenType::IDENTIFIER, "self"),
            .args = {}
        };

        // Add "self" as the first parameter
        params.insert(params.begin(), TypedParamStmtNode{
            .ident = Token(TokenType::IDENTIFIER, "self"),
            .type = new TypeNode(ty)
        });
    }
};

struct NamespaceDeclStmtNode
{
    Token ident;
    std::vector<PropertyDeclStmtNode*> properties;
    std::vector<FuncDeclStmtNode*> funcs;
    std::vector<TypeNode*> template_types;
};

struct StructDeclStmtNode
{
    Token ident;
    std::vector<PropertyDeclStmtNode*> properties;
    std::vector<FuncDeclStmtNode*> funcs;
    std::vector<TypeNode*> template_types;
};

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
    ExprIndexCallStmtNode,
    ExprIndexAssignStmtNode,
    BracketExprIndexCallStmtNode,
    BracketExprIndexAssignStmtNode
> StmtNode;

struct ScopeStmtNode
{
    std::vector<StmtNode*> stmts;
};

struct AST
{
    std::vector<StmtNode*> stmts;
};

} // namespace AST
    
} // namespace Parsing

} // namespace via

#endif // VIA_AST_H
