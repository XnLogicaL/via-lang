/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "parser.h"
#include "ast.h"
#include "token.h"
#include <vector>

namespace via
{

Token Parser::consume()
{
    VIA_ASSERT(!is_type(TokenType::EOF_), "Attempt to consume EOF");
    return program.tokens->at(current_position++);
}

Token Parser::peek(int offset) const
{
    return program.tokens->at(current_position + offset);
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
        if (current_position >= program.tokens->size())
            break;

        // Check if there are more tokens to process
        if (is_type(TokenType::EOF_))
            break;

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
        lhs = alloc.emplace<ExprNode>(ExprNode(bin));
    }

    return lhs;
}

ExprNode *Parser::parse_prim_expr()
{
    Token token = consume();

    // Start with the base expression
    ExprNode *base_expr = nullptr;

    switch (token.type)
    {
    case TokenType::IDENTIFIER:
    {
        // Initial identifier (variable or function)
        VarExprNode *varid = alloc.emplace<VarExprNode>();
        varid->ident = token;
        base_expr = alloc.emplace<ExprNode>(*varid);
        break;
    }

    case TokenType::LIT_INT:
    case TokenType::LIT_FLOAT:
    case TokenType::LIT_STRING:
    case TokenType::LIT_BOOL:
    case TokenType::LIT_NIL:
    {
        LiteralExprNode lit;
        lit.value = token;
        base_expr = alloc.emplace<ExprNode>(lit);
        break;
    }

    case TokenType::PAREN_OPEN:
    {
        base_expr = parse_expr();
        consume(); // Consume ')'
        break;
    }

    default:
        VIA_ASSERT(false, std::format("parse_prim_expr(): Unexpected token: {}", token.to_string()));
        return nullptr;
    }

    // Parse the chain of operations
    while (true)
    {
        if (current_position >= program.tokens->size())
            break;

        if (is_type(TokenType::DOT)) // Member access: obj.prop
        {
            consume(); // Consume '.'

            Token member = consume();
            // Change the member type from IDENTIFIER to LIT_STRING
            // in order to not confuse the compiler
            member.type = TokenType::LIT_STRING;

            LiteralExprNode *member_var = alloc.emplace<LiteralExprNode>();
            member_var->value = member;

            IndexExprNode *index_expr = alloc.emplace<IndexExprNode>();
            index_expr->object = base_expr;
            index_expr->index = alloc.emplace<ExprNode>(*member_var);

            base_expr = alloc.emplace<ExprNode>(*index_expr);
        }
        // Function call: obj.func(args)
        else if (is_type(TokenType::PAREN_OPEN) || is_type(TokenType::OP_LT))
        {
            bool has_generics = is_type(TokenType::OP_LT);

            consume(); // Consume '(' or '<'

            CallExprNode *call_expr = alloc.emplace<CallExprNode>();
            call_expr->callee = base_expr;

            if (has_generics)
                call_expr->type_args = parse_call_type_arguments();
            call_expr->args = parse_call_arguments();
            base_expr = alloc.emplace<ExprNode>(*call_expr);
        }
        // Array indexing: obj[index]
        else if (is_type(TokenType::BRACKET_OPEN))
        {
            consume(); // Consume '['

            IndexExprNode *index_expr = alloc.emplace<IndexExprNode>();
            index_expr->object = base_expr;
            index_expr->index = parse_expr();

            consume(); // Consume ']'

            base_expr = alloc.emplace<ExprNode>(*index_expr);
        }
        else if (is_type(TokenType::OP_INC))
        {
            IncExprNode *inc_expr = alloc.emplace<IncExprNode>();
            inc_expr->expr = base_expr;

            base_expr = alloc.emplace<ExprNode>(*inc_expr);
        }

        else if (is_type(TokenType::OP_DEC))
        {
            DecExprNode *dec_expr = alloc.emplace<DecExprNode>();
            dec_expr->expr = base_expr;

            base_expr = alloc.emplace<ExprNode>(*dec_expr);
        }
        else
            break;
    }

    return base_expr;
}

std::vector<ExprNode> Parser::parse_call_arguments()
{
    std::vector<ExprNode> arguments;

    while (!is_type(TokenType::PAREN_CLOSE))
    {
        ExprNode *expr = parse_expr();
        arguments.push_back(*expr);

        if (is_type(TokenType::COMMA))
            consume(); // Consume ','
    }

    consume();

    return arguments;
}

std::vector<TypeNode> Parser::parse_call_type_arguments()
{
    std::vector<TypeNode> arguments;

    while (!is_type(TokenType::OP_GT))
    {
        arguments.push_back(*parse_type());

        if (is_type(TokenType::COMMA))
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

    return alloc.emplace<TypeNode>(type);
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

            return alloc.emplace<TypeNode>(utype);
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

            return alloc.emplace<TypeNode>(var);
        }
        case TokenType::QUESTION:
        {
            consume();

            OptionalTypeNode opt;
            opt.type = gen;

            return alloc.emplace<TypeNode>(opt);
        }
        default:
            break;
        }

        return gen;
    }
    else if (is_type(TokenType::BRACE_OPEN))
    {
        consume();

        TypeNode *ktype = alloc.emplace<TypeNode>(GenericTypeNode{
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

        return alloc.emplace<TypeNode>(table);
    }
    else if (is_type(TokenType::PAREN_OPEN))
    {
    }

    return nullptr;
}

TypedParamNode *Parser::parse_parameter()
{
    // Consume identifier
    TypedParamNode *param = alloc.emplace<TypedParamNode>();
    param->ident = consume();

    if (is_type(TokenType::COLON))
    {
        consume();
        param->type = *parse_type();
    }

    return param;
}

LocalDeclStmtNode *Parser::parse_local_declaration()
{
    // Consume 'local'
    consume();

    LocalDeclStmtNode *decl = alloc.emplace<LocalDeclStmtNode>();
    decl->is_const = false;

    if (is_type(TokenType::KW_CONST))
    {
        decl->is_const = true;
        consume();
    }

    decl->ident = consume();

    if (is_type(TokenType::COLON))
    {
        consume();
        decl->type = *parse_type();
    }

    if (is_type(TokenType::OP_ASGN))
    {
        consume();
        decl->value = *parse_expr();
    }

    return decl;
}

GlobalDeclStmtNode *Parser::parse_global_declaration()
{
    // Consume 'local'
    consume();

    GlobalDeclStmtNode *decl = alloc.emplace<GlobalDeclStmtNode>();

    if (is_type(TokenType::KW_CONST))
        consume();

    decl->ident = consume();

    if (is_type(TokenType::COLON))
    {
        consume();
        decl->type = *parse_type();
    }

    if (is_type(TokenType::OP_ASGN))
    {
        consume();
        decl->value = *parse_expr();
    }

    return decl;
}

CallStmtNode *Parser::parse_call_statement(ExprNode *expr)
{
    CallStmtNode *call = alloc.emplace<CallStmtNode>();
    call->callee = expr;

    if (is_type(TokenType::OP_LT))
    {
        consume();
        call->generics = parse_call_type_arguments();
    }

    consume();

    call->args = parse_call_arguments();
    return call;
}

AssignStmtNode *Parser::parse_assignment_statement(ExprNode *expr)
{
    consume();

    AssignStmtNode *asgn = alloc.emplace<AssignStmtNode>();
    asgn->target = expr;
    asgn->value = parse_expr();

    return asgn;
}

ReturnStmtNode *Parser::parse_return_statement()
{
    consume();

    ReturnStmtNode *ret = alloc.emplace<ReturnStmtNode>();
    ret->values.push_back(*parse_expr());

    while (is_type(TokenType::COMMA))
    {
        consume();
        ret->values.push_back(*parse_expr());
    }

    return ret;
}

WhileStmtNode *Parser::parse_while_statement()
{
    consume();

    WhileStmtNode *loop = alloc.emplace<WhileStmtNode>();
    loop->condition = parse_expr();
    loop->body = parse_scope_statement();

    return loop;
}

ForStmtNode *Parser::parse_for_statement()
{
    consume();

    ForStmtNode *loop = alloc.emplace<ForStmtNode>();
    loop->keys = consume();
    // Consume ','
    consume();

    loop->values = consume();
    // Consume 'in'
    consume();

    loop->iterator = parse_expr();
    loop->body = parse_scope_statement();

    return loop;
}

IfStmtNode *Parser::parse_if_statement()
{
    consume();

    IfStmtNode *ifs = alloc.emplace<IfStmtNode>();
    ifs->condition = parse_expr();
    ifs->then_body = parse_scope_statement();

    while (is_type(TokenType::KW_ELIF))
    {
        consume();

        ElifStmtNode *elif = alloc.emplace<ElifStmtNode>();
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
    consume();

    SwitchStmtNode *switchs = alloc.emplace<SwitchStmtNode>();
    switchs->condition = parse_expr();

    while (is_type(TokenType::KW_CASE))
    {
        consume();

        CaseStmtNode *cases = alloc.emplace<CaseStmtNode>();
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

FunctionDeclStmtNode *Parser::parse_function_declaration()
{
    bool is_global = is_type(TokenType::KW_GLOBAL);

    consume(); // Consume 'local/global'

    // Useless qualifier for now
    if (is_type(TokenType::KW_CONST))
        consume();

    consume(); // Consume 'func'

    FunctionDeclStmtNode *decl = alloc.emplace<FunctionDeclStmtNode>();
    decl->is_global = is_global;
    decl->ident = consume();

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

    consume();

    while (!is_type(TokenType::PAREN_CLOSE))
    {
        decl->params.push_back(*parse_parameter());

        if (is_type(TokenType::COMMA))
            consume();
    }

    consume();

    decl->body = parse_scope_statement();
    return decl;
}

ScopeStmtNode *Parser::parse_scope_statement()
{
    consume();

    ScopeStmtNode *scope = alloc.emplace<ScopeStmtNode>();

    while (!is_type(TokenType::BRACE_CLOSE))
        scope->statements.push_back(*parse_statement());

    consume();

    return scope;
}

StmtNode *Parser::parse_statement()
{
#define emplace(expr) alloc.emplace<StmtNode>(expr)
    switch (peek().type)
    {
    case TokenType::KW_LOCAL:
        if (is_type(TokenType::KW_FUNC, 1))
            return emplace(*parse_function_declaration());
        return emplace(*parse_local_declaration());
    case TokenType::KW_GLOBAL:
        if (is_type(TokenType::KW_FUNC, 1))
            return emplace(*parse_function_declaration());
        return emplace(*parse_global_declaration());
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
    case TokenType::KW_STRUCT:
    case TokenType::KW_NAMESPACE:
        // TODO: Self explanatory...
        goto invalid_statement;
    case TokenType::KW_BREAK:
        return emplace(*alloc.emplace<BreakStmtNode>());
    case TokenType::KW_CONTINUE:
        return emplace(*alloc.emplace<ContinueStmtNode>());
    case TokenType::KW_DO:
        consume();
        return emplace(*parse_scope_statement());
    default:
    {
        ExprNode *expr = parse_expr();

        if (CallExprNode *call = std::get_if<CallExprNode>(expr))
        {
            // Construct a new statement based on the call expression
            // This is bad for several reasons;
            // - It soft-leaks memory
            // - It's unscalable
            // TODO: SO FIND A SOLUTION FFS
            CallStmtNode *stmt = alloc.emplace<CallStmtNode>();
            stmt->callee = call->callee;
            stmt->args = call->args;
            stmt->generics = call->type_args;

            return emplace(*stmt);
        }
        else if (BinaryExprNode *bin_expr = std::get_if<BinaryExprNode>(expr))
        {
            // Only binary expression allowed is an "assignment operation"
            // Which isn't a actually a real thing, it's just a quirk caused by how operators are classified
            // I guess it's beneficial in this case, although it will be HELL to debug in other cases
            // Because it technically cannot evaluate to any value
            if (bin_expr->op.type != TokenType::OP_ASGN)
                goto invalid_statement;

            AssignStmtNode *asgn = alloc.emplace<AssignStmtNode>();
            asgn->target = bin_expr->lhs;
            asgn->value = bin_expr->rhs;

            return emplace(*asgn);
        }

        goto invalid_statement;
    }
    }
invalid_statement:
    VIA_ASSERT(false, "Parser::parse_statement(): Invalid statement");
    return nullptr;
#undef emplace
}

AbstractSyntaxTree *Parser::parse_program()
{
    AbstractSyntaxTree *ast = alloc.emplace<AbstractSyntaxTree>();

    for (Token tok : *program.tokens)
    {
        if (peek().type == TokenType::EOF_)
            break;

        ast->statements.push_back(*parse_statement());
    }

    return ast;
}

} // namespace via
