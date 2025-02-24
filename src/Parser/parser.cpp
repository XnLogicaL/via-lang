// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "parser.h"

#define EXPECT(expected, message) \
    do { \
        if (current().type != expected) { \
            throw ParserError(std::format(message, current().lexeme), position); \
        } \
    } while (0)

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
        return Token();
    }

    return program->tokens->tokens[position + ahead];
}

Token Parser::consume(U32 ahead)
{
    U64 new_pos = position + static_cast<U64>(ahead);
    if (new_pos >= program->tokens->tokens.size()) {
        throw ParserError(
            std::format("Unexpected end of file (attempted read of: token #{})", new_pos), position
        );
    }

    position = new_pos;
    return peek(-static_cast<I32>(ahead));
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

pExprNode Parser::parse_primary()
{
    Token token = consume();

    try {
        switch (token.type) {
        case LIT_INT:
        case LIT_HEX: {
            int value = std::stoi(token.lexeme);
            return std::make_unique<LiteralNode>(token, value);
        }
        case LIT_FLOAT: {
            float value = std::stof(token.lexeme);
            return std::make_unique<LiteralNode>(token, value);
        }
        case LIT_BINARY:
            return std::make_unique<LiteralNode>(
                token, static_cast<int>(std::bitset<64>(token.lexeme.substr(2)).to_ullong())
            );
        case LIT_NIL:
            return std::make_unique<LiteralNode>(token, std::monostate());
        case LIT_BOOL:
            return std::make_unique<LiteralNode>(token, token.lexeme == "true");
        case IDENTIFIER:
            return std::make_unique<SymbolNode>(token);
        case LIT_STRING:
            return std::make_unique<LiteralNode>(token, token.lexeme);
        case OP_SUB: {
            Token op = token;
            pExprNode expr = parse_primary();
            return std::make_unique<UnaryNode>(std::move(expr));
        }
        case PAREN_OPEN: {
            consume();
            pExprNode expression = parse_expr();
            EXPECT(PAREN_CLOSE, "Expected ')' to close grouping expression, got '{}'");
            return expression;
        }
        default:
            throw ParserError(
                std::format("Unexpected token '{}' while parsing primary expression", token.lexeme),
                token.position
            );
        }
    }
    catch (const std::invalid_argument &) {
        throw ParserError(std::format("Malformed numeric format"), token.position);
    }
    catch (const std::out_of_range &) {
        throw ParserError(std::format("Numeric value out of range"), token.position);
    }

    VIA_UNREACHABLE;
    return nullptr;
}

pExprNode Parser::parse_postfix(pExprNode lhs)
{
    while (true) {
        switch (current().type) {
        case DOT: { // Member access: obj.property
            consume();
            EXPECT(IDENTIFIER, "Expected identifier while parsing index, got '{}'");
            Token index_token = consume();

            lhs = std::make_unique<IndexNode>(
                std::move(lhs), std::make_unique<SymbolNode>(index_token)
            );

            break;
        }
        case BRACKET_OPEN: { // Array indexing: obj[expr]
            consume();
            pExprNode index = parse_expr();

            EXPECT(BRACKET_CLOSE, "Expected ']' to close index expression, got '{}'");
            consume();

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

            EXPECT(PAREN_CLOSE, "Expected ')' to close function call arguments, got '{}'");
            consume();

            lhs = std::make_unique<CallNode>(std::move(lhs), std::move(arguments));
            break;
        }
        default:
            return lhs;
        }
    }
}

pExprNode Parser::parse_binary(int precedence)
{
    pExprNode lhs = parse_postfix(parse_primary());

    while (position < program->tokens->tokens.size() && current().is_operator()) {
        Token op = current();
        int op_prec = op.bin_prec();
        if (op_prec < precedence) {
            break;
        }

        consume();
        lhs = std::make_unique<BinaryNode>(op, std::move(lhs), parse_binary(op_prec + 1));
    }

    return lhs;
}

pExprNode Parser::parse_expr()
{
    return parse_binary(0);
}

pStmtNode Parser::parse_declaration()
{
    using ParameterNode = FunctionNode::ParameterNode;

    Token declaration_keyword = consume();
    TokenType declaration_type = declaration_keyword.type;

    bool is_local = declaration_type == KW_LOCAL;
    bool is_global = declaration_type == KW_GLOBAL;
    bool is_const = declaration_type == KW_CONST;

    if (is_local && current().type == KW_CONST) {
        is_const = true;
        consume();
    }

    if (current().type == KW_FUNC) {
        consume();

        EXPECT(IDENTIFIER, "Expected identifier for function declaration, got '{}'");
        Token identifier = consume();

        EXPECT(PAREN_OPEN, "Expected '(' to open function parameters, got '{}'");
        consume();

        std::vector<ParameterNode> parameters;
        if (current().type != PAREN_CLOSE) {
            while (true) {
                EXPECT(IDENTIFIER, "Expected identifier for function parameter, got '{}'");
                parameters.emplace_back(consume());

                if (current().type != COMMA) {
                    break;
                }

                consume(); // Consume ','
            }
        }

        EXPECT(PAREN_CLOSE, "Expected ')' to close function parameters, got '{}'");
        consume();

        Modifiers modifiers{is_const};
        pStmtNode body_scope = parse_scope();

        return std::make_unique<FunctionNode>(
            is_global, modifiers, identifier, std::move(body_scope), parameters
        );
    }

    EXPECT(IDENTIFIER, "Expected identifier for variable declaration, got '{}'");
    Token identifier = consume();

    EXPECT(EQUAL, "Expected '=' for variable declaration, got '{}'");
    consume();
    pExprNode value = parse_expr();

    return std::make_unique<DeclarationNode>(
        is_global, Modifiers{is_const}, identifier, std::move(value)
    );
}

pStmtNode Parser::parse_scope()
{
    EXPECT(BRACE_OPEN, "Expected '{{' to open scope, got '{}'");
    consume();

    std::vector<pStmtNode> scope_statements;
    while (current().type != BRACE_CLOSE) {
        pStmtNode stmt = parse_stmt();
        scope_statements.emplace_back(std::move(stmt));
    }

    EXPECT(BRACE_CLOSE, "Expected '}}' to close scope, got '{}'");
    consume();

    return std::make_unique<ScopeNode>(std::move(scope_statements));
}

pStmtNode Parser::parse_assign()
{
    EXPECT(IDENTIFIER, "Expected identifier for assignment, got '{}'");
    Token identifier = consume();
    Token augmentation_operator;

    if (current().type != EQUAL && current().is_operator()) {
        augmentation_operator = consume();
    }

    EXPECT(EQUAL, "Expected '=' for assignment, got '{}'");
    consume();

    pExprNode value = parse_expr();
    return std::make_unique<AssignNode>(identifier, augmentation_operator, std::move(value));
}

pStmtNode Parser::parse_if()
{
    using ElseIfNode = IfNode::ElseIfNode;

    consume();

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

pStmtNode Parser::parse_while()
{
    consume();

    pExprNode condition = parse_expr();
    pStmtNode body = parse_scope();

    return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

pStmtNode Parser::parse_stmt()
{
    Token initial_token = current();
    switch (initial_token.type) {
    case KW_LOCAL:
    case KW_GLOBAL:
    case KW_FUNC:
    case KW_CONST:
        return parse_declaration();
    case KW_DO:
        consume();
        return parse_scope();
    case KW_IF:
        return parse_if();
    case KW_WHILE:
        return parse_while();
    default:
        try {
            pExprNode expression = parse_expr();
            return std::make_unique<ExprStmtNode>(std::move(expression));
        }
        catch (const ParserError &) {
            // I thought that rethrowing exceptions was a fucking meme,
            // but here I am.
            throw ParserError(
                std::format("Unexpected token '{}' while parsing statement", initial_token.lexeme),
                initial_token.position
            );
        }

        VIA_ASSERT(false, "wait... how did you get this assertion to fail? you're a wizard!")
    }

    VIA_UNREACHABLE;
    return nullptr; // Yes, this is redundant, shut up
}

bool Parser::parse_program() noexcept
{
    bool failed = false;
    while (!failed) {
        if (current().type == EOF_) {
            break;
        }

        try {
            pStmtNode stmt = parse_stmt();
            program->ast->statements.emplace_back(std::move(stmt));
        }
        catch (const ParserError &e) {
            failed = true;
            emitter.out(e.where(), e.what(), Error);
            break;
        }
        catch (const std::exception &e) {
            failed = true;
            emitter.out_flat(e.what(), Error);
            break;
        }
    }

    return failed;
}

} // namespace via
