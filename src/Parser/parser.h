#ifndef VIA_PARSER_H
#define VIA_PARSER_H

#include "common.h"
#include "ast.h"
#include "arena.hpp"

#include "../Lexer/token.h"

#ifndef __VIA_PARSER_ALLOC_SIZE
    // 8MiB
    #define __VIA_PARSER_ALLOC_SIZE 8 * 1024 * 1024
#endif

namespace via
{

namespace Parsing
{

using namespace via::Tokenization;

class Parser
{
    ArenaAllocator m_alloc;

    std::vector<Token> toks;
    size_t pos;

    template <typename T> AST::TypeNode* cast_type_ptr(T* v);
    template <typename T> AST::ExprNode* cast_expr_ptr(T* v);
    template <typename T> AST::StmtNode* cast_stmt_ptr(T* v);

    inline Token consume();
    inline Token peek(size_t ahead = 0);

    bool check_value(std::string expected = "", size_t ahead = 0);
    bool check_type(TokenType expected = TokenType::UNKNOWN, size_t ahead = 0);

    std::vector<AST::ExprNode*> get_call_args();
    std::vector<AST::TypeNode*> get_call_type_args();

    AST::TypeNode* parse_type();

    AST::ExprNode* parse_expr();
    AST::ExprNode* parse_bin_expr(int precedence = 0);
    AST::ExprNode* parse_ident_expr(const Token& current);
    AST::ExprNode* parse_primary_expr();

    AST::TypedParamStmtNode* parse_param();
    AST::LocalDeclStmtNode* parse_local_decl_stmt();
    AST::GlobDeclStmtNode* parse_glob_decl_stmt();
    AST::CallStmtNode* parse_call_stmt();
    AST::ReturnStmtNode* parse_ret_stmt();
    AST::IndexCallStmtNode* parse_index_call_stmt();
    AST::AssignStmtNode* parse_assign_stmt();
    AST::IndexAssignStmtNode* parse_index_assign_stmt();
    AST::PropertyDeclStmtNode* parse_property_decl_stmt();
    AST::WhileStmtNode* parse_while_stmt();
    AST::ForStmtNode* parse_for_stmt();
    AST::IfStmtNode* parse_if_stmt();
    AST::SwitchStmtNode* parse_switch_stmt();
    AST::FuncDeclStmtNode* parse_func_decl_stmt();
    AST::MethodDeclStmtNode* parse_method_decl_stmt();
    AST::NamespaceDeclStmtNode* parse_namesp_decl_stmt();
    AST::StructDeclStmtNode* parse_struct_decl_stmt();
    AST::ScopeStmtNode* parse_scope_stmt();
    AST::StmtNode* parse_stmt();

public:

    Parser(std::vector<Token> toks)
        : m_alloc(ArenaAllocator(__VIA_PARSER_ALLOC_SIZE))
        , toks(toks)
        , pos(0) {}

    AST::AST* parse_prog();
};

} // namespace Parsing
    
} // namespace via

#endif // VIA_PARSER_H
