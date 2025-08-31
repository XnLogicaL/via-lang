// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "type.h"
#include "sema/stack.h"

namespace via
{

namespace sema
{

using namespace ast;

enum class UnaryOp : u8
{
  NEG,
  NOT,
  BNOT,
};

enum class BinaryOp : u8
{
  ADD,
  SUB,
  MUL,
  DIV,
  POW,
  MOD,
  AND,
  OR,
  BAND,
  BOR,
  BXOR,
  BSHL,
  BSHR,
  EQ,
  NEQ,
  LT,
  GT,
  LTEQ,
  GTEQ,
  CONCAT,
};

static UnaryOp to_unary_op(Token::Kind kind)
{
  using enum Token::Kind;

  switch (kind) {
    case MINUS:
      return UnaryOp::NEG;
    case KW_NOT:
      return UnaryOp::NOT;
    case TILDE:
      return UnaryOp::BNOT;
    default:
      break;
  }

  debug::bug("Failed to get unary operator from token kind");
}

static BinaryOp to_binary_op(Token::Kind kind)
{
  using enum Token::Kind;

  switch (kind) {
    case PLUS:
      return BinaryOp::ADD;
    case MINUS:
      return BinaryOp::SUB;
    case ASTERISK:
      return BinaryOp::MUL;
    case FSLASH:
      return BinaryOp::DIV;
    case POW:
      return BinaryOp::POW;
    case PERCENT:
      return BinaryOp::MOD;
    case KW_AND:
      return BinaryOp::AND;
    case KW_OR:
      return BinaryOp::OR;
    case AMPERSAND:
      return BinaryOp::BAND;
    case PIPE:
      return BinaryOp::BOR;
    case CARET:
      return BinaryOp::BXOR;
    case KW_SHL:
      return BinaryOp::BSHL;
    case KW_SHR:
      return BinaryOp::BSHR;
    case DBEQUALS:
      return BinaryOp::EQ;
    case BANGEQUALS:
      return BinaryOp::NEQ;
    case LESSTHAN:
      return BinaryOp::LT;
    case GREATERTHAN:
      return BinaryOp::GT;
    case LESSTHANEQUALS:
      return BinaryOp::LTEQ;
    case GREATERTHANEQUALS:
      return BinaryOp::GTEQ;
    case CONCAT:
      return BinaryOp::CONCAT;
    default:
      break;
  }

  debug::bug("Failed to get binary operator from token kind");
}

struct UnaryVisitInfo : VisitInfo
{
  Type* type = nullptr;
  String fail;
  Allocator& alloc;
  UnaryOp op;

  explicit UnaryVisitInfo(Allocator& alloc, UnaryOp op) : alloc(alloc), op(op)
  {}

  static UnaryVisitInfo* from(VisitInfo* raw)
  {
    if TRY_COERCE (UnaryVisitInfo, uvi, raw) {
      return uvi;
    } else {
      debug::bug("Invalid visit info passed to unary visitor");
    }
  }
};

struct BinaryVisitInfo : VisitInfo
{
  Type* type = nullptr;
  String fail;
  Allocator& alloc;
  BinaryOp op;

  explicit BinaryVisitInfo(Allocator& alloc, BinaryOp& op)
      : alloc(alloc), op(op)
  {}

  static BinaryVisitInfo* from(VisitInfo* raw)
  {
    if TRY_COERCE (BinaryVisitInfo, bvi, raw) {
      return bvi;
    } else {
      debug::bug("Invalid visit info passed to unary visitor");
    }
  }
};

struct UnaryVisitor : TypeVisitor
{
  void visit(const BuiltinType& bt, VisitInfo* raw) override
  {
    using enum BuiltinType::Kind;

    auto* vi = UnaryVisitInfo::from(raw);

    switch (vi->op) {
      case UnaryOp::NEG:
        if (bt.bt == Int || bt.bt == Float) {
          vi->type = vi->alloc.emplace<BuiltinType>(bt.bt);
        } else {
          vi->type = nullptr;
        }
        break;
      case UnaryOp::NOT:
        vi->type = vi->alloc.emplace<BuiltinType>(Bool);
        break;
      case UnaryOp::BNOT:
        if (bt.bt == Int) {
          vi->type = vi->alloc.emplace<BuiltinType>(Int);
        } else {
          vi->type = nullptr;
          vi->fail = fmt::format("");
        }
        break;
      default:
        debug::bug("Unknown unary operator in UnaryVisitor");
        vi->type = nullptr;
    }
  }

  // clang-format off
  void visit(const ArrayType&, VisitInfo*) override { debug::bug("Unary operator cannot be applied to arrays"); }
  void visit(const FuncType&, VisitInfo*) override { debug::bug("Unary operator cannot be applied to functions"); }
  void visit(const DictType&, VisitInfo*) override { debug::bug("Unary operator cannot be applied to dictionaries"); }
  void visit(const UserType&, VisitInfo*) override { debug::unimplemented(); }
  void visit(const TemplateParamType&, VisitInfo*) override { debug::unimplemented(); }
  void visit(const TemplateSpecType&, VisitInfo*) override { debug::unimplemented(); }
  void visit(const SubstParamType&, VisitInfo*) override { debug::unimplemented(); }
  // clang-format on
};

struct BinaryVisitor : TypeVisitor
{};

struct InferVisitInfo : VisitInfo
{
  Type* type = nullptr;
  String fail;
  Allocator& alloc;

  explicit InferVisitInfo(Allocator& alloc) : alloc(alloc) {}

  static InferVisitInfo* from(VisitInfo* raw)
  {
    if (auto* info = dynamic_cast<InferVisitInfo*>(raw)) {
      return info;
    } else {
      debug::bug("Invalid visit info passed to inference visitor");
    }
  }
};

struct InferVisitor : Visitor
{
  void visit(const ExprLit& elit, VisitInfo* raw) override
  {
    using enum Token::Kind;
    using enum BuiltinType::Kind;

    auto* vi = InferVisitInfo::from(raw);
    BuiltinType::Kind kind;

    switch (elit.tok->kind) {
      case NIL:
        kind = Nil;
        break;
      case TRUE:
      case FALSE:
        kind = Bool;
        break;
      case INT:
      case XINT:
      case BINT:
        kind = Int;
        break;
      case FP:
        kind = Float;
        break;
      case STRING:
        kind = String;
        break;
      default:
        debug::bug("inference visitor: bad literal token");
        break;
    }

    vi->type = vi->alloc.emplace<BuiltinType>(kind);
  }

  void visit(const ExprSymbol& esym, VisitInfo* raw) override
  {
    auto* vi = InferVisitInfo::from(raw);
    Frame& frame = stack::top();
    StringView symbol = esym.sym->toStringView();

    if (auto lref = frame.getLocal(symbol)) {
      lref->local.getType()->accept(*this, raw);
    } else {
      debug::bug("inference visitor: symbol lookup failed");
    }
  }

  void visit(const ExprUnary& eun, VisitInfo* raw) override
  {
    auto* vi = InferVisitInfo::from(raw);

    UnaryVisitor uvis;
    UnaryVisitInfo uvi(vi->alloc, to_unary_op(eun.op->kind));

    if (auto ir = Type::infer(vi->alloc, eun.expr)) {
      ir.value()->accept(uvis, &uvi);
    }

    vi->type = uvi.type;
  }

  void visit(const ExprBinary&, VisitInfo* raw) override {}
  void visit(const ExprGroup&, VisitInfo* raw) override {}
  void visit(const ExprCall&, VisitInfo* raw) override {}
  void visit(const ExprSubscript&, VisitInfo* raw) override {}
  void visit(const ExprTuple&, VisitInfo* raw) override {}
  void visit(const ExprLambda&, VisitInfo* raw) override {}

  void visit(const TypeBuiltin&, VisitInfo* raw) override {}
  void visit(const TypeArray&, VisitInfo* raw) override {}
  void visit(const TypeDict&, VisitInfo* raw) override {}
  void visit(const TypeFunc&, VisitInfo* raw) override {}
};

Type::InferResult Type::infer(Allocator& alloc, const ast::Expr* expr)
{
  InferVisitor vis;
  InferVisitInfo raw(alloc);
  expr->accept(vis, &raw);

  if (raw.type == nullptr) {
    return std::unexpected(raw.fail);
  } else {
    return raw.type;
  }
}

Type::InferResult Type::from(Allocator& alloc, const ast::Type* type)
{
  InferVisitor vis;
  InferVisitInfo raw(alloc);
  type->accept(vis, &raw);

  if (raw.type == nullptr) {
    return std::unexpected(raw.fail);
  } else {
    return raw.type;
  }
}

}  // namespace sema

}  // namespace via
