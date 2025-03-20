// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "parser.h"
#include "compiler-types.h"

#define CHECK_RESULT(result)                                                                       \
    if (!result.has_value()) {                                                                     \
        return std::unexpected(result.error());                                                    \
    }

VIA_NAMESPACE_BEGIN

using enum OutputSeverity;
using enum TokenType;

template<typename T>
using result = Parser::result<T>;

result<Token> Parser::current() {
    return peek(0);
}

result<Token> Parser::peek(int32_t ahead) {
    if (position + ahead >= program.token_stream->size()) {
        return std::unexpected<ParserError>({
            position,
            std::format("Unexpected end of file (attempted read of: token #{})", position + ahead),
        });
    }

    return program.token_stream->at(position + ahead);
}

result<Token> Parser::consume(uint32_t ahead) {
    uint64_t new_pos = position + static_cast<uint64_t>(ahead);
    if (new_pos >= program.token_stream->size()) {
        return std::unexpected<ParserError>({
            position,
            std::format("Unexpected end of file (attempted read of: token #{})", new_pos),
        });
    }

    position = new_pos;
    return peek(-static_cast<int32_t>(ahead));
}

result<Modifiers> Parser::parse_modifiers() {
    Modifiers modifiers;

    while (true) {
        result<Token> curr = current();
        CHECK_RESULT(curr);

        if (curr->type == KW_CONST) {
            if (modifiers.is_const) {
                emitter.out(*curr, "Modifier 'const' encountered multiple times", Warning);
            }

            modifiers.is_const = true;
            consume();
        }
        else {
            break;
        }
    }

    return modifiers;
}

result<pTypeNode> Parser::parse_generic() {
    using Generics = GenericNode::Generics;

    result<Token>     identifier = consume();
    result<Modifiers> modifiers  = parse_modifiers();

    CHECK_RESULT(identifier);
    CHECK_RESULT(modifiers);

    Generics generics;

    result<Token> expect_lt = expect_consume(OP_LT, "Expected '<' to open type generic, got '{}'");
    result<Token> curr      = current();

    CHECK_RESULT(expect_lt);
    CHECK_RESULT(curr);

    while (curr->type != OP_GT) {
        result<pTypeNode> result_type  = parse_type();
        result<Token>     result_token = current();

        CHECK_RESULT(result_type);
        CHECK_RESULT(result_token);

        generics.emplace_back(std::move(result_type.value()));

        if (curr->type != OP_GT) {
            result<Token> expect_comma =
                expect_consume(COMMA, "Expected ',' to seperate type generics, got '{}'");

            CHECK_RESULT(expect_comma);
        }
    }

    result<Token> expect_gt = expect_consume(OP_GT, "Expected '>' to close type generic, got '{}'");

    CHECK_RESULT(expect_gt);

    return std::make_unique<GenericNode>(
        identifier.value(), std::move(generics), modifiers.value()
    );
}

result<pTypeNode> Parser::parse_type_primary() {
    static std::unordered_map<std::string, ValueType> primitive_map = {
        {"int", ValueType::integer},
        {"float", ValueType::floating_point},
        {"bool", ValueType::boolean},
        {"string", ValueType::string},
    };

    result<Token> tok = current();

    CHECK_RESULT(tok);

    switch (tok->type) {
    case IDENTIFIER:
    case LIT_NIL: {
        auto it = primitive_map.find(tok->lexeme);
        if (it != primitive_map.end()) {
            return std::make_unique<PrimitiveNode>(consume().value(), it->second);
        }

        return parse_generic();
    }
    case PAREN_OPEN: {
        using parameters = FunctionTypeNode::Parameters;
        parameters    params;
        result<Token> curr = consume();

        CHECK_RESULT(curr);

        while (curr->type != PAREN_CLOSE) {
            result<pTypeNode> type_result = parse_type();
            result<Token>     token       = current();

            CHECK_RESULT(type_result);
            CHECK_RESULT(token);

            params.emplace_back(std::move(type_result.value()));

            if (token->type != PAREN_CLOSE) {
                result<Token> expect_comma = expect_consume(
                    COMMA, "Expected ',' to seperate function parameter types, got '{}'"
                );

                CHECK_RESULT(expect_comma);
            }
        }

        result<Token> expect_pc =
            expect_consume(PAREN_CLOSE, "Expected ')' to close function type parameters, got '{}'");
        result<Token> expect_rt =
            expect_consume(RETURNS, "Expected '->' to specify function return type, got '{}'");
        result<pTypeNode> return_type = parse_type();

        CHECK_RESULT(expect_pc);
        CHECK_RESULT(expect_rt);
        CHECK_RESULT(return_type);

        return std::make_unique<FunctionTypeNode>(
            std::move(params), std::move(return_type.value())
        );
    }
    default: {
        break;
    }
    }

    return std::unexpected<ParserError>({
        position,
        std::format("Unexpected token '{}' while parsing type primary", tok->lexeme),
    });
}

result<pTypeNode> Parser::parse_type() {
    return parse_type_primary();
}

result<pExprNode> Parser::parse_primary() {
    result<Token> token = current();

    try {
        switch (token.type) {
        case LIT_INT:
        case LIT_HEX: {
            consume();
            int value = std::stoi(token.lexeme);
            return std::make_unique<LiteralNode>(token, value);
        }
        case LIT_FLOAT: {
            consume();
            float value = std::stof(token.lexeme);
            return std::make_unique<LiteralNode>(token, value);
        }
        case LIT_BINARY:
            consume();
            return std::make_unique<LiteralNode>(
                token, static_cast<int>(std::bitset<64>(token.lexeme.substr(2)).to_ullong())
            );
        case LIT_NIL:
            consume();
            return std::make_unique<LiteralNode>(token, std::monostate());
        case LIT_BOOL:
            consume();
            return std::make_unique<LiteralNode>(token, token.lexeme == "true");
        case IDENTIFIER:
            consume();
            return std::make_unique<SymbolNode>(token);
        case LIT_STRING:
            consume();
            return std::make_unique<LiteralNode>(token, token.lexeme);
        case OP_SUB: {
            consume();
            Token     op   = token;
            pExprNode expr = parse_primary();
            return std::make_unique<UnaryNode>(std::move(expr));
        }
        default:
            parser_error(
                std::format("Unexpected token '{}' while parsing primary expression", token.lexeme),
                position
            );
        }
    }
    catch (const std::invalid_argument& err) {
        parser_error(std::format("Malformed numeric format ({})", err.what()), position);
    }
    catch (const std::out_of_range& err) {
        parser_error(std::format("Numeric value out of range ({})", err.what()), position);
    }

    VIA_UNREACHABLE;
    return nullptr;
}

pExprNode Parser::parse_postfix(pExprNode lhs) {
    while (true) {
        switch (current().type) {
        case DOT: { // Member access: obj.property
            consume();
            expect(IDENTIFIER, "Expected identifier while parsing index, got '{}'");
            Token index_token = consume();

            lhs = std::make_unique<IndexNode>(
                std::move(lhs), std::make_unique<SymbolNode>(index_token)
            );

            break;
        }
        case BRACKET_OPEN: { // Array indexing: obj[expr]
            consume();
            pExprNode index = parse_expr();

            expect(BRACKET_CLOSE, "Expected ']' to close index expression, got '{}'");
            consume();

            lhs = std::make_unique<IndexNode>(std::move(lhs), std::move(index));
            break;
        }
        case COLON: { // Member access obj::id
            consume();

            expect(COLON, "Expected ':' to complete member access delimiter, got '{}'");
            consume();

            expect(IDENTIFIER, "Expected identifier for member access specifier, got '{}'");
            Token id = consume();

            lhs = std::make_unique<IndexNode>(std::move(lhs), std::make_unique<SymbolNode>(id));
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

            expect(PAREN_CLOSE, "Expected ')' to close function call arguments, got '{}'");
            consume();

            lhs = std::make_unique<CallNode>(std::move(lhs), std::move(arguments));
            break;
        }
        case KW_AS: {
            consume();
            lhs = std::make_unique<TypeCastNode>(std::move(lhs), parse_type());
            break;
        }
        default:
            return lhs;
        }
    }

    return lhs;
}

pExprNode Parser::parse_binary(int precedence) {
    pExprNode lhs = parse_postfix(parse_primary());

    while (position < program.token_stream->size() && current().is_operator()) {
        Token op      = current();
        int   op_prec = op.bin_prec();
        if (op_prec < precedence) {
            break;
        }

        consume();

        pExprNode rhs = parse_binary(op_prec + 1);

        lhs = std::make_unique<BinaryNode>(op, std::move(lhs), std::move(rhs));
    }

    return lhs;
}

pExprNode Parser::parse_expr() {
    return parse_binary(0);
}

pStmtNode Parser::parse_declaration() {
    Token     declaration_keyword = consume();
    TokenType declaration_type    = declaration_keyword.type;

    bool is_local  = declaration_type == KW_LOCAL;
    bool is_global = declaration_type == KW_GLOBAL;
    bool is_const  = declaration_type == KW_CONST;

    if (is_local && current().type == KW_CONST) {
        is_const = true;
        consume();
    }

    if (current().type == KW_FUNC) {
        consume();

        expect(IDENTIFIER, "Expected identifier for function declaration, got '{}'");
        Token identifier = consume();

        expect(PAREN_OPEN, "Expected '(' to open function parameters, got '{}'");
        consume();

        std::vector<ParameterNode> parameters;

        while (current().type != PAREN_CLOSE) {
            Modifiers modifiers = parse_modifiers();

            expect(IDENTIFIER, "Expected identifier for function parameter name, got '{}'");
            Token identifier = consume();

            expect(COLON, "Expected ':' to segregate function parameter and type, got '{}'");
            consume();

            parameters.emplace_back(identifier, modifiers, parse_type());

            if (current().type != PAREN_CLOSE) {
                expect(COMMA, "Expected ',' to seperate function parameters");
                consume();
            }
        }

        expect(PAREN_CLOSE, "Expected ')' to close function parameters, got '{}'");
        consume();

        expect(RETURNS, "Expected '->' to denote function return type, got '{}'");
        consume();

        Modifiers modifiers{is_const};
        pTypeNode returns    = parse_type();
        pStmtNode body_scope = parse_scope();

        return std::make_unique<FunctionNode>(
            is_global,
            modifiers,
            identifier,
            std::move(body_scope),
            std::move(returns),
            std::move(parameters)
        );
    }

    pTypeNode type;

    expect(IDENTIFIER, "Expected identifier for variable declaration, got '{}'");
    Token identifier = consume();

    if (current().type == COLON) {
        consume();
        type = parse_type();
    }
    else {
        type = std::make_unique<AutoNode>();
    }

    expect(EQUAL, "Expected '=' for variable declaration, got '{}'");
    consume();
    pExprNode value = parse_expr();

    // Add expression reference to type
    type->expression = value.get();

    return std::make_unique<DeclarationNode>(
        is_global, Modifiers{is_const}, identifier, std::move(value), std::move(type)
    );
}

pStmtNode Parser::parse_scope() {
    expect(BRACE_OPEN, "Expected '{{' to open scope, got '{}'");
    consume();

    std::vector<pStmtNode> scope_statements;
    while (current().type != BRACE_CLOSE) {
        pStmtNode stmt = parse_stmt();
        scope_statements.emplace_back(std::move(stmt));
    }

    expect(BRACE_CLOSE, "Expected '}}' to close scope, got '{}'");
    consume();

    return std::make_unique<ScopeNode>(std::move(scope_statements));
}

pStmtNode Parser::parse_if() {
    using ElseIfNode = IfNode::ElseIfNode;

    consume();

    pExprNode condition = parse_expr();
    pStmtNode scope     = parse_scope();

    std::optional<pStmtNode> else_scope;
    std::vector<ElseIfNode>  elseif_nodes;

    while (current().type == KW_ELIF) {
        consume();

        pExprNode  elseif_condition = parse_expr();
        pStmtNode  elseif_scope     = parse_scope();
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

pStmtNode Parser::parse_return() {
    consume();

    try { // Optional return value
        pExprNode expr = parse_expr();
        return std::make_unique<ReturnNode>(std::move(expr));
    }
    catch (const ParserError& e) {
        return std::make_unique<ReturnNode>(nullptr);
    }
}

pStmtNode Parser::parse_while() {
    consume();

    pExprNode condition = parse_expr();
    pStmtNode body      = parse_scope();

    return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

pStmtNode Parser::parse_stmt() {
    Token  initial_token    = current();
    size_t initial_position = position;

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
    case KW_RETURN:
        return parse_return();
    case KW_WHILE:
        return parse_while();
    case KW_BREAK:
        return std::make_unique<BreakNode>(consume());
    case KW_CONTINUE:
        return std::make_unique<ContinueNode>(consume());
    default:
        try {
            pExprNode expression = parse_expr();

            if (current().lexeme == "=" || (current().is_operator() && peek().lexeme == "=")) {
                Token possible_augment = consume();
                if (possible_augment.lexeme != "=") {
                    consume();
                }

                return std::make_unique<AssignNode>(
                    std::move(expression), possible_augment, parse_expr()
                );
            }

            return std::make_unique<ExprStmtNode>(std::move(expression));
        }
        catch (const ParserError&) {
            // I thought that rethrowing exceptions was a meme,
            // but here I am.
            parser_error(
                std::format("Unexpected token '{}' while parsing statement", initial_token.lexeme),
                initial_position
            );
        }

        VIA_ASSERT(false, "wait... how did you get this assertion to fail? you're a wizard!")
    }

    VIA_UNREACHABLE;
    return nullptr;
}

bool Parser::parse() {
    bool failed = false;
    while (!failed) {
        if (current().type == EOF_) {
            break;
        }

        try {
            pStmtNode stmt = parse_stmt();
            program.ast->statements.emplace_back(std::move(stmt));
        }
        catch (const ParserError& e) {
            failed = true;
            emitter.out(program.token_stream->at(e.where()), e.what(), Error);
            break;
        }
        catch (const std::exception& e) {
            failed = true;
            emitter.out_flat(e.what(), Error);
            break;
        }
    }

    return failed;
}

VIA_NAMESPACE_END
