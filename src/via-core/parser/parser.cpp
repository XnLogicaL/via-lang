/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "parser.h"
#include <magic_enum/magic_enum.hpp>

#define SAVE_FIRST()                                                                     \
    auto* first = advance();                                                             \
    auto loc = first->location(m_source);

#define SAVE_FIRST_DONT_ADVANCE_THO()                                                    \
    auto* first = peek();                                                                \
    auto loc = first->location(m_source);

using enum via::TokenKind;
using namespace via::ast;

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
            unexp.location(m_source),
            std::format(
                "Unexpected token '{}' ({}) while {}",
                unexp.to_string(),
                magic_enum::enum_name(unexp.kind),
                task
            )
        );
    }

    return advance();
}

const Path* via::Parser::parse_static_path()
{
    auto* sp = m_alloc.emplace<Path>();

    while (true) {
        sp->path.push_back(expect(IDENTIFIER, "parsing static path"));

        if (match(COLON_COLON)) {
            advance();
        }
        else {
            break;
        }
    }

    sp->loc = {
        sp->path.front()->location(m_source).begin,
        sp->path.back()->location(m_source).end
    };
    return sp;
}

const Expr* via::Parser::parse_lvalue()
{
    const Expr* expr = parse_expr();
    if (isLValue(expr)) {
        return expr;
    }
    else {
        throw ParserError(expr->loc, "Unexpected expression while parsing lvalue");
    }
}

const Parameter* via::Parser::parse_parameter()
{
    SAVE_FIRST()

    auto* par = m_alloc.emplace<Parameter>();
    par->symbol = first;

    if (optional(COLON)) {
        par->type = parse_type();
        par->loc = {loc.begin, par->type->loc.end};
    }
    else {
        par->loc = loc;
    }

    return par;
}

const AttributeGroup* via::Parser::parse_attributes()
{
    SAVE_FIRST()
    expect(BRACKET_OPEN, "parsing attribute group");

    auto* atg = m_alloc.emplace<AttributeGroup>();

    while (true) {
        atg->ats.push_back({
            .sp = parse_static_path(),
            .args = {},
        });

        if (match(BRACKET_CLOSE)) {
            break;
        }
        else {
            expect(COMMA, "parsing attribute group");
        }
    }

    auto* last = expect(BRACKET_CLOSE, "terminating attribute group");
    atg->loc = {loc.begin, last->location(m_source).end};
    return atg;
}

const ExprLiteral* via::Parser::parse_expr_literal()
{
    auto* lit = m_alloc.emplace<ExprLiteral>();
    lit->tok = advance();
    lit->loc = lit->tok->location(m_source);
    return lit;
}

const ExprSymbol* via::Parser::parse_expr_symbol()
{
    auto* symbol = m_alloc.emplace<ExprSymbol>();
    symbol->symbol = advance();
    symbol->loc = symbol->symbol->location(m_source);
    return symbol;
}

const Expr* via::Parser::parse_expr_group_or_tuple()
{
    auto loc = advance()->location(m_source);
    auto* first = parse_expr();

    if (match(COMMA)) {
        std::vector<const Expr*> vals;
        vals.push_back(first);

        while (match(COMMA)) {
            advance();
            vals.push_back(parse_expr());
        }

        expect(PAREN_CLOSE, "parsing tuple expression");

        auto* tup = m_alloc.emplace<ExprTuple>();
        tup->vals = std::move(vals);
        tup->loc = {loc.begin, peek(-1)->location(m_source).end};

        return tup;
    }

    expect(PAREN_CLOSE, "parsing grouping expression");

    auto* group = m_alloc.emplace<ExprGroup>();
    group->expr = first;
    group->loc = {loc.begin, peek(-1)->location(m_source).end};
    return group;
}

const ExprUnary* via::Parser::parse_expr_unary(const ast::Expr* expr)
{
    auto* un = m_alloc.emplace<ExprUnary>();
    un->op = advance();
    un->expr = parse_expr_affix();
    un->loc = {un->op->location(m_source).begin, un->expr->loc.end};
    return un;
}

const ExprDynAccess* via::Parser::parse_expr_dyn_access(const ast::Expr* expr)
{
    advance(); // consume '.'

    auto* da = m_alloc.emplace<ExprDynAccess>();
    da->root = expr;
    da->index = expect(IDENTIFIER, "parsing dynamic access specifier");
    da->loc = {da->root->loc.begin, da->index->location(m_source).end};
    return da;
}

const ExprStaticAccess* via::Parser::parse_expr_st_access(const ast::Expr* expr)
{
    advance(); // consume '::'

    auto* sa = m_alloc.emplace<ExprStaticAccess>();
    sa->root = expr;
    sa->index = expect(IDENTIFIER, "parsing static access specifier");
    sa->loc = {sa->root->loc.begin, sa->index->location(m_source).end};
    return sa;
}

const ExprCall* via::Parser::parse_expr_call(const ast::Expr* expr)
{
    advance(); // consume '('

    std::vector<const Expr*> args;

    if (!match(PAREN_CLOSE)) {
        do
            args.push_back(parse_expr());
        while (match(COMMA) && advance());

        expect(PAREN_CLOSE, "parsing function call");
    }
    else
        advance(); // consume ')'

    auto* call = m_alloc.emplace<ExprCall>();
    call->lval = expr;
    call->args = std::move(args);
    call->loc = {expr->loc.begin, peek(-1)->location(m_source).end};
    return call;
}

const ExprSubscript* via::Parser::parse_expr_subscript(const ast::Expr* expr)
{
    advance(); // consume '['

    const Expr* idx = parse_expr();

    expect(BRACKET_CLOSE, "parsing subscript expression");

    auto* subs = m_alloc.emplace<ExprSubscript>();
    subs->lval = expr;
    subs->idx = idx;
    subs->loc = {expr->loc.begin, peek(-1)->location(m_source).end};
    return subs;
}

const ExprCast* via::Parser::parse_expr_cast(const ast::Expr* expr)
{
    advance();

    auto* cast = m_alloc.emplace<ExprCast>();
    cast->expr = expr;
    cast->type = parse_type();
    cast->loc = {expr->loc.begin, cast->type->loc.end};
    return cast;
}

const ExprTernary* via::Parser::parse_expr_ternary(const ast::Expr* expr)
{
    advance();

    auto* tern = m_alloc.emplace<ExprTernary>();
    tern->lhs = expr;
    tern->cnd = parse_expr();

    expect(KW_ELSE, "parsing ternary expression");

    tern->rhs = parse_expr();
    tern->loc = {expr->loc.begin, tern->rhs->loc.end};
    return tern;
}

const ExprArray* via::Parser::parse_expr_array()
{
    auto loc = peek()->location(m_source);
    auto* arr = m_alloc.emplace<ExprArray>();

    if (!match(BRACKET_CLOSE)) {
        while (true) {
            arr->init.push_back(parse_expr());

            if (match(BRACKET_CLOSE)) {
                optional(COMMA); // trailing comma
                break;
            }
            else {
                expect(COMMA, "parsing array initializer");
            }
        }
    }

    auto* last = expect(BRACKET_CLOSE, "terminating array initializer");
    arr->loc = {loc.begin, last->location(m_source).end};
    return arr;
}

const ExprLambda* via::Parser::parse_expr_lambda()
{
    auto loc = peek()->location(m_source);
    auto* fn = m_alloc.emplace<ExprLambda>();

    expect(PAREN_OPEN, "parsing lambda parameter list");

    if (!match(PAREN_CLOSE)) {
        while (true) {
            fn->pms.push_back(parse_parameter());

            if (match(PAREN_CLOSE)) {
                break;
            }
            else {
                expect(COMMA, "parsing lambda parameter list");
            }
        }

        expect(PAREN_CLOSE, "terminating lambda parameter list");
    }

    fn->scope = parse_stmt_scope();
    fn->loc = {loc.begin, fn->scope->loc.end};
    return fn;
}

const Expr* via::Parser::parse_expr_primary()
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
        return parse_expr_literal();
    case IDENTIFIER:
        return parse_expr_symbol();
    case PAREN_OPEN:
        return parse_expr_group_or_tuple();
    case BRACKET_OPEN:
        return parse_expr_array();
    case KW_FN:
        return parse_expr_lambda();
    default:
        throw ParserError(
            loc,
            std::format(
                "Unexpected token '{}' ({}) while parsing primary expression",
                first->to_string(),
                magic_enum::enum_name(first->kind)
            ),
            Footnote(
                Footnote::Kind::HINT,
                "Expected INT | BINARY_INT | HEX_INT | 'nil' | FLOAT | 'true' "
                "| 'false' | "
                "STRING | IDENTIFIER | '(' | ')' | 'fn'"
            )
        );
    }
}

const Expr* via::Parser::parse_expr_affix()
{
    const Expr* expr;

    switch (peek()->kind) {
    case KW_NOT:
    case OP_MINUS:
    case OP_TILDE:
    case OP_AMP:
        expr = parse_expr_unary(expr);
        break;
    default:
        expr = parse_expr_primary();
        break;
    }

    while (true) {
        switch (peek()->kind) {
        case KW_AS:
            expr = parse_expr_cast(expr);
            break;
        case KW_IF:
            expr = parse_expr_ternary(expr);
            break;
        case PAREN_OPEN:
            expr = parse_expr_call(expr);
            break;
        case BRACKET_OPEN:
            expr = parse_expr_subscript(expr);
            break;
        case PERIOD:
            expr = parse_expr_dyn_access(expr);
            break;
        case COLON_COLON:
            expr = parse_expr_st_access(expr);
            break;
        default:
            return expr;
        }
    }
}

const Expr* via::Parser::parse_expr(int minPrec)
{
    const Expr* lhs = parse_expr_affix();

    int prec;
    while ((prec = bin_prec(peek()->kind), prec >= minPrec)) {
        auto bin = m_alloc.emplace<ExprBinary>();
        bin->op = advance();
        bin->lhs = lhs;
        bin->rhs = parse_expr(prec + 1);
        bin->loc = {lhs->loc.begin, bin->rhs->loc.end};
        lhs = bin;
    }

    return lhs;
}

const TypeBuiltin* via::Parser::parse_type_builtin()
{
    SAVE_FIRST()

    auto* bt = m_alloc.emplace<TypeBuiltin>();
    bt->tok = first;
    bt->loc = loc;
    return bt;
}

const TypeArray* via::Parser::parse_type_array()
{
    SAVE_FIRST();

    auto* at = m_alloc.emplace<TypeArray>();
    at->type = parse_type();

    auto* end = expect(BRACKET_CLOSE, "terminating array type");

    at->loc = {loc.begin, end->location(m_source).end};
    return at;
}

const TypeDict* via::Parser::parse_type_dict()
{
    SAVE_FIRST();

    auto* dt = m_alloc.emplace<TypeDict>();
    dt->key = parse_type();

    expect(COLON, "parsing dictionary type");

    dt->val = parse_type();

    auto* end = expect(BRACE_CLOSE, "terminating dictionary type");

    dt->loc = {first->location(m_source).begin, end->location(m_source).end};
    return dt;
}

const TypeFunc* via::Parser::parse_type_function()
{
    SAVE_FIRST()
    expect(PAREN_OPEN, "parsing function type parameter list");

    auto* fn = m_alloc.emplace<TypeFunc>();

    while (!match(PAREN_CLOSE)) {
        fn->params.push_back(parse_parameter());
        expect(COMMA, "terminating function type parameter");
    }

    expect(ARROW, "parsing function type return type");

    fn->ret = parse_type();
    fn->loc = {loc.begin, fn->ret->loc.end};
    return fn;
}

const Type* via::Parser::parse_type()
{
    auto* tok = peek();
    switch (tok->kind) {
    case LIT_NIL:
    case KW_BOOL:
    case KW_INT:
    case KW_FLOAT:
    case KW_STRING:
        return parse_type_builtin();
    case BRACKET_OPEN:
        return parse_type_array();
    case BRACE_OPEN:
        return parse_type_dict();
    case KW_FN:
        return parse_type_function();
    default:
        throw ParserError(
            tok->location(m_source),
            std::format(
                "Unexpected token '{}' ({}) while parsing type",
                tok->to_string(),
                magic_enum::enum_name(tok->kind)
            ),
            Footnote(
                Footnote::Kind::HINT,
                "Expected 'nil' | 'bool' | 'int' | 'float' | "
                "'string' | '[' | '{' | 'fn'"
            )
        );
    }
}

const StmtScope* via::Parser::parse_stmt_scope()
{
    SAVE_FIRST()

    auto scope = m_alloc.emplace<StmtScope>();

    if (first->kind == COLON) {
        scope->stmts.push_back(parse_stmt());
        scope->loc = {loc.begin, scope->stmts.back()->loc.end};
    }
    else if (first->kind == BRACE_OPEN) {
        while (!match(BRACE_CLOSE)) {
            scope->stmts.push_back(parse_stmt());
        }

        auto* last = advance();
        scope->loc = {
            first->location(m_source).begin,
            last->location(m_source).end,
        };
    }
    else
        throw ParserError(
            loc,
            std::format("Unexpected token '{}' while parsing scope", first->to_string()),
            Footnote(Footnote::Kind::HINT, "Expected ':' | '{'")
        );

    return scope;
}

const StmtVarDecl* via::Parser::parse_stmt_var_decl(bool semicolon)
{
    SAVE_FIRST()

    auto vars = m_alloc.emplace<StmtVarDecl>();
    vars->decl = first;
    vars->lval = parse_lvalue();

    if (optional(COLON)) {
        vars->type = parse_type();
    }
    else {
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

const StmtFor* via::Parser::parse_stmt_for()
{
    SAVE_FIRST()

    auto fors = m_alloc.emplace<StmtFor>();
    fors->init = parse_stmt_var_decl(false);

    if (fors->init->decl->kind == KW_CONST) {
        throw ParserError(
            fors->init->decl->location(m_source),
            "'const' variable not allowed in ranged for loop"
        );
    }

    expect(COMMA, "parsing ranged for loop");

    fors->target = parse_expr();

    if (match(COMMA)) {
        advance();
        fors->step = parse_expr();
    }

    fors->br = parse_stmt_scope();
    fors->loc = {loc.begin, fors->br->loc.end};
    return fors;
}

const StmtForEach* via::Parser::parse_stmt_for_each()
{
    SAVE_FIRST()

    auto fors = m_alloc.emplace<StmtForEach>();
    fors->lval = parse_lvalue();

    expect(KW_IN, "parsing for each statement");

    fors->iter = parse_expr();
    fors->br = parse_stmt_scope();
    fors->loc = {loc.begin, fors->br->loc.end};
    return fors;
}

const StmtIf* via::Parser::parse_stmt_if()
{
    using Branch = StmtIf::Branch;

    SAVE_FIRST()

    Branch br;
    br.cnd = parse_expr();
    br.br = parse_stmt_scope();

    auto* ifs = m_alloc.emplace<StmtIf>();
    ifs->brs.push_back(br);

    while (match(KW_ELSE)) {
        advance();

        Branch br;

        if (match(KW_IF)) {
            advance();
            br.cnd = parse_expr();
        }
        else {
            br.cnd = nullptr;
        }

        br.br = parse_stmt_scope();
        ifs->brs.push_back(br);
    }

    ifs->loc = {loc.begin, br.br->loc.end};
    return ifs;
}

const StmtWhile* via::Parser::parse_stmt_while()
{
    SAVE_FIRST()

    auto* whs = m_alloc.emplace<StmtWhile>();
    whs->cnd = parse_expr();
    whs->br = parse_stmt_scope();
    whs->loc = {loc.begin, whs->br->loc.end};
    return whs;
}

const StmtAssign* via::Parser::parse_stmt_assign(const Expr* expr)
{
    auto as = m_alloc.emplace<StmtAssign>();
    as->lval = expr;
    as->op = advance();
    as->rval = parse_expr();
    as->loc = {as->lval->loc.begin, as->rval->loc.end};
    optional(SEMICOLON);
    return as;
}

const StmtReturn* via::Parser::parse_stmt_return()
{
    SAVE_FIRST()

    auto* ret = m_alloc.emplace<StmtReturn>();

    if (is_expr_start(peek()->kind)) {
        ret->expr = parse_expr();
        ret->loc = {loc.begin, ret->expr->loc.end};
    }
    else {
        ret->expr = nullptr;
        ret->loc = loc;
    }

    optional(SEMICOLON);
    return ret;
}

const StmtEnum* via::Parser::parse_stmt_enum()
{
    SAVE_FIRST()

    auto ens = m_alloc.emplace<StmtEnum>();
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

    ens->loc = {loc.begin, advance()->location(m_source).end};
    return ens;
}

const StmtModule* via::Parser::parse_stmt_module()
{
    SAVE_FIRST()

    auto* mod = m_alloc.emplace<StmtModule>();
    mod->symbol = advance();

    expect(BRACE_OPEN, "parsing module body");

    if (!match(BRACE_CLOSE)) {
        while (true) {
            auto* tok = peek();
            switch (tok->kind) {
            case KW_CONST:
            case KW_VAR:
                mod->scope.push_back(parse_stmt_var_decl(true));
                break;
            case KW_FN:
                mod->scope.push_back(parse_stmt_func_decl());
                break;
            case KW_STRUCT:
                mod->scope.push_back(parse_stmt_struct_decl());
                break;
            case KW_TYPE:
                mod->scope.push_back(parse_stmt_type_decl());
                break;
            case KW_MODULE:
                mod->scope.push_back(parse_stmt_module());
                break;
            case KW_USING:
                mod->scope.push_back(parse_stmt_using_decl());
                break;
            case KW_ENUM:
                mod->scope.push_back(parse_stmt_enum());
                break;
            default:
                throw ParserError(
                    tok->location(m_source),
                    std::format(
                        "Unexpected token '{}' ({}) while parsing module",
                        tok->to_string(),
                        magic_enum::enum_name(tok->kind)
                    )
                );
            }
        }
    }

    auto* last = expect(BRACE_CLOSE, "terminating module body");
    mod->loc = {loc.begin, last->location(m_source).end};
    return mod;
}

const StmtImport* via::Parser::parse_stmt_import()
{
    using TailKind = StmtImport::TailKind;

    SAVE_FIRST()

    usize end;
    auto imp = m_alloc.emplace<StmtImport>();
    imp->kind = TailKind::IMPORT;

    while (true) {
        auto* tok = advance();

        if (tok->kind == IDENTIFIER) {
            imp->path.push_back(tok);

            if (match(COLON_COLON)) {
                advance();
            }
            else {
                end = tok->location(m_source).end;
                break;
            }
        }
        else if (tok->kind == BRACE_OPEN) {
            imp->kind = TailKind::IMPORT_COMPOUND;

            while (!match(BRACE_CLOSE)) {
                auto* member = expect(IDENTIFIER, "parsing compound import member");
                imp->tail.push_back(member);
                expect(COMMA, "parsing compound import");
            }

            auto* last = expect(BRACE_CLOSE, "terminating compound import");
            end = last->location(m_source).end;
            break;
        }
        else if (tok->kind == OP_STAR) {
            imp->kind = TailKind::IMPORT_ALL;
            end = tok->location(m_source).end;
            break;
        }
        else {
            throw ParserError(
                tok->location(m_source),
                std::format(
                    "Unexpected token '{}' ({}) while parsing import path",
                    tok->to_string(),
                    magic_enum::enum_name(tok->kind)
                )
            );
        }
    }

    imp->loc = {loc.begin, end};
    optional(SEMICOLON);
    return imp;
}

const StmtFunctionDecl* via::Parser::parse_stmt_func_decl()
{
    SAVE_FIRST()

    auto* fn = m_alloc.emplace<StmtFunctionDecl>();
    fn->name = expect(IDENTIFIER, "parsing function name");

    expect(PAREN_OPEN, "parsing function parameter list");

    while (!match(PAREN_CLOSE)) {
        fn->parms.push_back(parse_parameter());

        if (match(PAREN_CLOSE)) {
            optional(COMMA);
            break;
        }
        else {
            expect(COMMA, "terminating function parameter");
        }
    }

    expect(PAREN_CLOSE, "terminating function parameter list");

    if (optional(ARROW)) {
        fn->ret = parse_type();
    }
    else {
        fn->ret = nullptr;
    }

    fn->scope = parse_stmt_scope();
    fn->loc = {loc.begin, fn->scope->loc.end};
    return fn;
}

const StmtStructDecl* via::Parser::parse_stmt_struct_decl()
{
    SAVE_FIRST()

    auto* strc = m_alloc.emplace<StmtStructDecl>();
    strc->name = expect(IDENTIFIER, "parsing struct name");

    expect(BRACE_OPEN, "parsing struct body");

    while (!match(BRACE_CLOSE)) {
        auto* tok = peek();
        switch (tok->kind) {
        case KW_CONST:
        case KW_VAR:
            strc->scope.push_back(parse_stmt_var_decl(false));
            expect(COMMA, "terminating struct member");
            break;
        case KW_FN:
            strc->scope.push_back(parse_stmt_func_decl());
            break;
        case KW_TYPE:
            strc->scope.push_back(parse_stmt_type_decl());
            expect(COMMA, "terminating struct member");
            break;
        case KW_USING:
            strc->scope.push_back(parse_stmt_using_decl());
            break;
        case KW_ENUM:
            strc->scope.push_back(parse_stmt_enum());
            break;
        default:
            throw ParserError(
                tok->location(m_source),
                std::format(
                    "Unexpected token '{}' ({}) while parsing struct body",
                    tok->to_string(),
                    magic_enum::enum_name(tok->kind)
                )
            );
        }
    }

    auto* last = expect(BRACE_CLOSE, "terminating struct body");
    strc->loc = {loc.begin, last->location(m_source).end};
    return strc;
}

const StmtTypeDecl* via::Parser::parse_stmt_type_decl()
{
    SAVE_FIRST()

    auto* ty = m_alloc.emplace<StmtTypeDecl>();
    ty->symbol = advance();

    expect(OP_EQ, "parsing type declaration");

    ty->type = parse_type();
    ty->loc = {loc.begin, ty->type->loc.end};

    optional(SEMICOLON);
    return ty;
}

const StmtUsing* via::Parser::parse_stmt_using_decl()
{
    SAVE_FIRST();

    auto* usn = m_alloc.emplace<StmtUsing>();
    usn->sp = parse_static_path();
    usn->scope = parse_stmt_scope();
    usn->loc = {loc.begin, usn->scope->loc.end};
    return usn;
}

const Stmt* via::Parser::parse_stmt()
{
    switch (peek()->kind) {
    case KW_IF:
        return parse_stmt_if();
    case KW_WHILE:
        return parse_stmt_while();
    case KW_VAR:
    case KW_CONST:
        return parse_stmt_var_decl(true);
    case KW_DO:
        advance();
        return parse_stmt_scope();
    case KW_FOR:
        // generic for loop
        if (match(KW_VAR, 1))
            return parse_stmt_for();

        // for each loop
        return parse_stmt_for_each();
    case KW_RETURN:
        return parse_stmt_return();
    case KW_ENUM:
        return parse_stmt_enum();
    case KW_MODULE:
        return parse_stmt_module();
    case KW_IMPORT:
        return parse_stmt_import();
    case KW_FN:
        return parse_stmt_func_decl();
    case KW_STRUCT:
        return parse_stmt_struct_decl();
    case KW_TYPE:
        return parse_stmt_type_decl();
    case KW_USING:
        return parse_stmt_using_decl();
    case SEMICOLON: {
        auto empty = m_alloc.emplace<StmtEmpty>();
        empty->loc = advance()->location(m_source);
        return empty;
    }
    default:
        break;
    }

    const Token* first = peek();
    if (!is_expr_start(first->kind)) {
unexpected_token:
        throw ParserError(
            first->location(m_source),
            std::format(
                "Unexpected token '{}' ({}) while parsing statement",
                first->to_string(),
                magic_enum::enum_name(first->kind)
            )
        );
    }

    const Expr* expr = parse_expr();

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
        return parse_stmt_assign(expr);
    default: {
        auto es = m_alloc.emplace<StmtExpr>();
        es->expr = expr;
        es->loc = es->expr->loc;

        if TRY_COERCE (const ExprCall, _, expr) {
            goto valid_expr_stmt;
        }
        else {
            goto unexpected_token;
        }

valid_expr_stmt:
        optional(SEMICOLON);
        return es;
    }
    }
}

via::SyntaxTree via::Parser::parse()
{
    SyntaxTree nodes;

    while (!match(EOF_)) {
        try {
            nodes.push_back(parse_stmt());
        }
        catch (const ParserError& e) {
            m_diags.report(e.diag);
            break;
        }
    }

    return nodes;
}
