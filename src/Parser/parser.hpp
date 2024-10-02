#ifndef via_parser_hpp
#define via_parser_hpp

#include "common.hpp"

#include "Lexer/token.hpp"

#include "AST/Stmt.hpp"
#include "AST/Expr.hpp"
#include "AST/Header.hpp"

typedef std::vector<Stmt> AST;

const auto FUNC_KW = "fn";
const auto DECL_KW = "local";
const auto SCOPE_KW = "do";
const auto CONST_SPEC = TokenType::EXCLAM;
const auto INDEX_OP = TokenType::SEMICOL;
const auto CLASS_CONSTRUCTOR_IDENT = "new";
const auto CLASS_DESTRUCTOR_IDENT = "destroy";

class Parser
{
public:
    Parser(
        std::vector<Token> toks,
        std::string fname,
        std::vector<std::string> flines)

        : toks(std::move(toks))
        , pos(0)
        , ast(nullptr)
        , fname(fname)
        , flines(flines) {}

    void generate_ast()
    {
        auto ast = new AST();



        this->ast = ast;
    }

private:

    Stmt parse_stmt()
    {
        if (check_keyword(DECL_KW))
            return parse_decl();
        else if (check_keyword(FUNC_KW))
            return parse_fun_decl();
        else if (check_type(TokenType::IDENTIFIER) && check_type(TokenType::LPAR))
            return parse_fun_call();
        else if (check_type(TokenType::IDENTIFIER) && check_type(TokenType::ASSIGN))
            return parse_assign_stmt();
        else if (check_keyword("if"))
            return parse_if_stmt();
        else if (check_keyword("while"))
            return parse_while_stmt();
        else if (check_keyword("for"))
            return parse_for_stmt();
        else if (check_keyword("module"))
            return parse_module_decl_stmt();
        else if (check_keyword("export"))
            return parse_module_export_stmt();
        else if (check_keyword("using"))
            return parse_module_import_stmt();
        else if (check_keyword("class"))
            return parse_class_decl_stmt();
        else if (check_type(TokenType::IDENTIFIER) && (check_type(INDEX_OP) || check_type(TokenType::LSQU)))
            return std::visit([](auto&& arg) -> Stmt { return arg; }, parse_index_stmt());
        else if (check_keyword("continue"))
        {
            consume(); return ContinueStmt();
        }
        else if (check_keyword("break"))
        {
            consume(); return BreakStmt();
        }
        else if (check_keyword("return"))
        {
            consume(); return ReturnStmt{ parse_expr() };
        }
        else if (check_keyword(SCOPE_KW))
        {
            consume(); return parse_scope();
        }
    }

    ModuleDeclStmt parse_module_decl_stmt()
    {
        if (!fname.ends_with(".viam"))
            CompilerOut::error(
                get_finfo_header() + "Cannot declare module in script file\n  Incorrect extension; expected .viam, got .via"
            );

        expect_keyword("module");

        auto module_decl_stmt = new ModuleDeclStmt();
        module_decl_stmt->ident = expect_type(TokenType::IDENTIFIER);

        return *module_decl_stmt;
    }

    ExportStmt parse_module_export_stmt()
    {
        if (!fname.ends_with(".viam"))
            CompilerOut::error(
                get_finfo_header() + "Cannot export module in script file\n  Incorrect extension; expected .viam, got .via"
            );

        expect_keyword("export");

        auto export_stmt = new ExportStmt();
        export_stmt->ident = expect_type(TokenType::IDENTIFIER);

        return *export_stmt;
    }

    ImportStmt parse_module_import_stmt()
    {
        expect_keyword("using");

        auto import_stmt = new ImportStmt();
        import_stmt->path = expect_type(TokenType::STRING_LIT);

        if (check_keyword("as"))
        {
            consume();
            import_stmt->ident = expect_type(TokenType::IDENTIFIER);
        }

        return *import_stmt;
    }

    ClassDeclStmt parse_class_decl_stmt()
    {
        expect_keyword("class");

        auto class_stmt = new ClassDeclStmt();
        class_stmt->ident = expect_type(TokenType::IDENTIFIER);
        class_stmt->constructor = FunDeclStmt();
        class_stmt->destructor = FunDeclStmt();

        auto class_scope = parse_scope();

        for (const auto& scope_content : class_scope.stmts)
            std::visit([&](auto&& scope_stmt) {
            using T = std::decay_t<decltype(scope_stmt)>;

            if constexpr (std::is_same_v<T, DeclStmt>)
                class_stmt->attribs.push_back(scope_stmt);
            else if constexpr (std::is_same_v<T, FunDeclStmt>)
            {
                if (scope_stmt.ident.value == CLASS_CONSTRUCTOR_IDENT)
                    class_stmt->constructor = scope_stmt;
                else if (scope_stmt.ident.value == CLASS_DESTRUCTOR_IDENT)
                    class_stmt->destructor = scope_stmt;
                else
                    class_stmt->methods.push_back(scope_stmt);
            }
                }, scope_content);

        return *class_stmt;
    }

    ForStmt parse_for_stmt()
    {
        expect_keyword("for");

        auto k_ident = expect_type(TokenType::IDENTIFIER);

        expect_type(TokenType::COMMA);

        auto v_ident = expect_type(TokenType::IDENTIFIER);

        expect_keyword("in");

        auto for_stmt = new ForStmt();
        for_stmt->k_ident = k_ident;
        for_stmt->v_ident = v_ident;
        for_stmt->iterator = parse_expr();
        for_stmt->do_block = parse_scope();

        return *for_stmt;
    }

    WhileStmt parse_while_stmt()
    {
        expect_keyword("while");

        auto while_stmt = new WhileStmt();
        while_stmt->cond = parse_expr();
        while_stmt->do_block = parse_scope();

        return *while_stmt;
    }

    IfStmt parse_if_stmt()
    {
        auto if_stmt = new IfStmt();

        expect_keyword("if");

        if_stmt->cond = parse_expr();
        if_stmt->then_block = parse_scope();

        while (true)
        {
            if (!check_keyword("elseif"))
                break;
            else if (check_type(TokenType::END))
                CompilerOut::error(
                    get_finfo_header() + "Unexpected token END to elseif statement chain"
                );

            expect_keyword("elseif");
            expect_type(TokenType::LPAR);

            auto elif_stmt = new ElifStmt();
            elif_stmt->cond = parse_expr();

            expect_type(TokenType::RPAR);

            elif_stmt->then_block = parse_scope();
            if_stmt->elif_stmts.push_back(*elif_stmt);
        }

        if (check_keyword("else"))
        {
            consume();
            if_stmt->else_block = parse_scope();
        }

        return *if_stmt;
    }

    AssignStmt parse_assign_stmt()
    {
        auto ident = expect_type(TokenType::IDENTIFIER);

        expect_type(TokenType::ASSIGN);

        auto value = parse_expr();
        auto assign_stmt = new AssignStmt();
        assign_stmt->ident = ident;
        assign_stmt->value = value;

        return *assign_stmt;
    }

    auto parse_call_args()
    {
        std::vector<Expr> args;

        expect_type(TokenType::LPAR);

        auto expecting_expr = true;
        auto expecting_comma = false;

        while (true)
        {
            if (check_type(TokenType::RPAR))
                break;
            else if (check_type(TokenType::END))
                CompilerOut::error(
                    get_finfo_header() + "Unexpected token END to close function call arguments"
                );

            if (expecting_expr)
            {
                args.push_back(parse_expr());

                expecting_expr = false;
                expecting_comma = true;

                continue;
            }

            if (expecting_comma)
            {
                expect_type(TokenType::COMMA);

                expecting_expr = true;
                expecting_comma = false;

                continue;
            }
        }

        expect_type(TokenType::RPAR);

        return args;
    }

    _IndexStmt parse_index_stmt()
    {
        auto ident = expect_type(TokenType::IDENTIFIER);
        auto is_quick_index = check_type(INDEX_OP);
        auto is_expr_index = check_type(TokenType::RSQU);

        Expr index_key_expr;

        if (is_quick_index)
        {
            expect_type(INDEX_OP);

            auto index = expect_type(TokenType::IDENTIFIER);
            auto index_key = index;
            index_key.type = TokenType::STRING_LIT;
            index_key_expr = LitExpr{ index_key };
        }

        if (is_expr_index)
        {
            expect_type(TokenType::LSQU);
            index_key_expr = parse_expr();
            expect_type(TokenType::RSQU);
        }

        if (check_type(TokenType::LPAR))
        {
            auto index_call = new IndexCallStmt();
            index_call->ident = ident;
            index_call->key = index_key_expr;
            index_call->args = parse_call_args();

            return *index_call;
        }
        else if (check_type(TokenType::ASSIGN))
        {
            consume();

            auto index_assign = new IndexAssignStmt();
            index_assign->ident = ident;
            index_assign->key = index_key_expr;
            index_assign->value = parse_expr();
        }

        auto index_stmt = new IndexStmt();
        index_stmt->ident = ident;
        index_stmt->key = index_key_expr;

        return *index_stmt;
    }

    CallStmt parse_fun_call()
    {
        auto ident = expect_type(TokenType::IDENTIFIER);
        auto fun_call_stmt = new CallStmt();
        fun_call_stmt->ident = ident;
        fun_call_stmt->args = parse_call_args();

        return *fun_call_stmt;
    }

    BinExpr parse_bin_expr()
    {
        auto bin_expr = new BinExpr();
        bin_expr->lhs = &parse_expr();
        bin_expr->op = current().is_operator() ? current() : NULL_TOKEN;

        if (bin_expr->op.type == TokenType::ERROR)
            CompilerOut::error(
                get_finfo_header() + "Expected valid binary operator after lvalue"
            );

        bin_expr->rhs = &parse_expr();

        return *bin_expr;
    }

    Expr parse_expr()
    {
        auto is_literal = current().is_literal();

        if (check_type(TokenType::MINUS))
        {
            consume();

            auto un_expr = new UnExpr();
            un_expr->expr = &parse_expr();

            return *un_expr;
        }
        else if (check_type(TokenType::LPAR))
        {
            consume();

            auto group_expr = new GroupExpr();
            group_expr->expr = &parse_expr();

            expect_type(TokenType::RPAR);

            return *group_expr;
        }
        else if (check_keyword("not"))
        {
            consume();
            return NotExpr{ &parse_expr() };
        }
        else if (check_type(TokenType::IDENTIFIER) && check_type(TokenType::LPAR, 1))
        {
            auto call_expr = new CallExpr();
            call_expr->ident = consume();
            call_expr->args = parse_call_args();

            return *call_expr;
        }
        else if (check_type(TokenType::IDENTIFIER) || is_literal)
        {
            if (peek(1).is_operator())
                return parse_bin_expr();
            else
            {
                if (is_literal)
                {
                    auto lit = consume();
                    auto lit_expr = new LitExpr();
                    lit_expr->lit = lit;

                    return *lit_expr;
                }
                else
                {
                    consume();

                    auto ref_expr = new RefExpr();
                    ref_expr->ident = expect_type(TokenType::IDENTIFIER);

                    return *ref_expr;
                }
            }
        }

        CompilerOut::error(
            get_finfo_header() + "Failed to parse expression (malformed/invalid expression)"
        );

        // To shut the fucking compiler warnings up
        return {};
    }

    Header parse_header()
    {

    }

    DeclStmt parse_decl()
    {
        expect_keyword(DECL_KW);

        auto is_const = check_type(CONST_SPEC);

        if (is_const)
            consume();

        auto ident = expect_type(TokenType::IDENTIFIER);

        expect_type(TokenType::ASSIGN);

        auto decl = new DeclStmt();
        decl->ident = ident;
        decl->is_cnst = is_const;
        decl->is_glb = false; /* TODO */
        decl->value = parse_expr();

        return *decl;
    }

    BlockStmt parse_scope()
    {
        expect_type(TokenType::LCURLY);

        auto scope = new BlockStmt();

        while (true)
        {
            if (check_type(TokenType::RCURLY))
            {
                consume();
                break;
            }
            else if (check_type(TokenType::END))
                CompilerOut::error(
                    get_finfo_header() + "Unexpected token END to close scope"
                );

            scope->stmts.push_back(parse_stmt());
        }

        return *scope;
    }

    FunDeclStmt parse_fun_decl()
    {
        expect_keyword(FUNC_KW);

        auto is_const = check_type(CONST_SPEC);

        if (is_const)
            consume();

        auto ident = expect_type(TokenType::IDENTIFIER);

        expect_type(TokenType::LPAR);

        auto expecting_ident = true;
        auto expecting_comma = false;

        std::vector<Token> params; /* TODO: Add type support */

        while (true)
        {
            if (check_type(TokenType::RPAR))
                break;
            else if (check_type(TokenType::END))
                CompilerOut::error(
                    get_finfo_header() + "Unexpected token END to close function arguments"
                );

            if (expecting_ident)
            {
                expect_type(TokenType::IDENTIFIER);

                auto has_type = check_type(TokenType::SEMICOL);

                if (has_type)
                {
                    expect_type(TokenType::SEMICOL);
                    auto ident = expect_type(TokenType::IDENTIFIER);

                    params.push_back(ident);

                    /* TODO: Types support */
                }

                expecting_comma = true;
                expecting_ident = false;

                continue;
            }

            if (expecting_comma)
            {
                expect_type(TokenType::COMMA);

                expecting_comma = false;
                expecting_ident = true;

                continue;
            }
        }

        auto fun_decl = new FunDeclStmt();
        fun_decl->ident = ident;
        fun_decl->is_cnst = is_const;
        fun_decl->is_glb = false; /* TODO */
        fun_decl->params = params;
        fun_decl->fun_scope = parse_scope();

        return *fun_decl;
    }

    AST* ast;
    size_t pos;
    std::string fname;
    std::vector<Token> toks;
    std::vector<std::string> flines;

    std::string get_finfo_header()
    {
        return fname + ":" + std::to_string(current().line) + ": ";
    }

    Token peek(int ahead = 1)
    {
        return toks.at(ahead);
    }

    Token consume(int ahead = 1)
    {
        pos += ahead; return current();
    }

    Token current()
    {
        return peek(0);
    }

    bool check_type(TokenType type = TokenType::ERROR, int ahead = 0)
    {
        return consume(ahead).type == type;
    }

    bool check_value(std::string value = "", int ahead = 0)
    {
        return consume(ahead).value == value;
    }

    bool check_keyword(std::string value)
    {
        return check_type(TokenType::KEYWORD) && check_value(value);
    }

    Token expect_type(TokenType expected_type)
    {
        if (check_type(expected_type))
            return consume();
        else
        {
            CompilerOut::error(
                get_finfo_header() + "Expected token of type "
                + std::string(magic_enum::enum_name(expected_type)) + ", got "
                + std::string(magic_enum::enum_name(current().type))
            );
            return NULL_TOKEN;
        }
    }

    Token expect_value(std::string expected_value)
    {
        if (check_value(expected_value))
            return consume();
        else
        {
            CompilerOut::error(
                get_finfo_header() + "Expected token of value "
                + expected_value + ", got "
                + current().value
            );
            return NULL_TOKEN;
        }
    }

    Token expect_keyword(std::string keyword)
    {
        auto type = check_type(TokenType::KEYWORD);
        auto val = check_value(keyword);

        if (type && val)
            return consume();
        else
        {
            CompilerOut::error(
                get_finfo_header() + "Expected keyword "
                + keyword + ", got "
                + current().value
            );
            return NULL_TOKEN;
        }
    }
};

#endif