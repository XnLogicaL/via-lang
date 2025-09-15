/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.h>
#include <via/types.h>
#include "instruction.h"
#include "ir/ir.h"
#include "sema/bytecode_local.h"
#include "sema/const_value.h"
#include "sema/register.h"
#include "sema/stack.h"

namespace via
{

namespace config
{
inline constexpr u32 kMagic = 0x2E766961;  // .via
}

class Module;
class Executable final
{
 public:
  enum Flags : u64
  {
  };

 public:
  static Executable* buildFromIR(Module* module,
                                 const IRTree& ir,
                                 u64 flags = 0) noexcept;

  static Executable* buildFromBinary(Module* module,
                                     const std::vector<unsigned char>& bytes,
                                     u64 flags = 0) noexcept;

 public:
  auto flags() const noexcept { return mFlags; }
  auto& constants() const noexcept { return mConstants; }
  auto& bytecode() const noexcept { return mBytecode; }
  std::string dump() const;

 private:
  usize programCounter() const noexcept { return mBytecode.size() - 1; }
  usize constantId() const noexcept { return mConstants.size() - 1; }
  usize setLabel(usize id) noexcept
  {
    mLabelTable[id] = programCounter();
    return mLabelTable.size() - 1;
  }

  void pushConstant(sema::ConstValue cvalue) noexcept
  {
    mConstants.push_back(std::move(cvalue));
  }

  void pushInstr(OpCode op, std::array<u16, 3>&& ops = {}) noexcept
  {
    mBytecode.emplace_back(op, ops[0], ops[1], ops[2]);
  }

  // clang-format off
  void lowerExprConstant(const ir::ExprConstant* exprConstant, u16 dst) noexcept;
  void lowerExprSymbol(const ir::ExprSymbol* exprSymbol, u16 dst) noexcept;
  void lowerExprAccess(const ir::ExprAccess* exprAccess, u16 dst) noexcept;
  void lowerExprModuleAccess(const ir::ExprModuleAccess* exprModuleAccess, u16 dst) noexcept;
  void lowerExprUnary(const ir::ExprUnary* exprUnary, u16 dst) noexcept;
  void lowerExprBinary(const ir::ExprBinary* exprBinary, u16 dst) noexcept;
  void lowerExprCall(const ir::ExprCall* exprCall, u16 dst) noexcept;
  void lowerExprSubscript(const ir::ExprSubscript* exprSubs, u16 dst) noexcept;
  void lowerExprCast(const ir::ExprCast* exprCast, u16 dst) noexcept;
  void lowerExprTernary(const ir::ExprTernary* exprTernary, u16 dst) noexcept;
  void lowerExprArray(const ir::ExprArray* exprArray, u16 dst) noexcept;
  void lowerExprTuple(const ir::ExprTuple* exprTuple, u16 dst) noexcept;
  void lowerExprLambda(const ir::ExprLambda* exprLambda, u16 dst) noexcept;
  void lowerExpr(const ir::Expr* expr, u16 dst) noexcept;

  void lowerStmtVarDecl(const ir::StmtVarDecl* stmtVarDecl) noexcept;
  void lowerStmtFuncDecl(const ir::StmtFuncDecl* stmtFuncDecl) noexcept;
  void lowerStmtBlock(const ir::StmtBlock* stmtBlock) noexcept;
  void lowerStmtExpr(const ir::StmtExpr* stmtExpr) noexcept;
  void lowerStmt(const ir::Stmt* stmt) noexcept;
  // clang-format on

  void lowerJumps() noexcept;

 private:
  u64 mFlags;
  u16 mGarbageReg;
  sema::RegisterState mRegState;
  sema::StackState<sema::BytecodeLocal> mStack;
  std::vector<Instruction> mBytecode;
  std::vector<sema::ConstValue> mConstants;
  std::unordered_map<usize, usize> mLabelTable;
};

}  // namespace via
