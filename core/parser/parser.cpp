/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "parser.hpp"
#include "ast/ast.hpp"
#include "diagnostics.hpp"
#include "lexer/token.hpp"
#include "sema/types.hpp"

#define SAVE_FIRST()                                                                     \
    auto* first = advance();                                                             \
    auto loc = m_source.get_location(*first);

#define SAVE_FIRST_DONT_ADVANCE_THO()                                                    \
    auto* first = peek();                                                                \
    auto loc = m_source.get_location(*first);

using enum via::TokenKind;

struct ParserError
{
  public:
    via::Diagnosis diag;

    template <typename... Args>
        requires std::is_constructible_v<via::Diagnosis, via::Level, Args...>
    explicit ParserError(Args&&... args)
        : diag(via::Level::ERROR, args...)
    {}
};

static bool is_expr_start(via::TokenKind kind)
{
    switch (kind) {
    case IDENTIFIER:
    case LIT_INT:
    case LIT_BINT:
    case LIT_XINT:
    case LIT_NIL:
    case LIT_FLOAT:
    case LIT_STRING:
    case KW_NOT:
    case KW_FN:
    case PAREN_OPEN:
    case OP_MINUS:
    case OP_TILDE:
    case OP_AMP:
        return true;
    default:
        return false;
    }
}

static int bin_prec(via::TokenKind kind)
{
    switch (kind) {
    case KW_OR:
        return 0;
    case KW_AND:
        return 1;
    case OP_EQ_EQ:
    case OP_BANG_EQ:
    case OP_LT:
    case OP_LT_EQ:
    case OP_GT:
    case OP_GT_EQ:
        return 2;
    case OP_AMP:
        return 3;
    case OP_CARET:
        return 4;
    case OP_PIPE:
        return 5;
    case OP_SHL:
    case OP_SHR:
        return 6;
    case OP_PLUS:
    case OP_MINUS:
        return 7;
    case OP_STAR:
    case OP_SLASH:
    case OP_PERCENT:
        return 8;
    case OP_STAR_STAR:
        return 9;
    default:
        return -1;
    }
}

const via::Token* via::Parser::peek(int ahead)
{
    return m_cursor[ahead];
}

const via::Token* via::Parser::advance()
{
    return *(m_cursor++);
}

bool via::Parser::match(TokenKind kind, int ahead)
{
    return peek(ahead)->kind == kind;
}

bool via::Parser::optional(TokenKind kind)
{
    if (match(kind)) {
        advance();
        return true;
    }

    return false;
}

const via::Token* via::Parser::expect(TokenKind kind, const char* task)
{
    if (!match(kind)) {
        const Token& unexp = *peek();
        throw ParserError(
            m_source.get_location(unexp),
            std::format(
                "Unexpected token '{}' ({}) while {}",
                unexp.to_string(),
                to_string(unexp.kind),
                task
            )
        );
    }

    return advance();
}

const via::ast::Path* via::Parser::parse_static_path()
{
    auto* sp = m_alloc.emplace<ast::Path>();

    while (true) {
        sp->path.push_back(expect(IDENTIFIER, "parsing static path"));

        if (match(COLON_COLON)) {
            advance();
        } else {
            break;
        }
    }

    sp->loc = {
        m_source.get_location(*sp->path.front()).begin,
        m_source.get_location(*sp->path.front()).end
    };
    return sp;
}

const via::ast::Expr* via::Parser::parse_lvalue()
{
    const ast::Expr* expr = parse_expr();
    if (is_lvalue(expr)) {
        return expr;
    } else {
        throw ParserError(expr->loc, "Unexpected expression while parsing lvalue");
    }
}

const via::ast::Parameter* via::Parser::parse_parameter()
{
    SAVE_FIRST()

    auto* par = m_alloc.emplace<ast::Parameter>();
    par->symbol = first;

    if (optional(COLON)) {
        par->type = parse_type();
        par->loc = {loc.begin, par->type->loc.end};
    } else {
        par->loc = loc;
    }

    return par;
}

const via::ast::Scope* via::Parser::parse_scope()
{
    SAVE_FIRST()

    auto scope = m_alloc.emplace<ast::Scope>();

    if (first->kind == COLON) {
        scope->stmts.push_back(parse_stmt());
        scope->loc = {loc.begin, scope->stmts.back()->loc.end};
    } else if (first->kind == BRACE_OPEN) {
        while (!match(BRACE_CLOSE)) {
            scope->stmts.push_back(parse_stmt());
        }

        auto* last = advance();
        scope->loc = {
            m_source.get_location(*first).begin,
            m_source.get_location(*last).end,
        };
    } else
        throw ParserError(
            loc,
            std::format("Unexpected token '{}' while parsing scope", first->to_string()),
            Footnote(FootnoteKind::HINT, "Expected ':' | '{'")
        );

    return scope;
}

const via::ast::ExprLiteral* via::Parser::parse_expr_literal()
{
    auto* lit = m_alloc.emplace<ast::ExprLiteral>();
    lit->tok = advance();
    lit->loc = m_source.get_location(*lit->tok);
    return lit;
}

const via::ast::ExprSymbol* via::Parser::parse_expr_symbol()
{
    auto* symbol = m_alloc.emplace<ast::ExprSymbol>();
    symbol->symbol = advance();
    symbol->loc = m_source.get_location(*symbol->symbol);
    return symbol;
}

const via::ast::Expr* via::Parser::parse_expr_group_or_tuple()
{
    auto loc = m_source.get_location(*advance());
    auto* first = parse_expr();

    if (match(COMMA)) {
        std::vector<const ast::Expr*> vals;
        vals.push_back(first);

        while (match(COMMA)) {
            advance();
            vals.push_back(parse_expr());
        }

        expect(PAREN_CLOSE, "parsing tuple expression");

        auto* tup = m_alloc.emplace<ast::ExprTuple>();
        tup->values = std::move(vals);
        tup->loc = {loc.begin, m_source.get_location(*peek(-1)).end};
        return reinterpret_cast<const ast::Expr*>(tup);
    }

    expect(PAREN_CLOSE, "parsing grouping expression");

    auto* group = m_alloc.emplace<ast::ExprGroup>();
    group->expr = first;
    group->loc = {loc.begin, m_source.get_location(*peek(-1)).end};
    return reinterpret_cast<const ast::Expr*>(group);
}

const via::ast::ExprUnary* via::Parser::parse_expr_unary(const ast::Expr* expr)
{
    auto* un = m_alloc.emplace<ast::ExprUnary>();
    un->op = advance();
    un->expr = parse_expr_affix();
    un->loc = {m_source.get_location(*un->op).begin, un->expr->loc.end};
    return un;
}

const via::ast::ExprDynAccess* via::Parser::parse_expr_dyn_access(const ast::Expr* expr)
{
    advance(); // consume '.'

    auto* da = m_alloc.emplace<ast::ExprDynAccess>();
    da->root = expr;
    da->index = expect(IDENTIFIER, "parsing dynamic access specifier");
    da->loc = {da->root->loc.begin, m_source.get_location(*da->index).end};
    return da;
}

const via::ast::ExprStaticAccess* via::Parser::parse_expr_st_access(const ast::Expr* expr)
{
    advance(); // consume '::'

    auto* sa = m_alloc.emplace<ast::ExprStaticAccess>();
    sa->root = expr;
    sa->index = expect(IDENTIFIER, "parsing static access specifier");
    sa->loc = {sa->root->loc.begin, m_source.get_location(*sa->index).end};
    return sa;
}

const via::ast::ExprCall* via::Parser::parse_expr_call(const ast::Expr* expr)
{
    advance(); // consume '('

    std::vector<const ast::Expr*> args;

    if (!match(PAREN_CLOSE)) {
        do
            args.push_back(parse_expr());
        while (match(COMMA) && advance());

        expect(PAREN_CLOSE, "parsing function call");
    } else
        advance(); // consume ')'

    auto* call = m_alloc.emplace<ast::ExprCall>();
    call->callee = expr;
    call->args = std::move(args);
    call->loc = {expr->loc.begin, m_source.get_location(*peek(-1)).end};
    return call;
}

const via::ast::ExprSubscript* via::Parser::parse_expr_subscript(const ast::Expr* expr)
{
    advance(); // consume '['

    auto* idx = parse_expr();

    expect(BRACKET_CLOSE, "parsing subscript expression");

    auto* subs = m_alloc.emplace<ast::ExprSubscript>();
    subs->lhs = expr;
    subs->rhs = idx;
    subs->loc = {expr->loc.begin, m_source.get_location(*peek(-1)).end};
    return subs;
}

const via::ast::ExprCast* via::Parser::parse_expr_cast(const ast::Expr* expr)
{
    advance();

    auto* cast = m_alloc.emplace<ast::ExprCast>();
    cast->expr = expr;
    cast->type = parse_type();
    cast->loc = {expr->loc.begin, cast->type->loc.end};
    return cast;
}

const via::ast::ExprTernary* via::Parser::parse_expr_ternary(const ast::Expr* expr)
{
    advance();

    auto* tern = m_alloc.emplace<ast::ExprTernary>();
    tern->lhs = expr;
    tern->cond = parse_expr();

    expect(KW_ELSE, "parsing ternary expression");

    tern->rhs = parse_expr();
    tern->loc = {expr->loc.begin, tern->rhs->loc.end};
    return tern;
}

const via::ast::ExprArray* via::Parser::parse_expr_array()
{
    auto loc = m_source.get_location(*peek());
    auto* arr = m_alloc.emplace<ast::ExprArray>();

    if (!match(BRACKET_CLOSE)) {
        while (true) {
            arr->values.push_back(parse_expr());

            if (match(BRACKET_CLOSE)) {
                optional(COMMA); // trailing comma
                break;
            } else {
                expect(COMMA, "parsing array initializer");
            }
        }
    }

    auto* last = expect(BRACKET_CLOSE, "terminating array initializer");
    arr->loc = {loc.begin, m_source.get_location(*last).end};
    return arr;
}

const via::ast::ExprLambda* via::Parser::parse_expr_lambda()
{
    auto loc = m_source.get_location(*peek());
    auto* fn = m_alloc.emplace<ast::ExprLambda>();

    expect(PAREN_OPEN, "parsing lambda parameter list");

    if (!match(PAREN_CLOSE)) {
        while (true) {
            fn->parms.push_back(parse_parameter());

            if (match(PAREN_CLOSE)) {
                break;
            } else {
                expect(COMMA, "parsing lambda parameter list");
            }
        }

        expect(PAREN_CLOSE, "terminating lambda parameter list");
    }

    fn->body = parse_scope();
    fn->loc = {loc.begin, fn->body->loc.end};
    return fn;
}

const via::ast::Expr* via::Parser::parse_expr_primary()
{
    SAVE_FIRST_DONT_ADVANCE_THO()

    switch (first->kind) {
    // Literal expression
    case LIT_INT:
    case LIT_BINT:
    case LIT_XINT:
    case LIT_NIL:
    case LIT_FLOAT:
    case LIT_TRUE:
    case LIT_FALSE:
    case LIT_STRING:
        return (const ast::Expr*) parse_expr_literal();
    case IDENTIFIER:
        return (const ast::Expr*) parse_expr_symbol();
    case PAREN_OPEN:
        return parse_expr_group_or_tuple();
    case BRACKET_OPEN:
        return (const ast::Expr*) parse_expr_array();
    case KW_FN:
        return (const ast::Expr*) parse_expr_lambda();
    default:
        throw ParserError(
            loc,
            std::format(
                "Unexpected token '{}' ({}) while parsing primary expression",
                first->to_string(),
                to_string(first->kind)
            ),
            Footnote(
                FootnoteKind::HINT,
                "Expected INT | BINARY_INT | HEX_INT | 'nil' | FLOAT | 'true' "
                "| 'false' | "
                "STRING | IDENTIFIER | '(' | ')' | 'fn'"
            )
        );
    }
}

const via::ast::Expr* via::Parser::parse_expr_affix()
{
    const ast::Expr* expr = nullptr;

    switch (peek()->kind) {
    case KW_NOT:
    case OP_MINUS:
    case OP_TILDE:
    case OP_AMP:
        expr = (const ast::Expr*) parse_expr_unary(expr);
        break;
    default:
        expr = parse_expr_primary();
        break;
    }

    while (true) {
        switch (peek()->kind) {
        case KW_AS:
            expr = (const ast::Expr*) parse_expr_cast(expr);
            break;
        case KW_IF:
            expr = (const ast::Expr*) parse_expr_ternary(expr);
            break;
        case PAREN_OPEN:
            expr = (const ast::Expr*) parse_expr_call(expr);
            break;
        case BRACKET_OPEN:
            expr = (const ast::Expr*) parse_expr_subscript(expr);
            break;
        case PERIOD:
            expr = (const ast::Expr*) parse_expr_dyn_access(expr);
            break;
        case COLON_COLON:
            expr = (const ast::Expr*) parse_expr_st_access(expr);
            break;
        default:
            return expr;
        }
    }
}

const via::ast::Expr* via::Parser::parse_expr(int min_prec)
{
    auto* lhs = parse_expr_affix();

    int prec;
    while ((prec = bin_prec(peek()->kind), prec >= min_prec)) {
        auto bin = m_alloc.emplace<ast::ExprBinary>();
        bin->op = advance();
        bin->lhs = lhs;
        bin->rhs = parse_expr(prec + 1);
        bin->loc = {lhs->loc.begin, bin->rhs->loc.end};
        lhs = (const ast::Expr*) bin;
    }

    return lhs;
}

const via::ast::TypeBuiltin* via::Parser::parse_type_builtin()
{
    SAVE_FIRST()

    auto* bt = m_alloc.emplace<ast::TypeBuiltin>();
    bt->token = first;
    bt->loc = loc;
    return bt;
}

const via::ast::TypeArray* via::Parser::parse_type_array()
{
    SAVE_FIRST();

    auto* at = m_alloc.emplace<ast::TypeArray>();
    at->type = parse_type();

    auto* end = expect(BRACKET_CLOSE, "terminating array type");

    at->loc = {loc.begin, m_source.get_location(*end).end};
    return at;
}

const via::ast::TypeMap* via::Parser::parse_type_map()
{
    SAVE_FIRST();

    auto* dt = m_alloc.emplace<ast::TypeMap>();
    dt->key = parse_type();

    expect(COLON, "parsing map type");

    dt->value = parse_type();

    auto* end = expect(BRACE_CLOSE, "terminating map type");

    dt->loc = {m_source.get_location(*first).begin, m_source.get_location(*end).end};
    return dt;
}

const via::ast::TypeFunc* via::Parser::parse_type_function()
{
    SAVE_FIRST()
    expect(PAREN_OPEN, "parsing function type parameter list");

    auto* fn = m_alloc.emplace<ast::TypeFunc>();

    while (!match(PAREN_CLOSE)) {
        fn->parms.push_back(parse_parameter());
        expect(COMMA, "terminating function type parameter");
    }

    expect(ARROW, "parsing function type return type");

    fn->ret = parse_type();
    fn->loc = {loc.begin, fn->ret->loc.end};
    return fn;
}

const via::ast::Type* via::Parser::parse_type_primary()
{
    auto* tok = peek();
    switch (tok->kind) {
    case LIT_NIL:
    case KW_BOOL:
    case KW_INT:
    case KW_FLOAT:
    case KW_STRING:
        return (const ast::Type*) parse_type_builtin();
    case BRACKET_OPEN:
        return (const ast::Type*) parse_type_array();
    case BRACE_OPEN:
        return (const ast::Type*) parse_type_map();
    case KW_FN:
        return (const ast::Type*) parse_type_function();
    default:
        throw ParserError(
            m_source.get_location(*tok),
            std::format(
                "Unexpected token '{}' ({}) while parsing type",
                tok->to_string(),
                to_string(tok->kind)
            ),
            Footnote(
                FootnoteKind::HINT,
                "Expected 'nil' | 'bool' | 'int' | 'float' | "
                "'string' | '[' | '{' | 'fn'"
            )
        );
    }
}

const via::ast::Type* via::Parser::parse_type()
{
    SAVE_FIRST_DONT_ADVANCE_THO();

    TypeQualifier quals = TypeQualifier::NONE;

    while (true) {
        auto* tok = peek();
        switch (tok->kind) {
        case KW_CONST:
            if (quals & TypeQualifier::CONST)
                m_diags.report<Level::WARNING>(
                    m_source.get_location(*tok),
                    "Duplicate 'const' qualifier will be ignored",
                    Footnote(FootnoteKind::SUGGESTION, "Remove 'const'")
                );
            quals |= TypeQualifier::CONST;
            advance();
            break;
        case KW_STRONG:
            if (quals & TypeQualifier::STRONG)
                m_diags.report<Level::WARNING>(
                    m_source.get_location(*tok),
                    "Duplicate 'strong' qualifier will be ignored",
                    Footnote(FootnoteKind::SUGGESTION, "Remove 'strong'")
                );
            quals |= TypeQualifier::STRONG;
            advance();
            break;
        case OP_AMP:
            if (quals & TypeQualifier::REFERENCE)
                throw ParserError(
                    m_source.get_location(*tok),
                    "Nested reference qualifier not allowed",
                    Footnote(FootnoteKind::SUGGESTION, "Remove '&'")
                );
            quals |= TypeQualifier::REFERENCE;
            advance();
            break;
        default:
            goto done;
        }
    }

done:
    auto* primary = const_cast<ast::Type*>(parse_type_primary());
    primary->loc = {loc.begin, primary->loc.end};
    primary->quals = quals;
    return primary;
}

const via::ast::StmtVarDecl* via::Parser::parse_stmt_var_decl(bool semicolon)
{
    SAVE_FIRST()

    auto vars = m_alloc.emplace<ast::StmtVarDecl>();
    vars->decl = first;
    vars->lval = parse_lvalue();

    if (optional(COLON)) {
        vars->type = parse_type();
    } else {
        vars->type = nullptr;
    }

    expect(OP_EQ, "parsing variable declaration");

    vars->rval = parse_expr();
    vars->loc = {loc.begin, vars->rval->loc.end};

    if (semicolon) {
        optional(SEMICOLON);
    }

    return vars;
}

const via::ast::StmtFor* via::Parser::parse_stmt_for()
{
    SAVE_FIRST()

    auto fors = m_alloc.emplace<ast::StmtFor>();
    fors->init = parse_stmt_var_decl(false);

    if (fors->init->decl->kind == KW_CONST) {
        throw ParserError(
            m_source.get_location(*fors->init->decl),
            "'const' variable not allowed in ranged for loop"
        );
    }

    expect(COMMA, "parsing ranged for loop");

    fors->target = parse_expr();

    if (match(COMMA)) {
        advance();
        fors->step = parse_expr();
    }

    fors->body = parse_scope();
    fors->loc = {loc.begin, fors->body->loc.end};
    return fors;
}

const via::ast::StmtForEach* via::Parser::parse_stmt_for_each()
{
    SAVE_FIRST()

    auto fors = m_alloc.emplace<ast::StmtForEach>();
    fors->name = parse_lvalue();

    expect(KW_IN, "parsing for each statement");

    fors->expr = parse_expr();
    fors->body = parse_scope();
    fors->loc = {loc.begin, fors->body->loc.end};
    return fors;
}

const via::ast::StmtIf* via::Parser::parse_stmt_if()
{
    using Branch = ast::StmtIf::Branch;

    SAVE_FIRST()

    Branch br;
    br.cond = parse_expr();
    br.body = parse_scope();

    auto* ifs = m_alloc.emplace<ast::StmtIf>();
    ifs->branches.push_back(br);

    while (match(KW_ELSE)) {
        advance();

        Branch br;

        if (match(KW_IF)) {
            advance();
            br.cond = parse_expr();
        } else {
            br.cond = nullptr;
        }

        br.body = parse_scope();
        ifs->branches.push_back(br);
    }

    ifs->loc = {loc.begin, br.body->loc.end};
    return ifs;
}

const via::ast::StmtWhile* via::Parser::parse_stmt_while()
{
    SAVE_FIRST()

    auto* whs = m_alloc.emplace<ast::StmtWhile>();
    whs->cond = parse_expr();
    whs->body = parse_scope();
    whs->loc = {loc.begin, whs->body->loc.end};
    return whs;
}

const via::ast::StmtAssign* via::Parser::parse_stmt_assign(const ast::Expr* expr)
{
    auto as = m_alloc.emplace<ast::StmtAssign>();
    as->lval = expr;
    as->op = advance();
    as->rval = parse_expr();
    as->loc = {as->lval->loc.begin, as->rval->loc.end};
    optional(SEMICOLON);
    return as;
}

const via::ast::StmtReturn* via::Parser::parse_stmt_return()
{
    SAVE_FIRST()

    auto* ret = m_alloc.emplace<ast::StmtReturn>();

    if (is_expr_start(peek()->kind)) {
        ret->expr = parse_expr();
        ret->loc = {loc.begin, ret->expr->loc.end};
    } else {
        ret->expr = nullptr;
        ret->loc = loc;
    }

    optional(SEMICOLON);
    return ret;
}

const via::ast::StmtEnum* via::Parser::parse_stmt_enum()
{
    SAVE_FIRST()

    auto ens = m_alloc.emplace<ast::StmtEnum>();
    ens->symbol = advance();

    if (optional(KW_OF)) {
        ens->type = parse_type();
    }

    expect(BRACE_OPEN, "parsing enumerator list");

    while (!match(BRACE_CLOSE)) {
        auto* symbol = advance();
        expect(OP_EQ, "parsing enumerator pair");

        ens->pairs.push_back({
            .symbol = symbol,
            .expr = parse_expr(),
        });

        expect(COMMA, "parsing enumerator pair");
    }

    ens->loc = {loc.begin, m_source.get_location(*advance()).end};
    return ens;
}

const via::ast::StmtImport* via::Parser::parse_stmt_import()
{
    SAVE_FIRST()

    size_t end;
    auto imp = m_alloc.emplace<ast::StmtImport>();

    while (true) {
        auto* tok = expect(IDENTIFIER, "parsing import path");
        imp->path.push_back(tok);

        if (match(COLON_COLON)) {
            advance();
        } else {
            end = m_source.get_location(*tok).end;
            break;
        }
    }

    imp->loc = {loc.begin, end};
    optional(SEMICOLON);
    return imp;
}

const via::ast::StmtFunctionDecl* via::Parser::parse_stmt_func_decl()
{
    SAVE_FIRST()

    auto* fn = m_alloc.emplace<ast::StmtFunctionDecl>();
    fn->name = expect(IDENTIFIER, "parsing function name");

    expect(PAREN_OPEN, "parsing function parameter list");

    while (!match(PAREN_CLOSE)) {
        fn->parms.push_back(parse_parameter());

        if (match(PAREN_CLOSE)) {
            optional(COMMA);
            break;
        } else {
            expect(COMMA, "terminating function parameter");
        }
    }

    expect(PAREN_CLOSE, "terminating function parameter list");

    if (optional(ARROW)) {
        fn->ret = parse_type();
    } else {
        fn->ret = nullptr;
    }

    fn->body = parse_scope();
    fn->loc = {loc.begin, fn->body->loc.end};
    return fn;
}

const via::ast::StmtStructDecl* via::Parser::parse_stmt_struct_decl()
{
    SAVE_FIRST()

    auto* strc = m_alloc.emplace<ast::StmtStructDecl>();
    strc->name = expect(IDENTIFIER, "parsing struct name");
    strc->body = parse_scope();
    strc->loc = {loc.begin, strc->loc.end};
    return strc;
}

const via::ast::StmtTypeDecl* via::Parser::parse_stmt_type_decl()
{
    SAVE_FIRST()

    auto* ty = m_alloc.emplace<ast::StmtTypeDecl>();
    ty->symbol = advance();

    expect(OP_EQ, "parsing type declaration");

    ty->type = parse_type();
    ty->loc = {loc.begin, ty->type->loc.end};

    optional(SEMICOLON);
    return ty;
}

const via::ast::Stmt* via::Parser::parse_stmt()
{
    switch (peek()->kind) {
    case KW_IF:
        return (const ast::Stmt*) parse_stmt_if();
    case KW_WHILE:
        return (const ast::Stmt*) parse_stmt_while();
    case KW_VAR:
    case KW_CONST:
        return (const ast::Stmt*) parse_stmt_var_decl(true);
    case KW_DO: {
        SAVE_FIRST();
        auto* scope = m_alloc.emplace<ast::StmtScope>();
        scope->body = parse_scope();
        scope->loc = {loc.begin, scope->body->loc.end};
        return (const ast::Stmt*) scope;
    }
    case KW_FOR:
        if (match(KW_VAR, 1)) // generic for loop
            return (const ast::Stmt*) parse_stmt_for();
        return (const ast::Stmt*) parse_stmt_for_each();
    case KW_RETURN:
        return (const ast::Stmt*) parse_stmt_return();
    case KW_ENUM:
        return (const ast::Stmt*) parse_stmt_enum();
    case KW_IMPORT:
        return (const ast::Stmt*) parse_stmt_import();
    case KW_FN:
        return (const ast::Stmt*) parse_stmt_func_decl();
    case KW_STRUCT:
        return (const ast::Stmt*) parse_stmt_struct_decl();
    case KW_TYPE:
        return (const ast::Stmt*) parse_stmt_type_decl();
    case SEMICOLON: {
        auto empty = m_alloc.emplace<ast::StmtEmpty>();
        empty->loc = m_source.get_location(*advance());
        return (const ast::Stmt*) empty;
    }
    default:
        break;
    }

    const Token* first = peek();
    if (!is_expr_start(first->kind)) {
unexpected_token:
        throw ParserError(
            m_source.get_location(*first),
            std::format(
                "Unexpected token '{}' ({}) while parsing statement",
                first->to_string(),
                to_string(first->kind)
            )
        );
    }

    auto* expr = parse_expr();

    switch (peek()->kind) {
    case OP_EQ:
    case OP_PLUS_EQ:
    case OP_MINUS_EQ:
    case OP_STAR_EQ:
    case OP_SLASH_EQ:
    case OP_STAR_STAR_EQ:
    case OP_PERCENT_EQ:
    case OP_PIPE_EQ:
    case OP_AMP_EQ:
        return (const ast::Stmt*) parse_stmt_assign(expr);
    default: {
        auto empty = m_alloc.emplace<ast::StmtExpr>();
        empty->expr = expr;
        empty->loc = empty->expr->loc;

        if TRY_COERCE (const ast::ExprCall, _, expr) {
            goto valid_expr_stmt;
        } else {
            goto unexpected_token;
        }

valid_expr_stmt:
        optional(SEMICOLON);
        return (const ast::Stmt*) empty;
    }
    }
}

via::SyntaxTree via::Parser::parse()
{
    SyntaxTree nodes;
    while (!match(EOF_)) {
        try {
            auto* stmt = parse_stmt();
            nodes.push_back(stmt);
        } catch (const ParserError& e) {
            m_diags.report(e.diag);
            break;
        }
    }
    return nodes;
}
