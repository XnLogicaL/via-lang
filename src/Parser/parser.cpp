/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "parser.h"
#include "ast.h"
#include "token.h"

#define PARSER_ERROR(msg) \
    { \
        emitter.out(current_position, (msg), OutputSeverity::ERROR_); \
        failed = true; \
    }

#define PARSER_WARNING(msg) \
    { \
        emitter.out(current_position, (msg), OutputSeverity::WARNING); \
    }

#define PARSER_ERROR_AT(msg, at) \
    { \
        emitter.out((at), (msg), OutputSeverity::ERROR_); \
        failed = true; \
    }

#define PARSER_WARNING_AT(msg, at) \
    { \
        emitter.out((at), (msg), OutputSeverity::WARNING); \
    }

namespace via
{

Token Parser::consume()
{
    std::vector<Token> tokens = program.tokens->tokens;

    if (current_position >= tokens.size())
        return tokens.at(tokens.size() - 1);
    return tokens.at(current_position++);
}

Token Parser::peek(int offset) const
{
    std::vector<Token> tokens = program.tokens->tokens;

    if (current_position >= tokens.size())
        return tokens.at(tokens.size() - 1);
    return tokens.at(current_position + offset);
}

bool Parser::is_value(const std::string &value, int offset) const
{
    return peek(offset).value == value;
}

bool Parser::is_type(TokenType type, int offset) const
{
    return peek(offset).type == type;
}

bool Parser::is_keyword()
{
#define is(ty) (is_type(TokenType::ty))
    return (
        is(KW_DEFINE) || is(KW_DEFINED) || is(KW_RETURN) || is(KW_TYPE) || is(KW_TYPEOF) || is(KW_AND) || is(KW_AS) || is(KW_BREAK) || is(KW_CASE) ||
        is(KW_CONST) || is(KW_CONTINUE) || is(KW_DEFAULT) || is(KW_DO) || is(KW_ELIF) || is(KW_ELSE) || is(KW_EXPORT) || is(KW_RETURN) ||
        is(KW_FUNC) || is(KW_FOR) || is(KW_IMPORT) || is(KW_IN) || is(KW_IF) || is(KW_GLOBAL) || is(KW_LOCAL) || is(KW_NAMESPACE) || is(KW_MACRO) ||
        is(KW_NOT) || is(KW_WHILE) || is(KW_STRUCT) || is(KW_PROPERTY) || is(KW_OR) || is(KW_NEW) || is(KW_STRICT)
    );
#undef is
}

void Parser::panic_and_recover()
{
    while (current_position < program.tokens->tokens.size() && !is_keyword())
        consume();
}

DeclarationType Parser::get_decl_type(TokenType type)
{
    static const HashMap<TokenType, DeclarationType> decl_type_table = {
        {TokenType::KW_LOCAL, DeclarationType::Local},
        {TokenType::KW_GLOBAL, DeclarationType::Global},
        {TokenType::KW_PROPERTY, DeclarationType::Property},
        // TODO: Add 'meta' keyword support
    };

    auto it = decl_type_table.find(type);
    if (it != decl_type_table.end())
        return it->second;

    return DeclarationType::Local;
}

Pragma Parser::parse_pragma()
{
    bool failed_inner = false;

    consume();

    if (!is_type(TokenType::IDENTIFIER))
    {
        PARSER_ERROR(std::format("Unexpected token '{}' while parsing pragma", peek().value));
        failed_inner = true;
    }

    Pragma pragma;
    pragma.body = consume();

    if (is_type(TokenType::PAREN_OPEN))
    {
        consume();

        while (true)
        {
            if (is_type(TokenType::PAREN_CLOSE))
                break;

            Token argument = consume();
            pragma.arguments.push_back(argument);

            if (is_type(TokenType::PAREN_CLOSE))
                break;

            if (is_type(TokenType::COMMA))
            {
                PARSER_ERROR("Expected ',' to seperate pragma arguments");
                failed_inner = true;
            }

            consume();
        }

        if (!is_type(TokenType::PAREN_CLOSE))
        {
            PARSER_ERROR("Expected ')' to close pragma arguments");
            failed_inner = true;
        }

        consume();
    }

    if (failed_inner)
        panic_and_recover();

    return pragma;
}

StatementModifiers Parser::parse_modifiers(std::function<bool(void)> predecate)
{
    bool failed_inner = false;
    StatementModifiers modifiers{false, false};

    if (!predecate())
        return modifiers;

    while (predecate())
    {
        if (is_type(TokenType::KW_STRICT))
        {
            modifiers.is_strict = true;
            consume();
        }
        else if (is_type(TokenType::KW_CONST))
        {
            modifiers.is_const = true;
            consume();
        }
        else
        {
            PARSER_ERROR(std::format("Unexpected token '{}' while parsing statement modifiers", peek().value));
            failed_inner = true;
            consume();
        }
    }

    if (failed_inner)
        panic_and_recover();

    return modifiers;
}

ExprNode *Parser::parse_table_expr()
{
    bool failed_inner = false;
    size_t index = 0;

    auto parse_kv_pair = [this, &failed_inner, &index]() -> TableExprNode::KVPair
    {
        if (is_type(TokenType::BRACKET_OPEN))
        {
            consume(); // Consume '['

            ExprNode *key = parse_expr();

            if (!is_type(TokenType::BRACKET_CLOSE))
            {
                // TODO: Find a better error message for this, the current one is dogshit
                PARSER_ERROR("Expected ']' to close table key expression container");
                failed_inner = true;
            }

            consume(); // Consume ']'

            if (!is_type(TokenType::OP_ASGN))
            {
                PARSER_ERROR("Expected '=' to assign table key value pair");
                failed_inner = true;
            }

            consume(); // Consume '='

            ExprNode *val = parse_expr();

            return {key, val};
        }

        LiteralExprNode key_expr{
            Token{
                TokenType::LIT_INT,
                std::to_string(index++),
                peek().line,
                peek().offset,
                false,
            },
        };

        ExprNode *key = alloc->emplace<ExprNode>(key_expr);
        ExprNode *expr = parse_expr();

        // TODO: Check for binary expression (with operator OP_ASGN) to check for non-wrapped key value specifiers
        return {key, expr};
    };

    TableExprNode *table_expr = alloc->emplace<TableExprNode>();

    while (!is_type(TokenType::EOF_))
    {
        if (is_type(TokenType::BRACE_CLOSE))
            break;

        TableExprNode::KVPair pair = parse_kv_pair();
        table_expr->values.push_back(pair);

        if (is_type(TokenType::BRACE_CLOSE))
            break;

        if (!is_type(TokenType::COMMA))
        {
            PARSER_ERROR("Expected ',' to seperate table key-value pairs");
            failed_inner = true;
        }

        consume();
    }

    if (!is_type(TokenType::BRACE_CLOSE))
    {
        PARSER_ERROR("Expected '}' to close table key-value pairs");
        failed_inner = true;
    }

    consume();

    if (failed_inner)
        panic_and_recover();

    return alloc->emplace<ExprNode>(*table_expr);
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
        if (current_position >= program.tokens->tokens.size())
            break;

        // Check if there are more tokens to process
        if (is_type(TokenType::EOF_))
            break;

        // This is not an error because not every expression is binary
        if (!peek().is_operator())
            break;

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

        // Construct a BinaryExprNode
        lhs = alloc->emplace<ExprNode>(ExprNode(bin));
    }

    return lhs;
}

ExprNode *Parser::parse_prim_expr()
{
    bool failed_inner = false;
    Token token = consume();
    // Start with the base expression
    ExprNode *base_expr = nullptr;

    switch (token.type)
    {
    case TokenType::IDENTIFIER:
    {
        // Initial identifier (variable or function)
        VarExprNode *varid = alloc->emplace<VarExprNode>();
        varid->ident = token;
        base_expr = alloc->emplace<ExprNode>(*varid);
        break;
    }

    case TokenType::LIT_INT:
    case TokenType::LIT_FLOAT:
    case TokenType::LIT_HEX:
    case TokenType::LIT_BINARY:
    case TokenType::LIT_STRING:
    case TokenType::LIT_BOOL:
    case TokenType::LIT_NIL:
    {
        LiteralExprNode *lit = alloc->emplace<LiteralExprNode>();
        lit->value = token;
        base_expr = alloc->emplace<ExprNode>(*lit);
        break;
    }

    case TokenType::PAREN_OPEN:
    {
        base_expr = parse_expr();

        if (!is_type(TokenType::PAREN_CLOSE))
        {
            PARSER_ERROR("Expected ')' to close group expression");
            failed_inner = true;
        }

        consume(); // Consume ')'
        break;
    }

    case TokenType::KW_FUNC:
    {
        if (!is_type(TokenType::PAREN_OPEN))
        {
            PARSER_ERROR("Expected '(' to open anonymous function (lambda) arguments");
            failed_inner = true;
        }

        consume(); // Consume '('

        LambdaExprNode *lambda = alloc->emplace<LambdaExprNode>();

        while (!is_type(TokenType::EOF_))
        {
            if (is_type(TokenType::PAREN_CLOSE))
                break;

            TypedParamNode *param = parse_parameter();
            lambda->params.push_back(*param);

            if (is_type(TokenType::PAREN_CLOSE))
                break;

            if (!is_type(TokenType::COMMA))
            {
                PARSER_ERROR("Expected ',' to seperate anonymous function (lambda) arguments");
                failed_inner = true;
            }

            consume();
        }

        consume(); // Consume ')'

        if (!is_type(TokenType::BRACE_OPEN))
        {
            PARSER_ERROR("Expected '{' to open anonymous function (lambda) body")
            failed_inner = true;
        }

        lambda->body = parse_scope_statement();
        base_expr = alloc->emplace<ExprNode>(*lambda);
        break;
    }

    case TokenType::BRACE_OPEN:
    {
        base_expr = parse_table_expr();
        break;
    }

    default:
        PARSER_ERROR_AT(std::format("Unexpected token '{}' while parsing primary expression", token.value), current_position - 1);
        failed_inner = true;
        break;
    }

    // Parse the chain of operations
    while (true)
    {
        if (current_position >= program.tokens->tokens.size())
        {
            PARSER_ERROR("Unexpected end to expression");
            failed_inner = true;
            break;
        }

        if (is_type(TokenType::DOT)) // Member access: obj.prop
        {
            consume(); // Consume '.'

            if (!is_type(TokenType::IDENTIFIER))
            {
                PARSER_ERROR("Expected identifier for table member access key");
                failed_inner = true;
            }

            Token member = consume();
            // Change the member type from IDENTIFIER to LIT_STRING
            // in order to not confuse the compiler
            member.type = TokenType::LIT_STRING;

            LiteralExprNode *member_var = alloc->emplace<LiteralExprNode>();
            member_var->value = member;

            IndexExprNode *index_expr = alloc->emplace<IndexExprNode>();
            index_expr->object = base_expr;
            index_expr->index = alloc->emplace<ExprNode>(*member_var);

            base_expr = alloc->emplace<ExprNode>(*index_expr);
        }
        // Function call: obj.func(args)
        else if (is_type(TokenType::PAREN_OPEN) || is_type(TokenType::OP_LT))
        {
            bool has_generics = is_type(TokenType::OP_LT);
            consume(); // Consume '(' or '<'

            CallExprNode *call_expr = alloc->emplace<CallExprNode>();
            call_expr->callee = base_expr;

            if (has_generics)
                call_expr->type_args = parse_call_type_arguments();
            call_expr->args = parse_call_arguments();
            base_expr = alloc->emplace<ExprNode>(*call_expr);
        }
        // Array indexing: obj[index]
        else if (is_type(TokenType::BRACKET_OPEN))
        {
            consume(); // Consume '['

            IndexExprNode *index_expr = alloc->emplace<IndexExprNode>();
            index_expr->object = base_expr;
            index_expr->index = parse_expr();

            if (!is_type(TokenType::BRACKET_CLOSE))
            {
                PARSER_ERROR("Expected ']' to close member access statement");
                failed_inner = true;
            }

            consume(); // Consume ']'

            base_expr = alloc->emplace<ExprNode>(*index_expr);
        }
        else if (is_type(TokenType::OP_INCREMENT))
        {
            IncExprNode *inc_expr = alloc->emplace<IncExprNode>();
            inc_expr->expr = base_expr;

            base_expr = alloc->emplace<ExprNode>(*inc_expr);
        }

        else if (is_type(TokenType::OP_DECREMENT))
        {
            DecExprNode *dec_expr = alloc->emplace<DecExprNode>();
            dec_expr->expr = base_expr;

            base_expr = alloc->emplace<ExprNode>(*dec_expr);
        }
        else
            break;
    }

    if (failed_inner)
        panic_and_recover();

    return base_expr;
}

std::vector<ExprNode *> Parser::parse_call_arguments()
{
    bool failed_inner = false;
    std::vector<ExprNode *> arguments;

    while (!is_type(TokenType::PAREN_CLOSE))
    {
        ExprNode *expr = parse_expr();
        if (!expr)
        {
            consume();
            continue;
        }

        arguments.push_back(expr);

        if (is_type(TokenType::PAREN_CLOSE))
            break;

        if (!is_type(TokenType::COMMA))
        {
            PARSER_ERROR("Expected ',' to seperate call arguments");
            failed_inner = true;
        }

        consume(); // Consume ','

        if (is_type(TokenType::PAREN_CLOSE))
        {
            PARSER_ERROR("Function call arguments cannot be closed with ','");
            failed_inner = true;
        }
    }

    if (!is_type(TokenType::PAREN_CLOSE))
    {
        PARSER_ERROR("Expected ')' to close call arguments");
        failed_inner = true;
    }

    consume();

    if (failed_inner)
        panic_and_recover();

    return arguments;
}

std::vector<TypeNode *> Parser::parse_call_type_arguments()
{
    bool failed_inner = false;
    std::vector<TypeNode *> arguments;

    while (!is_type(TokenType::OP_GT))
    {
        TypeNode *type = parse_type();
        if (!type)
        {
            consume();
            continue;
        }

        arguments.push_back(type);

        if (!is_type(TokenType::COMMA))
        {
            PARSER_ERROR("Expected ',' to seperate call type generics");
            failed_inner = true;
        }

        consume(); // Consume ','
    }

    if (failed_inner)
        panic_and_recover();

    return arguments;
}

TypeNode *Parser::parse_type_generic()
{
    bool failed_inner = false;
    Token token = consume();

    if (token.type != TokenType::IDENTIFIER)
    {
        PARSER_ERROR("Expected identifier for type generic body");
        failed_inner = true;
    }

    GenericTypeNode type;
    type.name = token;

    // Handle generic types like List<int>
    if (is_type(TokenType::OP_LT))
    {
        consume(); // Consume '<'

        while (!is_type(TokenType::OP_GT))
        {
            TypeNode *type_ = parse_type();
            if (!type_)
            {
                consume();
                continue;
            }

            type.generics.push_back(*type_);

            if (!is_type(TokenType::COMMA))
            {
                PARSER_ERROR("Expected ',' to seperate type generics");
                failed_inner = true;
            }

            consume(); // Consume ','
        }

        consume(); // Consume '>'
    }

    if (failed_inner)
        panic_and_recover();

    return alloc->emplace<TypeNode>(type);
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

            return alloc->emplace<TypeNode>(utype);
        }
        case TokenType::PIPE:
        {
            std::vector<TypeNode> types{*gen};

            while (is_type(TokenType::PIPE))
            {
                consume();

                TypeNode *type = parse_type();
                if (!type)
                {
                    consume();
                    continue;
                }

                types.push_back(*type);
            }

            VariantTypeNode var;
            var.types = types;

            return alloc->emplace<TypeNode>(var);
        }
        case TokenType::QUESTION:
        {
            consume();

            OptionalTypeNode opt;
            opt.type = gen;

            return alloc->emplace<TypeNode>(opt);
        }
        default:
            break;
        }

        return gen;
    }
    else if (is_type(TokenType::BRACE_OPEN))
    {
        consume();

        TypeNode *ktype = alloc->emplace<TypeNode>(GenericTypeNode{
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

            if (!is_type(TokenType::BRACKET_OPEN))
                PARSER_ERROR("Expected ']' to close table key type designator");

            consume(); // Consume ']'

            if (!is_type(TokenType::COLON))
                PARSER_ERROR("Expected ':' to before table value type designator");

            consume(); // Consume ':'
        }

        TableTypeNode table;
        table.ktype = ktype;
        table.vtype = parse_type();

        return alloc->emplace<TypeNode>(table);
    }
    else if (is_type(TokenType::PAREN_OPEN))
    {
    }

    PARSER_ERROR(std::format("Unexpected token '{}' while parsing type", peek().value));
    panic_and_recover();
    return nullptr;
}

TypedParamNode *Parser::parse_parameter()
{
    StatementModifiers modifiers = parse_modifiers(
        [this]()
        {
            return !is_type(TokenType::IDENTIFIER);
        }
    );

    if (!is_type(TokenType::IDENTIFIER))
        PARSER_ERROR("Expected identifier for parameter identifier");

    // Consume identifier
    TypedParamNode *param = alloc->emplace<TypedParamNode>();
    param->ident = consume();
    param->modifiers = modifiers;

    if (is_type(TokenType::COLON))
    {
        consume();

        TypeNode *type = parse_type();
        if (!type)
            consume();
        else
            param->type = *type;
    }

    return param;
}

VariableDeclStmtNode *Parser::parse_var_declaration()
{
    Token declaration_type_token = consume();
    StatementModifiers modifiers = parse_modifiers(
        [this]()
        {
            return !is_type(TokenType::IDENTIFIER);
        }
    );

    if (modifiers.is_strict)
        PARSER_ERROR("Using 'strict' modifier on variable declaration")

    if (!is_type(TokenType::IDENTIFIER))
        PARSER_ERROR("Expected identifier for local variable identifier");

    VariableDeclStmtNode *decl = alloc->emplace<VariableDeclStmtNode>();
    decl->ident = consume();
    decl->modifiers = modifiers;
    decl->decl_type = get_decl_type(declaration_type_token.type);

    if (is_type(TokenType::COLON))
    {
        consume();

        TypeNode *type = parse_type();
        if (!type)
            consume();
        else
            decl->type = *type;
    }

    if (is_type(TokenType::OP_ASGN))
    {
        consume();

        ExprNode *expr = parse_expr();
        if (!expr)
            consume();
        else
            decl->value = *expr;
    }

    return decl;
}

CallStmtNode *Parser::parse_call_statement(ExprNode *expr)
{
    CallStmtNode *call = alloc->emplace<CallStmtNode>();
    call->callee = expr;

    if (is_type(TokenType::OP_LT))
    {
        consume();
        call->generics = parse_call_type_arguments();
    }

    if (!is_type(TokenType::PAREN_OPEN))
        PARSER_ERROR("Expected '(' to open call arguments");

    consume();

    call->args = parse_call_arguments();

    if (failed)
        panic_and_recover();

    return call;
}

AssignStmtNode *Parser::parse_assignment_statement(ExprNode *expr)
{
    consume();

    AssignStmtNode *asgn = alloc->emplace<AssignStmtNode>();
    asgn->target = expr;
    asgn->value = parse_expr();

    return asgn;
}

ReturnStmtNode *Parser::parse_return_statement()
{
    consume();

    ReturnStmtNode *ret = alloc->emplace<ReturnStmtNode>();
    ExprNode *expr = parse_expr();
    if (!expr)
    {
        consume();
        return ret;
    }

    ret->values.push_back(*expr);

    while (is_type(TokenType::COMMA))
    {
        consume();

        ExprNode *expr = parse_expr();
        if (!expr)
        {
            consume();
            continue;
        }

        ret->values.push_back(*expr);
    }

    return ret;
}

WhileStmtNode *Parser::parse_while_statement()
{
    consume();

    WhileStmtNode *loop = alloc->emplace<WhileStmtNode>();
    loop->condition = parse_expr();
    loop->body = parse_scope_statement();

    return loop;
}

ForStmtNode *Parser::parse_for_statement()
{
    consume();

    if (!is_type(TokenType::IDENTIFIER))
        PARSER_ERROR("Expected identifier for for-loop keys variable")

    ForStmtNode *loop = alloc->emplace<ForStmtNode>();
    loop->keys = consume();

    if (!is_type(TokenType::COMMA))
        PARSER_ERROR("Expected ',' to seperate for-loop variable");

    // Consume ','
    consume();

    if (!is_type(TokenType::IDENTIFIER))
        PARSER_ERROR("Expected identifier for for-loop values variable")

    loop->values = consume();

    if (!is_type(TokenType::KW_IN))
        PARSER_ERROR("Expected 'in' before iterator");

    // Consume 'in'
    consume();

    loop->iterator = parse_expr();
    loop->body = parse_scope_statement();

    if (failed)
        panic_and_recover();

    return loop;
}

IfStmtNode *Parser::parse_if_statement()
{
    consume();

    IfStmtNode *ifs = alloc->emplace<IfStmtNode>();
    ifs->condition = parse_expr();
    ifs->then_body = parse_scope_statement();

    while (is_type(TokenType::KW_ELIF))
    {
        consume();

        ElifStmtNode *elif = alloc->emplace<ElifStmtNode>();
        elif->condition = parse_expr();
        elif->body = parse_scope_statement();

        ifs->elif_branches.push_back(*elif);
    }

    if (is_type(TokenType::KW_ELSE))
    {
        consume();
        ifs->else_body = *parse_scope_statement();
    }

    return ifs;
}

SwitchStmtNode *Parser::parse_switch_statement()
{
    consume(); // Consume 'switch'

    SwitchStmtNode *switchs = alloc->emplace<SwitchStmtNode>();
    switchs->condition = parse_expr();

    while (is_type(TokenType::KW_CASE))
    {
        consume();

        CaseStmtNode *cases = alloc->emplace<CaseStmtNode>();
        cases->value = parse_expr();
        cases->body = parse_scope_statement();

        switchs->cases.push_back(*cases);
    }

    if (is_type(TokenType::KW_DEFAULT))
    {
        consume();
        switchs->default_case = *parse_scope_statement();
    }

    return switchs;
}

// Parses a statement that follows the function-grammar
FunctionDeclStmtNode *Parser::parse_function_declaration(DeclarationType decl_type)
{
    bool failed_inner = false;
    // No need to check this
    consume(); // Consume 'func'

    StatementModifiers modifiers = parse_modifiers(
        [this]()
        {
            return !is_type(TokenType::IDENTIFIER) && !is_type(TokenType::EOF_);
        }
    );

    if (!is_type(TokenType::IDENTIFIER))
    {
        PARSER_ERROR("Expected identifier for function/method declaration");
        failed_inner = true;
    }

    FunctionDeclStmtNode *decl = alloc->emplace<FunctionDeclStmtNode>();
    decl->modifiers = modifiers;
    decl->decl_type = decl_type;
    decl->ident = consume();

    // TODO: Make this safe
    if (is_type(TokenType::OP_LT))
    {
        consume();

        while (!is_type(TokenType::OP_GT))
        {
            decl->generics.push_back(consume());

            if (is_type(TokenType::COMMA))
                consume();
        }

        consume();
    }

    if (!is_type(TokenType::PAREN_OPEN))
    {
        PARSER_ERROR("Expected '(' to open function parameters");
        failed_inner = true;
    }

    consume(); // Consume '('

    // TODO: Make this safe
    while (!is_type(TokenType::PAREN_CLOSE))
    {
        decl->params.push_back(*parse_parameter());

        if (is_type(TokenType::COMMA))
            consume();
    }

    consume(); // Consume ')'

    // Check for return type specifier
    if (is_type(TokenType::OP_SUB))
    {
        consume(); // Consume '-'

        if (!is_type(TokenType::OP_GT))
        {
            PARSER_ERROR("Expected '->' for return type specifier");
            failed_inner = true;
        }

        consume(); // Consume '>'

        TypeNode *ret_type = parse_type();
        if (!ret_type)
            // Recover to '{'
            while (!is_type(TokenType::BRACE_OPEN) && !is_type(TokenType::EOF_))
                consume();
        else
            decl->return_type = std::optional<TypeNode>(*ret_type);
    }

    decl->body = parse_scope_statement();

    if (failed_inner)
        panic_and_recover();

    return decl;
}

// Parses a statement that follows the struct-grammar
StructDeclStmtNode *Parser::parse_struct_declaration(DeclarationType decl_type)
{
    bool failed_inner = false;

    consume(); // Consume 'struct'

    if (!is_type(TokenType::IDENTIFIER))
    {
        PARSER_ERROR("Expected identifier for struct declaration");
        failed_inner = true;
    }

    StructDeclStmtNode *struct_decl = alloc->emplace<StructDeclStmtNode>();
    struct_decl->ident = consume();
    struct_decl->decl_type = decl_type;

    if (!is_type(TokenType::BRACE_OPEN))
    {
        PARSER_ERROR("Expected '{' to open struct body");
        failed_inner = true;
    }

    consume(); // Consume '{'

    while (!is_type(TokenType::EOF_))
    {
        if (is_type(TokenType::BRACE_CLOSE))
            break;

        bool is_property = is_type(TokenType::KW_PROPERTY);
        if (!is_property /* TODO: Add 'meta' keyword support */)
        {
            PARSER_ERROR(std::format("Unexpected token '{}' while parsing struct properties", peek().value));
            failed_inner = true;
        }

        // We do not consume 'property' because it's an essential sentinel keyword for both subparser functions
        if (is_type(TokenType::KW_FUNC, 1))
        {
            FunctionDeclStmtNode *func_decl = parse_function_declaration(is_property ? DeclarationType::Property : DeclarationType::Meta);
            StmtNode *func_decl_stmt = alloc->emplace<StmtNode>(*func_decl);
            struct_decl->declarations.push_back(*func_decl_stmt);
        }
        else
        {
            VariableDeclStmtNode *var_decl = parse_var_declaration();
            StmtNode *var_decl_stmt = alloc->emplace<StmtNode>(*var_decl);
            struct_decl->declarations.push_back(*var_decl_stmt);
            // TODO: Perform checks like type presence here
        }
    }

    if (!is_type(TokenType::BRACE_CLOSE))
    {
        PARSER_ERROR("Expected '}' to close struct body");
        failed_inner = true;
    }

    consume(); // Consume '}'

    if (failed_inner)
        panic_and_recover();

    return struct_decl;
}

ScopeStmtNode *Parser::parse_scope_statement()
{
    consume(); // Consume '{'

    // Instantiate scope statement node
    ScopeStmtNode *scope = alloc->emplace<ScopeStmtNode>();

    while (!is_type(TokenType::BRACE_CLOSE))
    {
        StmtNode *stmt = parse_statement();
        // Check if statement is invalid
        if (!stmt)
        {
            consume();
            continue;
        }

        scope->statements.push_back(*stmt);
    }

    consume(); // Consume '}'

    return scope;
}

StmtNode *Parser::parse_statement()
{
#define emplace(expr) alloc->emplace<StmtNode>(expr)

    switch (peek().type)
    {
    case TokenType::EOF_:
        return nullptr;
    case TokenType::KW_GLOBAL:
    case TokenType::KW_LOCAL:
    {
        DeclarationType decl_type = get_decl_type(peek().type);

        if (is_type(TokenType::KW_FUNC, 1))
            return emplace(*parse_function_declaration(decl_type));
        else if (is_type(TokenType::KW_STRUCT, 1))
            return emplace(*parse_struct_declaration(decl_type));

        return emplace(*parse_var_declaration());
    }
    case TokenType::KW_RETURN:
        return emplace(*parse_return_statement());
    case TokenType::KW_WHILE:
        return emplace(*parse_while_statement());
    case TokenType::KW_FOR:
        return emplace(*parse_for_statement());
    case TokenType::KW_IF:
        return emplace(*parse_if_statement());
    case TokenType::KW_SWITCH:
        return emplace(*parse_switch_statement());
    case TokenType::KW_FUNC:
        return emplace(*parse_function_declaration(DeclarationType::Local));
    case TokenType::KW_STRUCT:
        return emplace(*parse_struct_declaration(DeclarationType::Local));
    case TokenType::KW_NAMESPACE:
        // TODO: Self explanatory...
        break;
    case TokenType::KW_BREAK:
        return emplace(*alloc->emplace<BreakStmtNode>());
    case TokenType::KW_CONTINUE:
        return emplace(*alloc->emplace<ContinueStmtNode>());
    case TokenType::KW_DO:
        consume();
        return emplace(*parse_scope_statement());
    default:
    {
        ExprNode *expr = parse_expr();

        if (CallExprNode *call = std::get_if<CallExprNode>(&expr->expr))
        {
            // TODO: Construct a new statement based on the call expression
            // The current implementation is bad for several reasons;
            // - Soft memory leak (short term leak)
            // - Unscalable
            CallStmtNode *stmt = alloc->emplace<CallStmtNode>();
            stmt->callee = call->callee;
            stmt->args = call->args;
            stmt->generics = call->type_args;

            return emplace(*stmt);
        }
        else if (BinaryExprNode *bin_expr = std::get_if<BinaryExprNode>(&expr->expr))
        {
            // Only binary expression allowed is an "assignment operation"
            // Which isn't a actually a real thing, it's just a quirk caused by how operators are classified
            // I guess it's beneficial in this case, although it will be HELL to debug in other cases
            // Because it technically cannot evaluate to any value
            if (bin_expr->op.type != TokenType::OP_ASGN)
                goto invalid_statement;

            AssignStmtNode *asgn = alloc->emplace<AssignStmtNode>();
            asgn->target = bin_expr->lhs;
            asgn->value = bin_expr->rhs;

            return emplace(*asgn);
        }

        goto invalid_statement;
    }
    }
invalid_statement:
    PARSER_ERROR(std::format("Unexpected token '{}' while parsing statement", peek().value));
    panic_and_recover();
    return nullptr;
#undef emplace
#undef is_function_declaration
}

void Parser::parse_program()
{
    AbstractSyntaxTree *ast = new AbstractSyntaxTree(VIA_PARSER_ALLOC_SIZE);
    this->alloc = &ast->allocator;

    for (Token tok : program.tokens->tokens)
    {
        if (is_type(TokenType::EOF_))
            break;

        StmtNode *stmt = parse_statement();
        if (!stmt)
        {
            consume();
            continue;
        }

        ast->statements.push_back(*stmt);
    }

    program.ast = ast;
}

} // namespace via
