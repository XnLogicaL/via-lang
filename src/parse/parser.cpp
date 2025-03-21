// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "parser.h"
#include "compiler-types.h"

#define CHECK_RESULT(result)                                                                       \
    if (!result.has_value()) {                                                                     \
        return tl::unexpected(result.error());                                                     \
    }

VIA_NAMESPACE_BEGIN

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#elifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-result"
#elifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4834)
#endif

using enum OutputSeverity;
using enum TokenType;

template<typename T>
using result = Parser::result<T>;

result<Token> Parser::current() {
    return peek(0);
}

result<Token> Parser::peek(int32_t ahead) {
    if (position + ahead >= program.token_stream->size()) {
        return tl::unexpected<ParserError>({
            position,
            std::format("Unexpected end of file (attempted read of: token #{})", position + ahead),
        });
    }

    return program.token_stream->at(position + ahead);
}

result<Token> Parser::consume(uint32_t ahead) {
    uint64_t new_pos = position + static_cast<uint64_t>(ahead);
    if (new_pos >= program.token_stream->size()) {
        return tl::unexpected<ParserError>({
            position,
            std::format("Unexpected end of file (attempted read of: token #{})", new_pos),
        });
    }

    position = new_pos;
    return peek(-static_cast<int32_t>(ahead));
}

result<Token> Parser::expect_consume(TokenType type, const std::string& what) {
    result<Token> curr = current();
    if (!curr.has_value()) {
        return curr;
    }

    if (curr->type == type) {
        return consume();
    }

    return tl::unexpected<ParserError>({position, what});
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

    result<Token> expect_lt = expect_consume(OP_LT, "Expected '<' to open type generic");
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
                expect_consume(COMMA, "Expected ',' to seperate type generics");

            CHECK_RESULT(expect_comma);
        }
    }

    result<Token> expect_gt = expect_consume(OP_GT, "Expected '>' to close type generic");

    CHECK_RESULT(expect_gt);

    return std::make_unique<GenericNode>(
        identifier.value(), std::move(generics), modifiers.value()
    );
}

result<pTypeNode> Parser::parse_type_primary() {
    static std::unordered_map<std::string, ValueType> primitive_map = {
        {"nil", ValueType::nil},
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
                result<Token> expect_comma =
                    expect_consume(COMMA, "Expected ',' to seperate function parameter types");

                CHECK_RESULT(expect_comma);
            }
        }

        result<Token> expect_pc =
            expect_consume(PAREN_CLOSE, "Expected ')' to close function type parameters");
        result<Token> expect_rt =
            expect_consume(RETURNS, "Expected '->' to specify function return type");
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

    return tl::unexpected<ParserError>({
        position,
        std::format("Unexpected token '{}' while parsing type primary", tok->lexeme),
    });
}

result<pTypeNode> Parser::parse_type() {
    return parse_type_primary();
}

result<pExprNode> Parser::parse_primary() {
    result<Token> token = current();

    CHECK_RESULT(token);

    switch (token->type) {
    case LIT_INT:
    case LIT_HEX: {
        consume();
        int value = std::stoi(token->lexeme);
        return std::make_unique<LiteralNode>(*token, value);
    }
    case LIT_FLOAT: {
        consume();
        float value = std::stof(token->lexeme);
        return std::make_unique<LiteralNode>(*token, value);
    }
    case LIT_BINARY:
        consume();
        return std::make_unique<LiteralNode>(
            *token, static_cast<int>(std::bitset<64>(token->lexeme.substr(2)).to_ullong())
        );
    case LIT_NIL:
        consume();
        return std::make_unique<LiteralNode>(*token, std::monostate());
    case LIT_BOOL:
        consume();
        return std::make_unique<LiteralNode>(*token, token->lexeme == "true");
    case IDENTIFIER:
        consume();
        return std::make_unique<SymbolNode>(*token);
    case LIT_STRING:
        consume();
        return std::make_unique<LiteralNode>(*token, token->lexeme);
    case OP_SUB: {
        consume();
        result<Token>     op   = token;
        result<pExprNode> expr = parse_primary();

        CHECK_RESULT(op);
        CHECK_RESULT(expr);

        return std::make_unique<UnaryNode>(std::move(*expr));
    }
    default:
        break;
    }

    return tl::unexpected<ParserError>(
        {position,
         std::format("Unexpected token '{}' while parsing primary expression", token->lexeme)}
    );
}

result<pExprNode> Parser::parse_postfix(pExprNode lhs) {
    while (true) {
        result<Token> curr = current();

        CHECK_RESULT(curr);

        switch (curr->type) {
        case DOT: { // Member access: obj.property
            consume();

            result<Token> index_token =
                expect_consume(IDENTIFIER, "Expected identifier while parsing index");

            CHECK_RESULT(index_token);

            lhs = std::make_unique<IndexNode>(
                std::move(lhs), std::make_unique<SymbolNode>(*index_token)
            );

            break;
        }
        case BRACKET_OPEN: { // Array indexing: obj[expr]
            consume();
            result<pExprNode> index = parse_expr();
            result<Token>     expect_br =
                expect_consume(BRACKET_CLOSE, "Expected ']' to close index expression");

            CHECK_RESULT(index);
            CHECK_RESULT(expect_br);

            lhs = std::make_unique<IndexNode>(std::move(lhs), std::move(*index));
            break;
        }
        case COLON: { // Member access obj::id
            consume();

            result<Token> expect_col =
                expect_consume(COLON, "Expected ':' to complete member access delimiter");
            result<Token> id =
                expect_consume(IDENTIFIER, "Expected identifier for member access specifier");

            CHECK_RESULT(expect_col);
            CHECK_RESULT(id);

            lhs = std::make_unique<IndexNode>(std::move(lhs), std::make_unique<SymbolNode>(*id));
            break;
        }
        case PAREN_OPEN: { // Function calls: func(arg1, arg2)
            consume();
            std::vector<pExprNode> arguments;

            while (true) {
                result<Token> curr = current();

                CHECK_RESULT(curr);

                if (curr->type == PAREN_CLOSE) {
                    break;
                }

                result<pExprNode> expr = parse_expr();

                CHECK_RESULT(expr);

                arguments.emplace_back(std::move(*expr));
                curr = current();

                CHECK_RESULT(curr);

                if (curr->type == COMMA) {
                    consume();
                }
                else {
                    break;
                }
            }

            result<Token> expect_par =
                expect_consume(PAREN_CLOSE, "Expected ')' to close function call arguments");

            CHECK_RESULT(expect_par);

            lhs = std::make_unique<CallNode>(std::move(lhs), std::move(arguments));
            break;
        }
        case KW_AS: {
            consume();

            result<pTypeNode> type_result = parse_type();

            CHECK_RESULT(type_result);

            lhs = std::make_unique<TypeCastNode>(std::move(lhs), std::move(*type_result));
            break;
        }
        default:
            return lhs;
        }
    }

    return lhs;
}

result<pExprNode> Parser::parse_binary(int precedence) {
    result<pExprNode> prim = parse_primary();

    CHECK_RESULT(prim);

    result<pExprNode> lhs = parse_postfix(std::move(*prim));

    CHECK_RESULT(lhs);

    while (position < program.token_stream->size()) {
        result<Token> op = current();

        CHECK_RESULT(op);

        if (!op->is_operator()) {
            break;
        }

        int op_prec = op->bin_prec();
        if (op_prec < precedence) {
            break;
        }

        consume();

        result<pExprNode> rhs = parse_binary(op_prec + 1);

        CHECK_RESULT(rhs);

        lhs = std::make_unique<BinaryNode>(*op, std::move(*lhs), std::move(*rhs));
    }

    return lhs;
}

result<pExprNode> Parser::parse_expr() {
    return parse_binary(0);
}

result<pStmtNode> Parser::parse_declaration() {
    result<Token> declaration_keyword = consume();

    CHECK_RESULT(declaration_keyword);

    TokenType declaration_type = declaration_keyword->type;

    bool is_local  = declaration_type == KW_LOCAL;
    bool is_global = declaration_type == KW_GLOBAL;
    bool is_const  = declaration_type == KW_CONST;

    result<Token> curr = current();

    CHECK_RESULT(curr);

    if (is_local && curr->type == KW_CONST) {
        is_const = true;
        consume();
    }

    curr = current();

    CHECK_RESULT(curr);

    if (curr->type == KW_FUNC) {
        consume();

        result<Token> identifier =
            expect_consume(IDENTIFIER, "Expected identifier for function declaration");
        result<Token> expect_par =
            expect_consume(PAREN_OPEN, "Expected '(' to open function parameters");

        CHECK_RESULT(identifier);
        CHECK_RESULT(expect_par);

        std::vector<ParameterNode> parameters;

        while (true) {
            curr = current();
            if (curr.has_value() && curr->type == PAREN_CLOSE) {
                break;
            }

            result<Modifiers> modifiers = parse_modifiers();
            result<Token>     identifier =
                expect_consume(IDENTIFIER, "Expected identifier for function parameter name");
            result<Token> expect_col =
                expect_consume(COLON, "Expected ':' to segregate function parameter and type");
            result<pTypeNode> type = parse_type();

            CHECK_RESULT(modifiers);
            CHECK_RESULT(identifier);
            CHECK_RESULT(expect_col);

            parameters.emplace_back(*identifier, *modifiers, std::move(*type));
            curr = current();

            CHECK_RESULT(curr);

            if (curr->type != PAREN_CLOSE) {
                result<Token> expect_comma =
                    expect_consume(COMMA, "Expected ',' to seperate function parameters");

                CHECK_RESULT(expect_comma);
            }
            else {
                break;
            }
        }

        Modifiers modifiers{is_const};

        result<Token> expect_lpar =
            expect_consume(PAREN_CLOSE, "Expected ')' to close function parameters");
        result<Token> expect_rets =
            expect_consume(RETURNS, "Expected '->' to denote function return type");
        result<pTypeNode> returns    = parse_type();
        result<pStmtNode> body_scope = parse_scope();

        CHECK_RESULT(expect_lpar);
        CHECK_RESULT(expect_rets);
        CHECK_RESULT(returns);
        CHECK_RESULT(body_scope);

        return std::make_unique<FunctionNode>(
            is_global,
            modifiers,
            *identifier,
            std::move(*body_scope),
            std::move(*returns),
            std::move(parameters)
        );
    }

    pTypeNode type;

    result<Token> identifier =
        expect_consume(IDENTIFIER, "Expected identifier for variable declaration");
    curr = current();

    CHECK_RESULT(identifier);
    CHECK_RESULT(curr);

    if (curr->type == COLON) {
        consume();

        result<pTypeNode> temp = parse_type();

        CHECK_RESULT(temp);

        type = std::move(*temp);
    }
    else {
        type = std::make_unique<AutoNode>();
    }

    result<Token>     expect_eq = expect_consume(EQUAL, "Expected '=' for variable declaration");
    result<pExprNode> value     = parse_expr();

    CHECK_RESULT(expect_eq);
    CHECK_RESULT(value);

    // Add expression reference to type
    type->expression = value->get();

    return std::make_unique<DeclarationNode>(
        is_global, Modifiers{is_const}, *identifier, std::move(*value), std::move(type)
    );
}

result<pStmtNode> Parser::parse_scope() {
    std::vector<pStmtNode> scope_statements;
    result<Token>          expect_br = expect_consume(BRACE_OPEN, "Expected '{' to open scope");

    CHECK_RESULT(expect_br);

    while (true) {
        result<Token> curr = current();

        CHECK_RESULT(curr);

        if (curr->type == BRACE_CLOSE) {
            break;
        }

        result<pStmtNode> stmt = parse_stmt();

        CHECK_RESULT(stmt);

        scope_statements.emplace_back(std::move(*stmt));
    }

    expect_br = expect_consume(BRACE_CLOSE, "Expected '}' to close scope");

    CHECK_RESULT(expect_br);

    return std::make_unique<ScopeNode>(std::move(scope_statements));
}

result<pStmtNode> Parser::parse_if() {
    using ElseIfNode = IfNode::ElseIfNode;

    consume();

    result<pExprNode> condition = parse_expr();
    result<pStmtNode> scope     = parse_scope();

    CHECK_RESULT(condition);
    CHECK_RESULT(scope);

    std::optional<pStmtNode> else_scope;
    std::vector<ElseIfNode>  elseif_nodes;

    while (true) {
        result<Token> curr = current();

        CHECK_RESULT(curr);

        if (curr->type != KW_ELIF) {
            break;
        }

        result<pExprNode> elseif_condition = parse_expr();
        result<pStmtNode> elseif_scope     = parse_scope();

        CHECK_RESULT(elseif_condition);
        CHECK_RESULT(elseif_scope);

        ElseIfNode elseif(std::move(*elseif_condition), std::move(*elseif_scope));
        elseif_nodes.emplace_back(std::move(elseif));
    }

    result<Token> curr = current();

    CHECK_RESULT(curr);

    if (curr->type == KW_ELSE) {
        consume();

        result<pStmtNode> else_scope_inner = parse_scope();

        CHECK_RESULT(else_scope_inner);

        else_scope.emplace(std::move(*else_scope_inner));
    }
    else {
        else_scope = std::nullopt;
    }

    return std::make_unique<IfNode>(
        std::move(*condition), std::move(*scope), std::move(*else_scope), std::move(elseif_nodes)
    );
}

result<pStmtNode> Parser::parse_return() {
    consume();

    result<pExprNode> expr = parse_expr();

    CHECK_RESULT(expr);

    return std::make_unique<ReturnNode>(std::move(*expr));
}

result<pStmtNode> Parser::parse_while() {
    consume();

    result<pExprNode> condition = parse_expr();
    result<pStmtNode> body      = parse_scope();

    CHECK_RESULT(condition);
    CHECK_RESULT(body);

    return std::make_unique<WhileNode>(std::move(*condition), std::move(*body));
}

result<pStmtNode> Parser::parse_stmt() {
    result<Token> initial_token = current();
    CHECK_RESULT(initial_token);

    switch (initial_token->type) {
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
    case KW_BREAK: {
        result<Token> con = consume();
        CHECK_RESULT(con);
        return std::make_unique<BreakNode>(*con);
    }
    case KW_CONTINUE: {
        result<Token> con = consume();
        CHECK_RESULT(con);
        return std::make_unique<ContinueNode>(*con);
    }
    default:
        result<pExprNode> lvalue = parse_expr();
        result<Token>     curr   = current();
        result<Token>     pk     = peek();

        CHECK_RESULT(lvalue);
        CHECK_RESULT(curr);
        CHECK_RESULT(pk);

        if (curr->lexeme == "=" || (curr->is_operator() && pk->lexeme == "=")) {
            result<Token> possible_augment = consume();
            CHECK_RESULT(possible_augment);

            if (possible_augment->lexeme != "=") {
                consume();
            }

            result<pExprNode> rvalue = parse_expr();

            return std::make_unique<AssignNode>(
                std::move(*lvalue), *possible_augment, std::move(*rvalue)
            );
        }

        return std::make_unique<ExprStmtNode>(std::move(*lvalue));
    }

    VIA_UNREACHABLE;
    return nullptr;
}

bool Parser::parse() {
    while (true) {
        result<Token> curr = current();
        if (!curr.has_value()) {
            auto err = curr.error();
            emitter.out(program.token_stream->at(err.where), err.what, Error);
            return true;
        }

        if (curr->type == EOF_) {
            break;
        }

        result<pStmtNode> stmt = parse_stmt();
        if (!stmt.has_value()) {
            auto err = stmt.error();
            emitter.out(program.token_stream->at(err.where), err.what, Error);
            return true;
        }

        program.ast->statements.emplace_back(std::move(*stmt));
    }

    return false;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#elifdef __clang__
#pragma clang diagnostic pop
#elifdef _MSC_VER
#pragma warning(pop)
#endif

VIA_NAMESPACE_END
