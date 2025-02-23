// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "ast.h"
#include "ast_base.h"
#include "highlighter.h"
#include "token.h"

namespace via {

class ParserError : public std::exception {
public:
    ParserError(const std::string &error)
        : message(error)
    {
    }

    const char *what() const override
    {
        return message.c_str();
    }

private:
    std::string message;
};

class Parser {
public:
    Parser(ProgramData *program)
        : program(program)
        , emitter(program)
    {
    }

    bool parse_program() noexcept;

private:
    ProgramData *program;
    Emitter emitter;

    U64 position = 0;

private:
    Token current();
    Token peek(int ahead = 1);
    Token consume(int ahead = 1);
    Token expect_consume(TokenType type = TokenType::UNKNOWN, int ahead = 1);

    Modifiers parse_modifiers();

    pExprNode parse_prim_expr();
    pExprNode parse_postfix_expr(pExprNode);
    pExprNode parse_bin_expr(int prec);
    pExprNode parse_expr();

    pStmtNode parse_declaration();
    pStmtNode parse_assign();
    pStmtNode parse_scope();
    pStmtNode parse_if();
    pStmtNode parse_stmt();
};

} // namespace via
