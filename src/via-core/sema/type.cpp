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

  static UnaryVisitInfo* from(VisitInfo* raw_vi)
  {
    if TRY_COERCE (UnaryVisitInfo, uvi, raw_vi) {
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

  static BinaryVisitInfo* from(VisitInfo* raw_vi)
  {
    if TRY_COERCE (BinaryVisitInfo, bvi, raw_vi) {
      return bvi;
    } else {
      debug::bug("Invalid visit info passed to unary visitor");
    }
  }
};

struct UnaryVisitor : TypeVisitor
{
  void visit(const BuiltinType& bt, VisitInfo* raw_vi) override
  {
    using enum BuiltinType::Kind;

    auto* vi = UnaryVisitInfo::from(raw_vi);

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

  static InferVisitInfo* from(VisitInfo* raw_vi)
  {
    if (auto* info = dynamic_cast<InferVisitInfo*>(raw_vi)) {
      return info;
    } else {
      debug::bug("Invalid visit info passed to inference visitor");
    }
  }
};

struct InferVisitor : Visitor
{
  void visit(const ExprLit& elit, VisitInfo* raw_vi) override
  {
    using enum Token::Kind;
    using enum BuiltinType::Kind;

    auto* vi = InferVisitInfo::from(raw_vi);
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

  void visit(const ExprSymbol& esym, VisitInfo* raw_vi) override
  {
    auto* vi = InferVisitInfo::from(raw_vi);
    Frame& frame = stack::top();
    StringView symbol = esym.sym->to_string_view();

    if (auto lref = frame.get_local(symbol)) {
      lref->local.get_type()->accept(*this, raw_vi);
    } else {
      debug::bug("inference visitor: symbol lookup failed");
    }
  }

  void visit(const ExprUnary& eun, VisitInfo* raw_vi) override
  {
    auto* vi = InferVisitInfo::from(raw_vi);

    UnaryVisitor uvis;
    UnaryVisitInfo uvi(vi->alloc, to_unary_op(eun.op->kind));

    if (auto ir = Type::infer(vi->alloc, eun.expr)) {
      ir.value()->accept(uvis, &uvi);
    }

    vi->type = uvi.type;
  }

  void visit(const ExprBinary&, VisitInfo* raw_vi) override {}
  void visit(const ExprGroup&, VisitInfo* raw_vi) override {}
  void visit(const ExprCall&, VisitInfo* raw_vi) override {}
  void visit(const ExprSubscript&, VisitInfo* raw_vi) override {}
  void visit(const ExprTuple&, VisitInfo* raw_vi) override {}
  void visit(const ExprLambda&, VisitInfo* raw_vi) override {}

  void visit(const TypeBuiltin&, VisitInfo* raw_vi) override {}
  void visit(const TypeArray&, VisitInfo* raw_vi) override {}
  void visit(const TypeDict&, VisitInfo* raw_vi) override {}
  void visit(const TypeFunc&, VisitInfo* raw_vi) override {}
};

Type::InferResult Type::infer(Allocator& alloc, const ast::Expr* expr)
{
  InferVisitor vis;
  InferVisitInfo raw_vi(alloc);
  expr->accept(vis, &raw_vi);

  if (raw_vi.type == nullptr) {
    return std::unexpected(raw_vi.fail);
  } else {
    return raw_vi.type;
  }
}

Type::InferResult Type::from(Allocator& alloc, const ast::Type* type)
{
  InferVisitor vis;
  InferVisitInfo raw_vi(alloc);
  type->accept(vis, &raw_vi);

  if (raw_vi.type == nullptr) {
    return std::unexpected(raw_vi.fail);
  } else {
    return raw_vi.type;
  }
}

}  // namespace sema

}  // namespace via
