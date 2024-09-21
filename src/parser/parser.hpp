#pragma once

#include <iostream>
#include <string>
#include <variant>
#include <vector>
#include <optional>
#include <cassert>

#include "../lexer/lexer.hpp"
#include "../utils/arena.hpp"
#include "../utils/error.hpp"

#include "ast/Literals.hpp"
#include "ast/ProgNode.hpp"
#include "ast/StmtNode.hpp"

const size_t M_ALLOC_SIZE = 8 * 1024 * 1024;

class Parser {
public:
    explicit Parser(std::vector<Token> toks, std::string name)
        : m_tokens(std::move(toks))
        , m_allocator(M_ALLOC_SIZE)
        , m_error_provider(Error("parser.hpp", &prog_node))
    {
        prog_node.prog_name = std::move(name);
    }

    template <typename _Ty>
    ExprNode* visit_lit(std::optional<Token> _value)
    {
        auto node = m_allocator.emplace<_Ty>();
        node->val = _value.value();
        return m_allocator.emplace<ExprNode>(node);
    }

    std::optional<ExprNode*> parse_term() 
    {
        if (auto int_lit = non_strict_consume(TokenType::INT_LIT))
            return visit_lit<IntLitNode>(int_lit);

        if (auto ident = non_strict_consume(TokenType::IDENTIFIER))
            return visit_lit<IdentNode>(ident);

        if (auto bool_alpha = non_strict_consume(TokenType::BOOL_ALPHA))
            return visit_lit<BoolLitNode>(bool_alpha);

        if (auto string_lit = non_strict_consume(TokenType::STRING_LIT))
            return visit_lit<StringLitNode>(string_lit);

        if (non_strict_consume(TokenType::L_PAR)) 
        {
            auto expr = parse_expr();

            if (!expr)
                parse_err("Expected expression");

            strict_consume(TokenType::R_PAR);

            auto term_paren = m_allocator.emplace<ParenExprNode>();
            term_paren->expr = expr.value();

            return m_allocator.emplace<ExprNode>(term_paren);
        }

        return std::nullopt;
    }

    std::optional<ExprNode*> parse_expr(int min_prec = 0) 
    {
        auto term_lhs = parse_term();

        if (!term_lhs) 
        {
            parse_err("bad lvalue in expression");
            return std::nullopt;
        }

        auto expr_lhs = term_lhs.value();

        while (true) 
        {
            auto current_tok = peek();

            if (!current_tok || bin_prec(current_tok.value().type) < min_prec) 
                break;

            const auto token = consume();
            const int next_min_prec = bin_prec(token.type).value() + 1;

            auto term_rhs = parse_expr(next_min_prec);

            if (!term_rhs) 
                parse_err("bad rvalue in expression");

            auto bin_expr = m_allocator.emplace<BinExprNode>();
            bin_expr->lhs = expr_lhs;
            bin_expr->rhs = term_rhs.value();
            bin_expr->op = token.type;

            expr_lhs = m_allocator.emplace<ExprNode>(bin_expr);
        }

        return expr_lhs;
    }

    std::optional<ScopeNode*> parse_scope()
    {
        if (!non_strict_consume(TokenType::L_CR_BRACKET)) return std::nullopt;

        auto scope = m_allocator.emplace<ScopeNode>();
        while (auto stmt = parse_stmt())
            scope->stmts.push_back(stmt.value());

        strict_consume(TokenType::R_CR_BRACKET);
        return scope;
    }

    std::optional<IfPredNode*> parse_if_pred()
    {
        auto if_pred = m_allocator.emplace<IfPredNode>();

        if (const auto expr = parse_expr())
            if_pred->cond = expr.value();
        else
            parse_err("Expected expression");

        strict_consume(TokenType::R_PAR);

        if (const auto scope = parse_scope())
            if_pred->then_scope = scope.value();
        else
            parse_err("Expected scope for if statement body");

        return if_pred;
    }

    std::optional<StmtNode*> parse_stmt() 
    {
        if (
            peek_check_type(0, TokenType::END) && 
            peek_check_type(1, TokenType::L_PAR))
        {
            consume();
            consume();

            auto stmt_exit = m_allocator.emplace<StmtExitNode>();

            if (const auto node_expr = parse_expr())
                stmt_exit->expr = node_expr.value();
            else
                parse_err("Expected expression");

            strict_consume(TokenType::R_PAR);

            return m_allocator.emplace<StmtNode>(stmt_exit);
        }

        if (
            peek_check_type(0, TokenType::IDENTIFIER) && 
            peek_check_type(1, TokenType::L_PAR))
        {
            auto func_call = m_allocator.emplace<FuncCallNode>();
            func_call->ident = strict_consume(TokenType::IDENTIFIER);

            strict_consume(TokenType::L_PAR);
            std::vector<ExprNode*> args;

            while (peek().has_value() && peek()->type != TokenType::R_PAR) 
            {
                if (auto arg = parse_term())
                    args.push_back(arg.value());

                else
                    parse_err("Unsupported argument type in function call");
                if (peek().has_value() && peek()->type == TokenType::COMMA)
                    consume();
            }

            strict_consume(TokenType::R_PAR);
            func_call->args = args;
            return m_allocator.emplace<StmtNode>(func_call);
        }

        if (peek_check_type_value(0, TokenType::KEYWORD, "local") &&
            peek_check_type(1, TokenType::IDENTIFIER) &&
            peek_check_type(2, TokenType::EQUALS)) 
        {
            consume();

            auto declr = m_allocator.emplace<LocalDeclNode>();
            declr->ident = consume();

            strict_consume(TokenType::EQUALS);
            if (peek_check_type_value(0, TokenType::KEYWORD, "function")) {
                consume();

                strict_consume(TokenType::L_PAR);
                // TODO: Parse function parameters
                strict_consume(TokenType::R_PAR);

                if (const auto func_scope = parse_scope()) 
                {
                    auto func_node = m_allocator.emplace<FuncNode>();
                    func_node->body = func_scope.value();
                    func_node->params = {}; // Temporary

                    declr->expr = m_allocator.emplace<ExprNode>(func_node);
                }
                else
                    parse_err("Failed to parse function scope");
            }
            else 
            {
                if (const auto expr = parse_expr())
                    declr->expr = expr.value();
                else
                    parse_err("Expected expression after '='");
            }

            return m_allocator.emplace<StmtNode>(declr);
        }

        if (peek_check_type(0, TokenType::IDENTIFIER) && peek_check_type(1, TokenType::DB_EQUALS)) 
        {
            const auto assign = m_allocator.emplace<StmtAssignNode>();
            assign->ident = consume();

            consume();

            if (const auto expr = parse_expr())
                assign->expr = expr.value();
            else
                m_error_provider.log(Error::Severity::Fatal, __LINE__, "Expected expression", peek()->line);

            return m_allocator.emplace<StmtNode>(assign);
        }

        if (peek_check_type_value(0, TokenType::KEYWORD, "do") &&
            peek_check_type(1, TokenType::L_CR_BRACKET))
        {
            strict_consume(TokenType::KEYWORD);

            if (const auto scope = parse_scope())
                return m_allocator.emplace<StmtNode>(scope.value());

            m_error_provider.log(Error::Severity::Fatal, __LINE__, "Expected scope", peek()->line);
        }

        if (peek_check_type_value(0, TokenType::KEYWORD, "if"))
        {
            auto stmt_if = m_allocator.emplace<IfStmtNode>();
            stmt_if->if_pred = m_allocator.emplace<IfPredNode>();

            if (const auto expr = parse_expr())
                stmt_if->if_pred->cond = expr.value();
            else
                parse_err("Expected expression");

            strict_consume(TokenType::R_PAR);
            if (const auto scope = parse_scope())
                stmt_if->if_pred->then_scope = scope.value();
            else
                m_error_provider.log(Error::Severity::Fatal, __LINE__, "Expected scope", peek()->line);

            return m_allocator.emplace<StmtNode>(stmt_if);
        }

        return {};
    }

    std::optional<ProgNode> parse_prog() 
    {
        while (peek().has_value()) 
        {
            if (peek()->type == TokenType::END) break;

            if (auto stmt = parse_stmt())
                prog_node.stmts.push_back(stmt.value());
            else
                m_error_provider.log(Error::Severity::Fatal, __LINE__, "Expected statement", peek()->line);
        }

        return prog_node;
    }

private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    ProgNode prog_node;
    Error m_error_provider;
    ArenaAllocator m_allocator;

    std::optional<Token> peek(int offset = 0) const 
    {
        if (m_index + offset >= m_tokens.size()) return std::nullopt;
        return m_tokens[m_index + offset];
    }

    bool peek_check_type(
        int ahead, 
        TokenType expected_type) const
    {
        return peek(ahead).has_value() && 
            peek(ahead)->type == expected_type;
    }

    bool peek_check_type_value(
        int ahead, 
        TokenType expected_type, 
        std::string expected_value) const
    {
        return peek(ahead).has_value() && 
            peek(ahead)->type == expected_type && 
            peek(ahead)->value == expected_value;
    }

    Token consume() 
    {
        if (m_index >= m_tokens.size())
            m_error_provider.log(Error::Severity::Fatal, __LINE__, "Token consumption overflow", -1);

        return m_tokens[m_index++];
    }

    Token strict_consume(TokenType expected_type) 
    {
        if (peek() && peek()->type == expected_type) return consume();
        
        m_error_provider.log(Error::Severity::Fatal, __LINE__,
            "Expected token of type " + TokenType_to_string(expected_type) +
            " got token of type " + TokenType_to_string(peek().value().type) + "\n", peek().value().line);

        unreachable();

        return {};
    }

    std::optional<Token> non_strict_consume(TokenType expected_type) 
    {
        if (peek() && peek()->type == expected_type) return consume();
        return std::nullopt;
    }

    void parse_err(const std::string& msg) 
    {
        m_error_provider.log(Error::Severity::Fatal, __LINE__, msg, peek().value().line);
    }

    void unreachable() 
    {
        assert(false);
    }
};