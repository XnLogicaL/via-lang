// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_PARSER_H
#define _VIA_PARSER_H

#include "common.h"
#include "ast.h"
#include "ast_base.h"
#include "highlighter.h"
#include "token.h"

VIA_NAMESPACE_BEGIN

class ParserError : public std::exception {
public:
    ParserError(const std::string& error, u64 position)
        : message(error),
          position(position) {}

    const char* what() const throw() {
        return message.c_str();
    }

    u64 where() const noexcept {
        return position;
    }

private:
    std::string message;
    u64         position;
};

class Parser {
public:
    Parser(ProgramData& program)
        : program(program),
          emitter(program) {}

    bool parse() noexcept;

private:
    ProgramData& program;
    ErrorEmitter emitter;

    u64 position = 0;

private:
    Token current();
    Token peek(int ahead = 1);
    Token consume(u32 ahead = 1);

    Modifiers parse_modifiers();

    pTypeNode parse_generic();
    pTypeNode parse_type_primary();
    pTypeNode parse_type();

    pExprNode parse_primary();
    pExprNode parse_postfix(pExprNode);
    pExprNode parse_binary(int);
    pExprNode parse_expr();

    pStmtNode parse_declaration();
    pStmtNode parse_scope();
    pStmtNode parse_if();
    pStmtNode parse_return();
    pStmtNode parse_while();
    pStmtNode parse_stmt();
};

VIA_NAMESPACE_END

#endif
