// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "type.h"
#include "ir/ir.h"

namespace sema = via::sema;

using UnaryOp = via::ir::ExprUnary::Op;
using BinaryOp = via::ir::ExprBinary::Op;

static UnaryOp toUnaryOp(via::Token::Kind kind)
{
  using enum via::Token::Kind;

  switch (kind) {
    case OP_MINUS:
      return UnaryOp::NEG;
    case KW_NOT:
      return UnaryOp::NOT;
    case OP_TILDE:
      return UnaryOp::BNOT;
    default:
      break;
  }

  via::debug::bug("Failed to get unary operator from token kind");
}

static BinaryOp toBinaryOp(via::Token::Kind kind)
{
  using enum via::Token::Kind;

  switch (kind) {
    case OP_PLUS:
      return BinaryOp::ADD;
    case OP_MINUS:
      return BinaryOp::SUB;
    case OP_STAR:
      return BinaryOp::MUL;
    case OP_SLASH:
      return BinaryOp::DIV;
    case OP_STAR_STAR:
      return BinaryOp::POW;
    case OP_PERCENT:
      return BinaryOp::MOD;
    case KW_AND:
      return BinaryOp::AND;
    case KW_OR:
      return BinaryOp::OR;
    case OP_AMP:
      return BinaryOp::BAND;
    case OP_PIPE:
      return BinaryOp::BOR;
    case OP_CARET:
      return BinaryOp::BXOR;
    case OP_SHL:
      return BinaryOp::BSHL;
    case OP_SHR:
      return BinaryOp::BSHR;
    // case OP_EQ_EQ:
    //   return BinaryOp::EQ;
    // case OP_BANG_EQ:
    //   return BinaryOp::NEQ;
    // case OP_LT:
    //   return BinaryOp::LT;
    // case OP_GT:
    //   return BinaryOp::GT;
    // case OP_LT_EQ:
    //   return BinaryOp::LTEQ;
    // case OP_GT_EQ:
    //   return BinaryOp::GTEQ;
    // case OP_DOT_DOT:
    //   return BinaryOp::CONCAT;
    default:
      break;
  }

  via::debug::bug("Failed to get binary operator from token kind");
}

via::Expected<sema::Type*> sema::Type::infer(Allocator& alloc,
                                             const ast::Expr* expr)
{
  return nullptr;
}

via::Expected<sema::Type*> sema::Type::from(Allocator& alloc,
                                            const ast::Type* type)
{
  return nullptr;
}
