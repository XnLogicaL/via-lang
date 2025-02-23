// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#include "parser.h"

namespace via {

using enum OutputSeverity;
using enum TokenType;

Token Parser::current()
{
    return peek(0);
}

Token Parser::peek(int ahead)
{
    if (position + ahead >= program->tokens->tokens.size()) {
        throw ParserError("Unexpected end of file");
    }

    return program->tokens->tokens[position + ahead];
}

Token Parser::consume(int ahead)
{
    if (position + ahead >= program->tokens->tokens.size()) {
        throw ParserError("Unexpected end of file");
    }

    position += ahead;
    return peek(position - ahead);
}

Token Parser::expect_consume(TokenType type, int ahead)
{
    Token tok = consume(ahead);
    if (tok.type != type) {
        throw ParserError(std::format(
            "Expected token {}, got {}",
            magic_enum::enum_name(type),
            magic_enum::enum_name(tok.type)
        ));
    }

    return tok;
}

pExprNode Parser::parse_prim_expr()
{
    Token token = consume();

    switch (token.type) {
    case LIT_INT:
    case LIT_HEX:
        return std::make_unique<LiteralNode>(token, std::stoi(token.lexeme));
    case LIT_FLOAT:
        return std::make_unique<LiteralNode>(token, std::stof(token.lexeme));
    case LIT_BINARY:
        return std::make_unique<LiteralNode>(
            token, static_cast<int>(std::bitset<32>(token.lexeme.substr(2)).to_ulong())
        );
    case LIT_NIL:
        return std::make_unique<LiteralNode>(token, std::monostate());
    case LIT_BOOL:
        return std::make_unique<LiteralNode>(token, token.lexeme == "true");
    case IDENTIFIER:
        return std::make_unique<VariableNode>(token);
    case LIT_STRING:
        return std::make_unique<LiteralNode>(token, token.lexeme);
    case OP_SUB: {
        Token op = token;
        pExprNode expr = parse_prim_expr();
        return std::make_unique<UnaryNode>(std::move(expr));
    }
    case PAREN_OPEN: {
        consume();
        pExprNode expression = parse_expr();
        expect_consume(PAREN_CLOSE);
        return expression;
    }
    default:
        throw ParserError(
            std::format("Unexpected token '{}' while parsing primary expression", token.lexeme)
        );
    }

    VIA_UNREACHABLE;
    return nullptr;
}

pExprNode Parser::parse_postfix_expr(pExprNode lhs)
{
    while (true) {
        switch (current().type) {
        case DOT: { // Member access: obj.property
            consume();
            Token index_token = expect_consume(IDENTIFIER);

            lhs = std::make_unique<IndexNode>(
                std::move(lhs), std::make_unique<VariableNode>(index_token)
            );

            break;
        }
        case BRACKET_OPEN: { // Array indexing: obj[expr]
            consume();
            pExprNode index = parse_expr();

            expect_consume(BRACKET_CLOSE);
            lhs = std::make_unique<IndexNode>(std::move(lhs), std::move(index));

            break;
        }
        case PAREN_OPEN: { // Function calls: func(arg1, arg2)
            consume();
            std::vector<pExprNode> arguments;
            arguments.reserve(4);

            while (current().type != PAREN_CLOSE) {
                arguments.emplace_back(parse_expr());
                if (current().type == COMMA) {
                    consume();
                }
                else {
                    break;
                }
            }

            expect_consume(PAREN_CLOSE);
            lhs = std::make_unique<CallNode>(std::move(lhs), std::move(arguments));
            break;
        }
        default:
            return lhs;
        }
    }
}

pExprNode Parser::parse_bin_expr(int precedence)
{
    pExprNode lhs =
        parse_postfix_expr(parse_prim_expr()); // Ensure postfix expressions are parsed immediately

    while (position < program->tokens->tokens.size() && current().is_operator()) {
        Token op = current();
        int op_prec = op.bin_prec();
        if (op_prec < precedence) {
            break;
        }

        consume();
        lhs = std::make_unique<BinaryNode>(op, std::move(lhs), parse_bin_expr(op_prec + 1));
    }

    return lhs;
}

pExprNode Parser::parse_expr()
{
    return parse_bin_expr(0);
}

pStmtNode Parser::parse_declaration()
{
    Token declaration_keyword = consume();
    TokenType declaration_type = declaration_keyword.type;

    bool is_global = declaration_type == KW_GLOBAL;
    Modifiers modifiers{declaration_type == KW_CONST};

    if (current().type == KW_CONST) {
        modifiers.is_const = true;
        consume();
    }

    Token identifier = expect_consume(IDENTIFIER);
    expect_consume(OP_ASGN);

    pExprNode value = parse_expr();
    return std::make_unique<DeclarationNode>(is_global, modifiers, identifier, std::move(value));
}

bool Parser::parse_program() noexcept
{
    bool failed = false;
    while (!failed) {
        try {
            pStmtNode stmt = parse_stmt();
            program->ast->statements.emplace_back(std::move(stmt));
        }
        catch (const ParserError &e) {
            failed = true;
            emitter.out(position, e.what(), Error);
            break;
        }
    }

    return failed;
}

} // namespace via
