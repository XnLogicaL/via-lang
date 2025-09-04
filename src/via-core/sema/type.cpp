// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "type.h"
#include "sema/stack.h"

namespace via
{

namespace sema
{

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

static UnaryOp toUnaryOp(Token::Kind kind)
{
  using enum Token::Kind;

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

  debug::bug("Failed to get unary operator from token kind");
}

static BinaryOp toBinaryOp(Token::Kind kind)
{
  using enum Token::Kind;

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
    case OP_EQ_EQ:
      return BinaryOp::EQ;
    case OP_BANG_EQ:
      return BinaryOp::NEQ;
    case OP_LT:
      return BinaryOp::LT;
    case OP_GT:
      return BinaryOp::GT;
    case OP_LT_EQ:
      return BinaryOp::LTEQ;
    case OP_GT_EQ:
      return BinaryOp::GTEQ;
    case OP_DOT_DOT:
      return BinaryOp::CONCAT;
    default:
      break;
  }

  debug::bug("Failed to get binary operator from token kind");
}

struct UnaryVisitInfo : VisitInfo
{
  const Type* type = nullptr;
  String error = "<no-error>";
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
  const Type* type = nullptr;
  String error = "<no-error>";
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
          vi->error = fmt::format("");
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
  const Type* type = nullptr;
  String error = "<no-error>";
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

struct InferVisitor : ast::Visitor
{
  void visit(const ast::ExprLit& elit, VisitInfo* raw) override
  {
    using enum Token::Kind;
    using enum BuiltinType::Kind;

    auto* vi = InferVisitInfo::from(raw);
    BuiltinType::Kind kind;

    switch (elit.tok->kind) {
      case LIT_NIL:
        kind = Nil;
        break;
      case LIT_TRUE:
      case LIT_FALSE:
        kind = Bool;
        break;
      case LIT_INT:
      case LIT_XINT:
      case LIT_BINT:
        kind = Int;
        break;
      case LIT_FLOAT:
        kind = Float;
        break;
      case LIT_STRING:
        kind = String;
        break;
      default:
        debug::bug("inference visitor: bad literal token");
        break;
    }

    vi->type = vi->alloc.emplace<BuiltinType>(kind);
  }

  void visit(const ast::ExprSymbol& esym, VisitInfo* raw) override
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

  void visit(const ast::ExprUnary& eun, VisitInfo* raw) override
  {
    auto* vi = InferVisitInfo::from(raw);

    UnaryVisitor uvis;
    UnaryVisitInfo uvi(vi->alloc, toUnaryOp(eun.op->kind));

    if (auto ir = Type::infer(vi->alloc, eun.expr)) {
      ir.value()->accept(uvis, &uvi);
    }

    vi->type = uvi.type;
  }

  void visit(const ast::ExprBinary&, VisitInfo* raw) override {}
  void visit(const ast::ExprGroup&, VisitInfo* raw) override {}
  void visit(const ast::ExprCall&, VisitInfo* raw) override {}
  void visit(const ast::ExprSubscript&, VisitInfo* raw) override {}
  void visit(const ast::ExprTuple&, VisitInfo* raw) override {}
  void visit(const ast::ExprLambda&, VisitInfo* raw) override {}

  void visit(const ast::TypeBuiltin& tbt, VisitInfo* raw) override
  {
    using enum Token::Kind;
    using enum BuiltinType::Kind;

    auto* vi = InferVisitInfo::from(raw);
    BuiltinType::Kind kind;

    switch (tbt.tok->kind) {
      case LIT_NIL:
        kind = Nil;
        break;
      case KW_INT:
        kind = Int;
        break;
      case KW_FLOAT:
        kind = Float;
        break;
      case KW_BOOL:
        kind = Bool;
        break;
      case KW_STRING:
        kind = String;
        break;
      default:
        debug::bug("unmapped builtin type token kind (wtf)");
    }

    vi->type = vi->alloc.emplace<BuiltinType>(kind);
  }

  void visit(const ast::TypeArray&, VisitInfo* raw) override {}
  void visit(const ast::TypeDict&, VisitInfo* raw) override {}
  void visit(const ast::TypeFunc&, VisitInfo* raw) override {}
};

Type::InferResult Type::infer(Allocator& alloc, const ast::Expr* expr)
{
  InferVisitor vis;
  InferVisitInfo raw(alloc);
  expr->accept(vis, &raw);

  if (raw.type == nullptr) {
    return std::unexpected(raw.error);
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
    return std::unexpected(raw.error);
  } else {
    return raw.type;
  }
}

}  // namespace sema

}  // namespace via
