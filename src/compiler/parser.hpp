#pragma once

#include <iostream>
#include <string>
#include <variant>
#include <vector>
#include <optional>
#include <cassert>

#include "lexer.hpp"

#include "../include/arena_alloc.hpp"

const size_t M_ALLOC_SIZE = 8 * 1024 * 1024;

// Forward node declarations
struct StmtNode;
struct ExprNode;
struct IfPredNode;

struct IntLitNode
{
    Token int_lit;
};

struct IdentNode
{
    Token ident;
};

struct TermParenNode
{
    ExprNode *expr;
};

struct AddBinExprNode
{
    ExprNode *lhs;
    ExprNode *rhs;
};

struct SubBinExprNode
{
    ExprNode *lhs;
    ExprNode *rhs;
};

struct MulBinExprNode
{
    ExprNode *lhs;
    ExprNode *rhs;
};

struct DivBinExprNode
{
    ExprNode *lhs;
    ExprNode *rhs;
};

struct ModBinExprNode
{
    ExprNode *lhs;
    ExprNode *rhs;
};

struct BinExprNode
{
    std::variant<AddBinExprNode *, SubBinExprNode *, MulBinExprNode *, DivBinExprNode *> node;
};

struct TermExprNode
{
    ExprNode *expr;
};

struct TermNode
{
    std::variant<TermParenNode*, TermExprNode*, IntLitNode*, IdentNode*> node;
};

struct ExprNode
{
    std::variant<TermNode *, BinExprNode *> node;
};

struct StmtExitNode
{
    ExprNode *node;
};

struct LocalDeclrNode
{
    Token ident;
    ExprNode *expr{};
    bool is_const;
};

struct GlobalDeclrNode
{
    Token ident;
    ExprNode *expr{};
    bool is_const;
};

struct ScopeNode
{
    std::vector<StmtNode *> scope;
};

struct IfPredElifNode
{
    ExprNode *expr{};
    ScopeNode *scope{};
    std::optional<IfPredNode *> pred;
};

struct IfPredElseNode
{
    ScopeNode *scope;
};

struct IfPredNode
{
    std::variant<IfPredElifNode *, IfPredElseNode *> var;
};

struct NodeStmtIf
{
    ExprNode *expr{};
    ScopeNode *scope{};
    std::optional<IfPredNode *> pred;
};

struct FuncCallNode
{
    Token ident;
    std::vector<IdentNode *> args;
};

struct StmtAssignNode
{
    Token ident;
    ExprNode *expr{};
};

struct StmtNode
{
    std::variant<
        StmtExitNode *, 
        LocalDeclrNode *, 
        GlobalDeclrNode *, 
        ScopeNode *, 
        NodeStmtIf *, 
        StmtAssignNode *, 
        FuncCallNode *> stmt;
};

struct ProgNode
{
    std::vector<StmtNode *> prog_scope;
    std::string prog_name;
};

class Parser
{
public:
    explicit Parser(std::vector<Token> toks)
        : m_tokens(std::move(toks)), m_allocator(M_ALLOC_SIZE) {}

    std::optional<TermNode *> parse_term()
    {
        if (auto int_lit = non_strict_consume(TokenType::INT_LIT))
        {
            auto term_int_lit = m_allocator.emplace<IntLitNode>(int_lit.value());
            auto term = m_allocator.emplace<TermNode>(term_int_lit);
            return term;
        }

        if (auto ident = non_strict_consume(TokenType::IDENTIFIER))
        {
            auto term_ident = m_allocator.emplace<IdentNode>(ident.value());
            auto term = m_allocator.emplace<TermNode>(term_ident);
            return term;
        }

        if (auto parent = non_strict_consume(TokenType::L_PAR))
        {
            auto expr = parse_expr();

            if (!expr.has_value())
                parse_err("Expected expression");

            strict_consume(TokenType::R_PAR);

            auto term_paren = m_allocator.emplace<TermParenNode>(expr.value());
            auto term = m_allocator.emplace<TermNode>(term_paren);

            return term;
        }

        return std::nullopt;
    }

    std::optional<ExprNode *> parse_expr(const int min_prec = 0)
    {
        auto term_lhs = parse_term();

        if (!term_lhs.has_value())
        {
            parse_err("bad lvalue in expression\n");
            return std::nullopt;
        }

        auto expr_lhs = m_allocator.emplace<ExprNode>(term_lhs.value());

        while (1)
        {
            auto current_tok = peek();
            std::optional<int> prec;

            if (current_tok.has_value())
            {
                prec = bin_prec(current_tok.value().type);

                if (!prec.has_value() || prec < min_prec)
                    break;
            }
            else
                break;

            const auto token = consume();
            const auto type = token.type;

            const int next_min_prec = prec.value() + 1;
            auto rhs_expr = parse_expr(next_min_prec);

            if (!rhs_expr)
                parse_err("bad rvalue in expression");

            auto bin_expr = m_allocator.emplace<BinExprNode>();

            if (type == TokenType::PLUS)
            {
                auto add = m_allocator.emplace<AddBinExprNode>();
                add->lhs = expr_lhs;
                add->rhs = rhs_expr.value();
                bin_expr->node = add;
            }
            else if (type == TokenType::MINUS)
            {
                auto sub = m_allocator.emplace<SubBinExprNode>();
                sub->lhs = expr_lhs;
                sub->rhs = rhs_expr.value();
                bin_expr->node = sub;
            }
            else if (type == TokenType::ASTERISK)
            {
                auto mul = m_allocator.emplace<MulBinExprNode>();
                mul->lhs = expr_lhs;
                mul->rhs = rhs_expr.value();
                bin_expr->node = mul;
            }
            else if (type == TokenType::F_SLASH)
            {
                auto div = m_allocator.emplace<DivBinExprNode>();
                div->lhs = expr_lhs;
                div->rhs = rhs_expr.value();
                bin_expr->node = div;
            }
            else unreachable(); // Unhandled binary operator

            expr_lhs = m_allocator.emplace<ExprNode>(bin_expr);
        }

        return expr_lhs;
    }

    std::optional<ScopeNode *> parse_scope() // NOLINT(*-no-recursion)
    {
        if (!non_strict_consume(TokenType::L_CR_BRACKET).has_value())
            return std::nullopt;

        auto scope = m_allocator.emplace<ScopeNode>();

        while (auto stmt = parse_stmt())
            scope->scope.push_back(stmt.value());

        strict_consume(TokenType::R_CR_BRACKET);

        return scope;
    }

    std::optional<IfPredNode *> parse_if_pred() // NOLINT(*-no-recursion)
    {
        if (peek().value().value == "if" && non_strict_consume(TokenType::KEYWORD))
        {
            strict_consume(TokenType::L_PAR);

            const auto elif = m_allocator.alloc<IfPredElifNode>();

            if (const auto expr = parse_expr())
                elif->expr = expr.value();
            else
                parse_err("Expected expression");

            strict_consume(TokenType::R_PAR);

            if (const auto scope = parse_scope())
                elif->scope = scope.value();
            else
                parse_err("Expected scope for if statement body");

            elif->pred = parse_if_pred();

            return m_allocator.emplace<IfPredNode>(elif);
        }

        if (peek().value().value == "else" && non_strict_consume(TokenType::KEYWORD))
        {
            auto else_ = m_allocator.alloc<IfPredElseNode>();

            if (const auto scope = parse_scope())
                else_->scope = scope.value();
            else
                parse_err("Expected scope for else statement body");

            return m_allocator.emplace<IfPredNode>(else_);
        }

        return {};
    }

    std::optional<StmtNode *> parse_stmt() // NOLINT(*-no-recursion)
    {
        if (peek().has_value() 
            && peek().value().type == TokenType::END 
            && peek(1).has_value() 
            && peek(1).value().type == TokenType::L_PAR)
        {
            consume();
            consume();

            auto stmt_exit = m_allocator.emplace<StmtExitNode>();

            if (const auto node_expr = parse_expr())
                stmt_exit->node = node_expr.value();
            else
                parse_err("Expected expression");

            strict_consume(TokenType::R_PAR);

            auto stmt = m_allocator.emplace<StmtNode>();
            stmt->stmt = stmt_exit;

            return stmt;
        }

        if (peek().has_value()
            && peek().value().type == TokenType::IDENTIFIER
            && peek(1).has_value() && peek(1).value().type == TokenType::L_PAR
        )
        {

            auto func_call = m_allocator.emplace<FuncCallNode>();
            func_call->ident = strict_consume(TokenType::IDENTIFIER);

            strict_consume(TokenType::L_PAR);

            bool expecting_ident = true;
            bool expecting_comma = false;

            std::vector<IdentNode*> args{};

            while (true)
            {
                auto current_tok = peek();
                auto token = current_tok.value();

                if (!current_tok.has_value())
                    unreachable();

                if (current_tok.value().type == TokenType::END)
                    parse_err("Expected ')' to close argument list, got <eof>");
                else if (current_tok.value().type == TokenType::R_PAR)
                    break;

                if (expecting_comma)
                    strict_consume(TokenType::COMMA);
                else if (expecting_ident)
                {
                    std::string prefix = "";

                    if (peek().value().type == TokenType::DOUBLE_QUOTE)
                        prefix = "'";

                    non_strict_consume(TokenType::DOUBLE_QUOTE);

                    auto arg = m_allocator.emplace<IdentNode>();
                    arg->ident = consume();
                    arg->ident.value = prefix + arg->ident.value + prefix;

                    args.push_back(arg);

                    non_strict_consume(TokenType::DOUBLE_QUOTE);
                }
                else unreachable();

                expecting_ident = !expecting_ident;
                expecting_comma = !expecting_comma;
            }

            strict_consume(TokenType::R_PAR);

            func_call->args = args;

            auto stmt = m_allocator.emplace<StmtNode>();
            stmt->stmt = func_call;

            return stmt;
        }

        if (
            peek().has_value() && (peek().value().type == TokenType::KEYWORD && peek().value().value == "local") 
            && peek(1).has_value() && peek(1).value().type == TokenType::IDENTIFIER 
            && peek(2).has_value() && peek(2).value().type == TokenType::EQUALS)
        {
            consume();

            auto declr = m_allocator.emplace<LocalDeclrNode>();
            declr->is_const = false;

            if (peek().value().type == TokenType::EXCLAMATION)
            {
                declr->is_const = true;
                consume();
            }

            declr->ident = consume();

            consume();

            if (const auto expr = parse_expr())
                declr->expr = expr.value();
            else
                parse_err("Expected expression");

            auto stmt = m_allocator.emplace<StmtNode>();
            stmt->stmt = declr;

            return stmt;
        }

        if (
            peek().has_value() && peek().value().type == TokenType::IDENTIFIER 
            && peek(1).has_value() && peek(1).value().type == TokenType::DB_EQUALS)
        {
            const auto assign = m_allocator.alloc<StmtAssignNode>();
            assign->ident = consume();

            consume();

            if (const auto expr = parse_expr())
                assign->expr = expr.value();
            else
                parse_err("Expected expression");

            return m_allocator.emplace<StmtNode>(assign);
        }

        if (peek().has_value() && peek().value().type == TokenType::L_CR_BRACKET)
        {
            if (auto scope = parse_scope())
                return m_allocator.emplace<StmtNode>(scope.value());

            parse_err("Expected scope");
        }

        if (auto if_ = non_strict_consume(TokenType::KEYWORD) && peek().value().value == "if")
        {
            strict_consume(TokenType::L_PAR);

            auto stmt_if = m_allocator.emplace<NodeStmtIf>();

            if (const auto expr = parse_expr())
                stmt_if->expr = expr.value();
            else
                parse_err("Expected expression");

            strict_consume(TokenType::R_PAR);

            if (const auto scope = parse_scope())
                stmt_if->scope = scope.value();
            else
                parse_err("Expected scope");

            stmt_if->pred = parse_if_pred();
            return m_allocator.emplace<StmtNode>(stmt_if);
        }

        return {};
    }

    std::optional<ProgNode> parse_prog()
    {
        ProgNode prog;

        while (peek().has_value())
        {
            if (peek().value().type == TokenType::END)
                break; 

            if (auto stmt = parse_stmt())
                prog.prog_scope.push_back(stmt.value());
            else
                parse_err("Expected statement");
        }

        return prog;
    }

private:
    ArenaAllocator m_allocator;
    const std::vector<Token> m_tokens;
    size_t m_index = 0;

    bool parse_complete = false;
    bool parse_success = true;

    void unreachable()
    {
        assert(false);
    }

    void parse_err(std::string msg)
    {
        std::cerr << "[ERROR] [Parser]: " << msg << std::endl;
        std::cerr << "  thrown at line " 
            << std::to_string(peek().value().line) << ", column " 
            << std::to_string(peek().value().column) << std::endl;
        exit(1);
    }

    void parse_warn(std::string msg) const
    {
        std::cout << "[WARNING] [Parser]: " << msg << std::endl;
    }

    std::optional<Token> peek(const int offset = 0) const
    {
        if (m_index + offset >= m_tokens.size())
            return std::nullopt;

        return m_tokens.at(m_index + offset);
    }

    Token consume()
    {
        if (m_index >= m_tokens.size())
        {
            parse_err("Attempted to consume a token when no more tokens are available");
        }

        return m_tokens.at(m_index++);
    }

    Token strict_consume(TokenType expected_type)
    {
        if (peek().has_value() && (peek().value().type == expected_type))
            return consume();
        else
        {
            parse_err("Expected token of type " 
                + token_to_string(expected_type) + " got token of type " 
                + token_to_string(peek().value().type) + "\n  at line " 
                + std::to_string(peek().value().line) + ", column " 
                + std::to_string(peek().value().column) + "\n"
            );
        }

        unreachable();

        return {};
    }

    std::optional<Token> non_strict_consume(TokenType expected_type)
    {
        if (peek().has_value() && (peek().value().type == expected_type))
            return consume();
        else
            return std::nullopt;

        unreachable();
    }
};