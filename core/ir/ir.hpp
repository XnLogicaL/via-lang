/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <cstddef>
#include <optional>
#include <via/config.hpp>
#include "module/symbol.hpp"
#include "sema/const.hpp"
#include "sema/types.hpp"
#include "support/utility.hpp"
#include "vm/instruction.hpp"

namespace via {

class Module;
struct Def;

#define FOR_EACH_UNARY_OP(X)                                                             \
    X(NEG)                                                                               \
    X(NOT)                                                                               \
    X(BNOT)

#define FOR_EACH_BINARY_OP(X)                                                            \
    X(ADD)                                                                               \
    X(SUB)                                                                               \
    X(MUL)                                                                               \
    X(DIV)                                                                               \
    X(POW)                                                                               \
    X(MOD)                                                                               \
    X(AND)                                                                               \
    X(OR)                                                                                \
    X(BAND)                                                                              \
    X(BOR)                                                                               \
    X(BXOR)                                                                              \
    X(BSHL)                                                                              \
    X(BSHR)

enum class UnaryOp
{
    FOR_EACH_UNARY_OP(DEFINE_ENUM)
};

enum class BinaryOp
{
    FOR_EACH_BINARY_OP(DEFINE_ENUM)
};

DEFINE_TO_STRING(UnaryOp, FOR_EACH_UNARY_OP(DEFINE_CASE_TO_STRING));
DEFINE_TO_STRING(BinaryOp, FOR_EACH_BINARY_OP(DEFINE_CASE_TO_STRING));

UnaryOp to_unary_op(TokenKind kind) noexcept;
BinaryOp to_binary_op(TokenKind kind) noexcept;

namespace ir {

struct Expr
{
    SourceLoc loc;
    QualType type;
    virtual ~Expr() = default;
    virtual std::string to_string(const SymbolTable* sym_tab, size_t depth = 0) const = 0;
};

struct Stmt
{
    SourceLoc loc;
    virtual ~Stmt() = default;
    virtual std::optional<SymbolId> get_symbol() const { return std::nullopt; }
    virtual std::string to_string(const SymbolTable* sym_tab, size_t depth = 0) const = 0;
};

struct Term
{
    SourceLoc loc;
    virtual ~Term() = default;
    virtual std::string to_string(const SymbolTable* sym_tab, size_t depth = 0) const = 0;
};

#define NODE_FIELDS(BASE)                                                                \
    std::string to_string(const SymbolTable* sym_tab, size_t depth) const override;

struct TrReturn: public Term
{
    NODE_FIELDS(Term)
    bool implicit;
    const Expr* val;
    QualType type;
};

struct TrContinue: public Term
{
    NODE_FIELDS(Term)
};

struct TrBreak: public Term
{
    NODE_FIELDS(Term)
};

struct StmtBlock;

struct TrBranch: public Term
{
    NODE_FIELDS(Term)
    StmtBlock* target;
};

struct TrCondBranch: public Term
{
    NODE_FIELDS(Term)
    const Expr* cnd;
    StmtBlock *iftrue, *iffalse;
};

struct Parameter
{
    SymbolId symbol;
    QualType type;
    std::string to_string(const SymbolTable* sym_tab, size_t depth = 0) const;
};

struct ExprConstant: public Expr
{
    NODE_FIELDS(Expr)
    ConstValue value;
};

struct ExprSymbol: public Expr
{
    NODE_FIELDS(Expr)
    SymbolId symbol;
};

struct ExprAccess: public Expr
{
    NODE_FIELDS(Expr)

    enum class Kind
    {
        STATIC,
        DYNAMIC,
    } kind;

    const Expr* root;
    SymbolId index;
};

struct ExprModuleAccess: public Expr
{
    NODE_FIELDS(Expr)
    Module* module;
    SymbolId mod_id, key_id;
    const Def* def;
};

struct ExprUnary: public Expr
{
    NODE_FIELDS(Expr)
    UnaryOp op;
    const Expr* expr;
};

struct ExprBinary: public Expr
{
    NODE_FIELDS(Expr)
    BinaryOp op;
    const Expr *lhs, *rhs;
};

struct ExprCall: public Expr
{
    NODE_FIELDS(Expr)
    const Expr* callee;
    std::vector<const Expr*> args;
};

struct ExprSubscript: public Expr
{
    NODE_FIELDS(Expr)
    const Expr *expr, *idx;
};

struct ExprCast: public Expr
{
    NODE_FIELDS(Expr)
    const Expr* expr;
    QualType cast;
};

struct ExprTernary: public Expr
{
    NODE_FIELDS(Expr)
    const Expr *cnd, *iftrue, *iffalse;
};

struct ExprArray: public Expr
{
    NODE_FIELDS(Expr)
    std::vector<const Expr*> exprs;
};

struct ExprTuple: public Expr
{
    NODE_FIELDS(Expr)
    std::vector<const Expr*> init;
};

struct Function;
struct ExprLambda: public Expr
{
    NODE_FIELDS(Expr)
};

struct StmtVarDecl: public Stmt
{
    NODE_FIELDS()
    SymbolId symbol;
    const Expr* expr;
    QualType type;
};

struct StmtBlock;
struct StmtFuncDecl: public Stmt
{
    NODE_FIELDS()

    enum class Kind
    {
        IR,
        NATIVE,
    } kind;

    SymbolId symbol;
    QualType ret;
    std::vector<Parameter> parms;
    const StmtBlock* body;

    std::optional<SymbolId> get_symbol() const override { return symbol; }
};

struct StmtInstruction: public Stmt
{
    NODE_FIELDS()
    Instruction instr;
};

struct StmtBlock: public Stmt
{
    NODE_FIELDS()
    uint32_t id;
    std::vector<const Stmt*> stmts;
    const Term* term;
};

struct StmtExpr: public Stmt
{
    NODE_FIELDS()
    const Expr* expr;
};

} // namespace ir

using IRTree = std::vector<const ir::Stmt*>;

std::string to_string(const SymbolTable& sym_tab, const IRTree& ir_tree);

} // namespace via
