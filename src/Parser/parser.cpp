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

Modifiers Parser::parse_modifiers()
{
    Modifiers modifiers;

    while (true) {
        Token cur = current();
        if (cur.type == KW_CONST) {
            if (modifiers.is_const) {
                emitter.out(cur.position, "Modifier 'const' encountered twice", Warning);
            }

            modifiers.is_const = true;
        }
        else {
            break;
        }
    }

    return modifiers;
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
    using ParameterNode = FunctionNode::ParameterNode;

    Token declaration_keyword = consume();
    TokenType declaration_type = declaration_keyword.type;
    bool is_global = declaration_type == KW_GLOBAL;

    if (declaration_type == KW_FUNC || current().type == KW_FUNC) {
        if (declaration_type != KW_FUNC) {
            consume();
        }

        Token identifier = expect_consume(IDENTIFIER);
        expect_consume(PAREN_OPEN);

        std::vector<ParameterNode> parameters;
        if (current().type != PAREN_CLOSE) {
            while (true) {
                parameters.emplace_back(expect_consume(IDENTIFIER));

                if (current().type != COMMA) {
                    break;
                }

                consume();
            }
        }

        expect_consume(PAREN_CLOSE);

        Modifiers modifiers = parse_modifiers();
        pStmtNode body_scope = parse_scope();

        return std::make_unique<FunctionNode>(
            is_global, modifiers, identifier, std::move(body_scope), parameters
        );
    }

    Modifiers modifiers{declaration_type == KW_CONST};

    if (declaration_type == KW_CONST || current().type == KW_CONST) {
        if (declaration_type != KW_CONST) {
            consume();
        }

        modifiers.is_const = true;
    }

    Token identifier = expect_consume(IDENTIFIER);
    expect_consume(EQUAL);
    pExprNode value = parse_expr();

    return std::make_unique<DeclarationNode>(is_global, modifiers, identifier, std::move(value));
}

pStmtNode Parser::parse_scope()
{
    expect_consume(BRACE_OPEN);

    std::vector<pStmtNode> scope_statements;
    while (current().type != BRACE_CLOSE) {
        pStmtNode stmt = parse_stmt();
        scope_statements.emplace_back(std::move(stmt));
    }

    expect_consume(BRACE_CLOSE);

    return std::make_unique<ScopeNode>(std::move(scope_statements));
}

pStmtNode Parser::parse_assign()
{
    Token identifier = expect_consume(IDENTIFIER);
    Token augmentation_operator;

    if (current().type != EQUAL && current().is_operator()) {
        augmentation_operator = consume();
    }

    expect_consume(EQUAL);

    pExprNode value = parse_expr();
    return std::make_unique<AssignNode>(identifier, augmentation_operator, std::move(value));
}

pStmtNode Parser::parse_if()
{
    using ElseIfNode = IfNode::ElseIfNode;

    expect_consume(KW_IF);

    pExprNode condition = parse_expr();
    pStmtNode scope = parse_scope();

    std::optional<pStmtNode> else_scope;
    std::vector<ElseIfNode> elseif_nodes;

    while (current().type == KW_ELIF) {
        consume();

        pExprNode elseif_condition = parse_expr();
        pStmtNode elseif_scope = parse_scope();

        ElseIfNode elseif(std::move(elseif_condition), std::move(elseif_scope));
        elseif_nodes.emplace_back(std::move(elseif));
    }

    if (current().type == KW_ELSE) {
        consume();

        pStmtNode else_scope_inner = parse_scope();
        else_scope.emplace(std::move(else_scope_inner));
    }
    else {
        else_scope = std::nullopt;
    }

    return std::make_unique<IfNode>(
        std::move(condition), std::move(scope), std::move(else_scope), std::move(elseif_nodes)
    );
}

pStmtNode Parser::parse_stmt()
{
    Token cur = current();
    if (cur.type == KW_LOCAL || cur.type == KW_GLOBAL || cur.type == KW_FUNC ||
        cur.type == KW_CONST) {
        return parse_declaration();
    }
    else if (cur.type == KW_DO) {
        consume();
        return parse_scope();
    }
    else {
        try {
            pExprNode expression = parse_expr();
            return std::make_unique<ExprStmtNode>(std::move(expression));
        }
        catch (const ParserError &) {
            // I thought that rethrowing exceptions was a fucking meme,
            // but here I am.
            throw ParserError(
                std::format("Unexpected token '{}' while parsing statement", cur.lexeme)
            );
        }

        VIA_ASSERT(false, "wait... how did you get this assertion to fail? you're a wizard!")
        return nullptr; // Yes, this is redundant, shut up
    }
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
