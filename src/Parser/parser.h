#ifndef VIA_PARSER_H
#define VIA_PARSER_H

#include "common.h"
#include "token.h"
#include "ast.h"
#include "arena.hpp"

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

    Token consume();
    Token peek(size_t ahead = 0);

    bool check_value(std::string expected = "", size_t ahead = 0);
    bool check_type(TokenType expected = TokenType::UNKNOWN, size_t ahead = 0);

    std::vector<AST::ExprNode*> get_call_args();
    std::vector<AST::TypeNode*> get_call_type_args();

    AST::TypeNode* parse_type();

    AST::ExprNode* parse_expr();
    AST::BinExprNode* parse_bin_expr();

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
        : toks(toks)
        , m_alloc(ArenaAllocator(8192)) {}

    AST::AST* parse_prog();
};

AST::AST* parse(std::vector<Token> toks);

} // namespace Parsing
    
} // namespace via

#endif // VIA_PARSER_H
