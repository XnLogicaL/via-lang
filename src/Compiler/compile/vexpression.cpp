#include "vexpression.h"

using namespace via;
using namespace Tokenization;
using namespace Compilation;
using namespace Parsing;

size_t Compilation::compile_lit_expr(Generator *gen, AST::LitExprNode lit)
{
    size_t reg = gen->get_available_register();

    gen->pushline(std::format("LI R{}, {};", reg, lit.val.value));

    return reg;
}

size_t Compilation::compile_un_expr(Generator *gen, AST::UnExprNode un)
{
    size_t expr_reg = compile_expression(gen, un);
    size_t un_reg   = gen->get_available_register();

    gen->free_register(expr_reg);
    gen->pushline(std::format("NEG R{}, R{};", un_reg, expr_reg));

    return un_reg;
}

size_t Compilation::compile_binary_expr(Generator *gen, AST::BinExprNode bin)
{
    size_t lhs_reg = compile_expression(gen, *bin.lhs);
    size_t rhs_reg = compile_expression(gen, *bin.rhs);
    size_t bin_reg = gen->get_available_register();

    static std::unordered_map<TokenType, std::string> op_table
        = { { TokenType::OP_ADD, "ADD" }, { TokenType::OP_SUB, "SUB" }, { TokenType::OP_MUL, "MUL" },
            { TokenType::OP_DIV, "DIV" }, { TokenType::OP_MOD, "MOD" }, { TokenType::OP_EQ, "EQ" },
            { TokenType::OP_DEC, "DEC" }, { TokenType::OP_EXP, "POW" }, { TokenType::OP_GEQ, "GEQ" },
            { TokenType::OP_GT, "GT" },   { TokenType::OP_INC, "INC" }, { TokenType::OP_LEQ, "LEQ" },
            { TokenType::OP_LT, "LT" },   { TokenType::OP_NEQ, "NEQ" } };

    gen->free_register(lhs_reg);
    gen->free_register(rhs_reg);
    gen->pushline(std::format("{} R{}, R{}, R{};", op_table[bin.op.type], bin_reg, lhs_reg, rhs_reg));

    return bin_reg;
}

size_t Compilation::compile_index_expr(Generator *gen, AST::IndexExprNode idx)
{
    size_t tbl_reg = gen->get_available_register();
    size_t idx_reg = gen->get_available_register();

    // TODO: Support other expression types
    auto ident = std::get<AST::IdentExprNode>(*idx.ident).val;

    gen->pushline(std::format("GETLOCAL @{}, R{};", ident, tbl_reg));
    gen->pushline(std::format("LI IR{}, \"{}\";", idx.index.value));
    gen->pushline(std::format("LOADIDX R{}, R{};", tbl_reg));
    gen->free_register(tbl_reg);

    return idx_reg;
}

size_t Compilation::compile_expression(Generator *gen, AST::ExprNode expr)
{
    
}
