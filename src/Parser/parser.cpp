/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "parser.h"
#include "Parser/ast.h"
#include "token.h"
#include <vector>

using namespace via::Tokenization;
using namespace via::Parsing::AST;

namespace via::Parsing
{

Token Parser::consume()
{
    return container.tokens.at(current_position++);
}

Token Parser::peek(int offset) const
{
    return container.tokens.at(current_position + offset);
}

bool Parser::is_value(const std::string &value, int offset) const
{
    return peek(offset).value == value;
}

bool Parser::is_type(TokenType type, int offset) const
{
    return peek(offset).type == type;
}

ExprNode *Parser::parse_expr()
{
    return parse_bin_expr();
}

ExprNode *Parser::parse_bin_expr(int precedence)
{
    // Parse the left-hand side of the expression
    ExprNode *lhs = parse_prim_expr();

    while (true)
    {
        Token token = peek();
        int token_precedence = token.bin_prec();

        // Stop if the current token's precedence is lower than the given precedence
        if (token_precedence < precedence)
            break;

        consume(); // Consume the operator

        // Parse the right-hand side of the expression
        ExprNode *rhs = parse_bin_expr(token_precedence + 1);

        BinaryExprNode bin = {
            .op = token,
            .lhs = lhs,
            .rhs = rhs,
        };

        // Properly construct a BinaryExprNode, respecting the AST structure
        lhs = m_alloc.emplace<ExprNode>(bin);
    }

    return lhs;
}

ExprNode *Parser::parse_prim_expr()
{
    Token token = consume();

    switch (token.type)
    {
    case TokenType::IDENTIFIER:
    {
        VarExprNode varid;
        varid.ident = token;

        // Handle identifiers, which might be variables or function calls
        if (is_type(TokenType::PAREN_OPEN)) // Function call
        {
            consume(); // Consume '('
            std::vector<ExprNode> args = parse_call_arguments();
            consume(); // Consume ')'

            CallExprNode call;
            call.callee = m_alloc.emplace<ExprNode>(varid);
            call.args = args;

            return m_alloc.emplace<ExprNode>();
        }

        return m_alloc.emplace<ExprNode>(varid);
    }

    case TokenType::LIT_INT:
    case TokenType::LIT_FLOAT:
    case TokenType::LIT_STRING:
    {
        LiteralExprNode lit;
        return m_alloc.emplace<ExprNode>(lit);
    }

    case TokenType::PAREN_OPEN:
    {
        ExprNode *expr = parse_expr();
        consume(); // Consume ')'
        return expr;
    }

    default:
        return nullptr;
    }
}

std::vector<ExprNode> Parser::parse_call_arguments()
{
    std::vector<ExprNode> arguments;

    while (!is_type(TokenType::PAREN_CLOSE))
    {
        arguments.push_back(*parse_expr());

        if (!is_type(TokenType::COMMA))
            break;

        consume(); // Consume ','
    }

    return arguments;
}

TypeNode *Parser::parse_type_generic()
{
    Token token = consume();

    if (token.type != TokenType::IDENTIFIER)
        // Types must start with an identifier
        return nullptr;

    GenericTypeNode type;
    type.name = token;

    // Handle generic types like List<int>
    if (is_type(TokenType::OP_LT))
    {
        consume(); // Consume '<'

        while (!is_type(TokenType::OP_GT))
        {
            type.generics.push_back(*parse_type());

            if (!is_type(TokenType::COMMA))
                break;

            consume(); // Consume ','
        }

        consume(); // Consume '>'
    }

    return m_alloc.emplace<TypeNode>(type);
}

TypeNode *Parser::parse_type()
{
    if (is_type(TokenType::IDENTIFIER))
    {
        TypeNode *gen = parse_type_generic();

        switch (peek().type)
        {
        case TokenType::AMPERSAND:
        {
            consume();

            TypeNode *rtype = parse_type_generic();
            UnionTypeNode utype;
            utype.lhs = gen;
            utype.rhs = rtype;

            return m_alloc.emplace<TypeNode>(utype);
        }
        case TokenType::PIPE:
        {
            std::vector<TypeNode> types{*gen};

            while (is_type(TokenType::PIPE))
            {
                consume();
                types.push_back(*parse_type());
            }

            VariantTypeNode var;
            var.types = types;

            return m_alloc.emplace<TypeNode>(var);
        }
        case TokenType::QUESTION:
        {
            consume();

            OptionalTypeNode opt;
            opt.type = gen;

            return m_alloc.emplace<TypeNode>(opt);
        }
        default:
            break;
        }

        return gen;
    }
    else if (is_type(TokenType::BRACE_OPEN))
    {
        consume();

        TypeNode *ktype = m_alloc.emplace<TypeNode>(GenericTypeNode{
            {
                TokenType::IDENTIFIER,
                "Number",
                peek().line,
                peek().offset,
            },
            {}
        });

        // Check if the key type is explicitly specified
        if (is_type(TokenType::BRACKET_OPEN))
        {
            consume(); // Consume '['
            ktype = parse_type();
            consume(); // Consume ']'
            consume(); // Consume ':'
        }

        TableTypeNode table;
        table.ktype = ktype;
        table.vtype = parse_type();

        return m_alloc.emplace<TypeNode>(table);
    }
    else if (is_type(TokenType::PAREN_OPEN))
    {
    }

    return nullptr;
}

} // namespace via::Parsing
