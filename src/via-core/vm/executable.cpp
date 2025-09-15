/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "executable.h"
#include <iostream>
#include "ansi.h"
#include "module/module.h"

#define PMR_CASE(NODE, TYPE, HANDLER, ...)  \
  if TRY_COERCE (const TYPE, inner, NODE) { \
    HANDLER(__VA_ARGS__);                   \
    return;                                 \
  }

void via::Executable::lowerExprConstant(const ir::ExprConstant* exprConstant,
                                        u16 dst) noexcept
{
  pushConstant(exprConstant->value);
  pushInstr(OpCode::LOADK, {dst, static_cast<u16>(constantId())});
}

void via::Executable::lowerExprSymbol(const ir::ExprSymbol* exprSymbol,
                                      u16 dst) noexcept
{
  auto& frame = mStack.top();
  if (auto lref = frame.getLocal(exprSymbol->symbol)) {
    pushInstr(OpCode::GETLOCAL, {dst, lref->id});
  }
}

void via::Executable::lowerExprAccess(const ir::ExprAccess* exprAccess,
                                      u16 dst) noexcept
{}

void via::Executable::lowerExprModuleAccess(
  const ir::ExprModuleAccess* exprModuleAccess,
  u16 dst) noexcept
{}

void via::Executable::lowerExprUnary(const ir::ExprUnary* exprUnary,
                                     u16 dst) noexcept
{}

void via::Executable::lowerExprBinary(const ir::ExprBinary* exprBinary,
                                      u16 dst) noexcept
{
  u16 opid = static_cast<u16>(exprBinary->op);
  u16 rlhs = mRegState.alloc(), rrhs = mRegState.alloc();

  lowerExpr(exprBinary->lhs, rlhs);
  lowerExpr(exprBinary->rhs, rrhs);

  if (opid >= static_cast<u16>(BinaryOp::ADD) &&
      opid <= static_cast<u16>(BinaryOp::MOD)) {
    /* TODO: Check if rhs is constexpr, in which case increment base by one
     * for K instructions*/
    u16 base =
      static_cast<u16>(OpCode::IADD) + static_cast<u16>(exprBinary->op);

    if (exprBinary->lhs->type->isIntegral()) {
      if (exprBinary->rhs->type->isFloat()) {
        base += 2;  // FP mode
        pushInstr(OpCode::ITOF, {rlhs, rlhs});
      }

    } else {
      base += 2;  // FP mode

      if (exprBinary->rhs->type->isIntegral()) {
        pushInstr(OpCode::ITOF, {rrhs, rrhs});
      }
    }

    pushInstr(static_cast<OpCode>(base), {dst, rlhs, rrhs});
  } else if (opid >= static_cast<u16>(BinaryOp::AND) &&
             opid <= static_cast<u16>(BinaryOp::OR)) {
    /* TODO: Check if rhs is constexpr, in which case increment base by one
     * for K instructions*/
    u16 base = static_cast<u16>(OpCode::AND) + static_cast<u16>(exprBinary->op);
    pushInstr(static_cast<OpCode>(base), {dst, rlhs, rrhs});
  } else if (opid >= static_cast<u16>(BinaryOp::BAND) &&
             opid <= static_cast<u16>(BinaryOp::BSHR)) {
    /* TODO: Check if rhs is constexpr, in which case increment base by one
     * for K instructions*/
    u16 base =
      static_cast<u16>(OpCode::BAND) + static_cast<u16>(exprBinary->op);
    pushInstr(static_cast<OpCode>(base), {dst, rlhs, rrhs});
  }
}

void via::Executable::lowerExprCall(const ir::ExprCall* exprCall,
                                    u16 dst) noexcept
{}

void via::Executable::lowerExprSubscript(const ir::ExprSubscript* exprSubs,
                                         u16 dst) noexcept
{}

void via::Executable::lowerExprCast(const ir::ExprCast* exprCast,
                                    u16 dst) noexcept
{}

void via::Executable::lowerExprTernary(const ir::ExprTernary* exprTernary,
                                       u16 dst) noexcept
{}

void via::Executable::lowerExprArray(const ir::ExprArray* exprArray,
                                     u16 dst) noexcept
{}

void via::Executable::lowerExprTuple(const ir::ExprTuple* exprTuple,
                                     u16 dst) noexcept
{}

void via::Executable::lowerExprLambda(const ir::ExprLambda* exprLambda,
                                      u16 dst) noexcept
{}

void via::Executable::lowerExpr(const ir::Expr* expr, u16 dst) noexcept
{
  PMR_CASE(expr, ir::ExprConstant, lowerExprConstant, inner, dst);
  PMR_CASE(expr, ir::ExprSymbol, lowerExprSymbol, inner, dst);
  PMR_CASE(expr, ir::ExprAccess, lowerExprAccess, inner, dst);
  PMR_CASE(expr, ir::ExprModuleAccess, lowerExprModuleAccess, inner, dst);
  PMR_CASE(expr, ir::ExprUnary, lowerExprUnary, inner, dst);
  PMR_CASE(expr, ir::ExprBinary, lowerExprBinary, inner, dst);
  PMR_CASE(expr, ir::ExprCall, lowerExprCall, inner, dst);
  PMR_CASE(expr, ir::ExprSubscript, lowerExprSubscript, inner, dst);
  PMR_CASE(expr, ir::ExprCast, lowerExprCast, inner, dst);
  PMR_CASE(expr, ir::ExprTernary, lowerExprTernary, inner, dst);
  PMR_CASE(expr, ir::ExprArray, lowerExprArray, inner, dst);
  PMR_CASE(expr, ir::ExprTuple, lowerExprTuple, inner, dst);
  PMR_CASE(expr, ir::ExprLambda, lowerExprLambda, inner, dst);

  debug::unimplemented(std::format("lowerExpr({}, u16)", TYPENAME(*expr)));
}

void via::Executable::lowerStmtVarDecl(
  const ir::StmtVarDecl* stmtVarDecl) noexcept
{
  u16 dst = mRegState.alloc();

  lowerExpr(stmtVarDecl->expr, dst);
  pushInstr(OpCode::PUSH, {dst});

  mRegState.free(dst);
}

void via::Executable::lowerStmtFuncDecl(
  const ir::StmtFuncDecl* stmtFuncDecl) noexcept
{
  u16 dst = mRegState.alloc();

  pushInstr(OpCode::NEWCLOSURE, {dst});
  lowerStmtBlock(stmtFuncDecl->body);
  pushInstr(OpCode::ENDCLOSURE);
  pushInstr(OpCode::PUSH, {dst});

  mRegState.free(dst);
}

void via::Executable::lowerStmtBlock(const ir::StmtBlock* stmtBlock) noexcept
{
  setLabel(stmtBlock->id);

  for (const auto& stmt : stmtBlock->stmts) {
    lowerStmt(stmt);
  }

  /* TODO: Lower terminator */
}

void via::Executable::lowerStmtExpr(const ir::StmtExpr* stmtExpr) noexcept
{
  lowerExpr(stmtExpr->expr, mGarbageReg);
}

void via::Executable::lowerStmt(const ir::Stmt* stmt) noexcept
{
  PMR_CASE(stmt, ir::StmtVarDecl, lowerStmtVarDecl, inner);
  PMR_CASE(stmt, ir::StmtFuncDecl, lowerStmtFuncDecl, inner);
  PMR_CASE(stmt, ir::StmtBlock, lowerStmtBlock, inner);
  PMR_CASE(stmt, ir::StmtExpr, lowerStmtExpr, inner);

  debug::unimplemented(std::format("lowerStmt({})", TYPENAME(*stmt)));
}

via::Executable* via::Executable::buildFromIR(Module* module,
                                              const IRTree& ir,
                                              u64 flags) noexcept
{
  auto& alloc = module->getAllocator();
  auto* exe = alloc.emplace<Executable>();
  exe->mGarbageReg = exe->mRegState.alloc();

  for (const auto& stmt : ir) {
    exe->lowerStmt(stmt);
  }

  exe->lowerJumps();
  exe->pushInstr(OpCode::HALT);
  return exe;
}

void via::Executable::lowerJumps() noexcept
{
  for (usize pc = 0; Instruction & instr : mBytecode) {
    auto opid = static_cast<u16>(instr.op);
    if (opid >= static_cast<u16>(OpCode::JMP) &&
        opid <= static_cast<u16>(OpCode::JMPIFX)) {
      auto address = mLabelTable[instr.a];
      auto offset = static_cast<isize>(address) - static_cast<isize>(pc);

      if (offset < 0) {
        // backward jump → bump opcode to BACK variant
        instr.op = static_cast<OpCode>(opid + 3);
        instr.a = static_cast<u32>(-offset);
      } else {
        // forward jump → keep opcode
        instr.a = static_cast<u32>(offset);
      }
    }
    ++pc;
  }
}

std::string via::Executable::dump() const
{
  std::ostringstream oss;
  oss << ansi::format("[section .text]\n", ansi::Foreground::Yellow,
                      ansi::Background::Black, ansi::Style::Underline);

  for (usize pc = 0; const Instruction& insn : mBytecode) {
    if (auto it = std::find_if(mLabelTable.begin(), mLabelTable.end(),
                               [&pc](auto&& p) { return p.second == pc; });
        it != mLabelTable.end())
      oss << " .LB" << it->first << ":\n";
    oss << "  " << insn.dump() << "\n";
    pc++;
  }

  oss << ansi::format("[section .data]\n", ansi::Foreground::Yellow,
                      ansi::Background::Black, ansi::Style::Underline);

  for (const sema::ConstValue& cv : mConstants) {
    oss << "  " << cv.dump() << "\n";
  }

  return oss.str();
}
