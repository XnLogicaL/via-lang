// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "ast.h"

namespace via {

inline usize DEFAULT_DEPTH = 0;

#define TAB                 String(depth * 4, ' ')
#define FORMAT(...)         TAB + std::format(__VA_ARGS__)
#define TRY_COERCE(T, a, b) (const T* a = dynamic_cast<const T*>(b))

template<typename T, typename F = Function<String(const T& __t)>>
static String __vec_to_string(const Vec<T>& __v, F __f) {
  std::ostringstream oss;
  oss << "{";

  for (const T& __t : __v)
    oss << __f(__t) << ", ";

  oss << "}";
  return oss.str();
}

static String tpb_to_string(const TupleBinding* tpb) {
  usize depth = 0;
  return FORMAT("TupleBinding[{}]", __vec_to_string(tpb->binds, [](const auto& e) {
                  return e->get_dump(DEFAULT_DEPTH);
                }));
}

static String lvalue_to_string(const LValue* lval) {
  if (lval->kind == LValue::Symbol)
    return lval->sym->get_dump(DEFAULT_DEPTH);
  else if (lval->kind == LValue::Tpb)
    return tpb_to_string(lval->tpb);

  std::unreachable();
}

// --------------------------------------------------
// NodeExprLit
// --------------------------------------------------

String NodeExprLit::get_dump(usize& depth) const {
  return FORMAT("NodeExprLit({}, {})", magic_enum::enum_name(tok->kind), tok->to_string());
}

// --------------------------------------------------
// NodeExprSym
// --------------------------------------------------

String NodeExprSym::get_dump(usize& depth) const {
  return FORMAT("NodeExprSym({})", tok->to_string());
}

// --------------------------------------------------
// NodeExprUn
// --------------------------------------------------

String NodeExprUn::get_dump(usize& depth) const {
  return FORMAT("NodeExprUn({}, {})", op->to_string(), expr->get_dump(DEFAULT_DEPTH));
}

// --------------------------------------------------
// NodeExprBin
// --------------------------------------------------

String NodeExprBin::get_dump(usize& depth) const {
  return FORMAT(
    "NodeExprBin({}, {}, {})",
    op->to_string(),
    lhs->get_dump(DEFAULT_DEPTH),
    rhs->get_dump(DEFAULT_DEPTH)
  );
}

// --------------------------------------------------
// NodeExprGroup
// --------------------------------------------------

String NodeExprGroup::get_dump(usize& depth) const {
  return FORMAT("NodeExprGroup({})", expr->get_dump(DEFAULT_DEPTH));
}

// --------------------------------------------------
// NodeExprCall
// --------------------------------------------------

String NodeExprCall::get_dump(usize& depth) const {
  return FORMAT(
    "NodeExprCall({}, {})",
    lval->get_dump(DEFAULT_DEPTH),
    __vec_to_string<ExprNode*>(
      args, [](ExprNode* const& val) { return val->get_dump(DEFAULT_DEPTH); }
    )
  );
}

// --------------------------------------------------
// NodeExprSubs
// --------------------------------------------------

String NodeExprSubs::get_dump(usize& depth) const {
  return FORMAT("NodeExprSubs({}, {})", lval->get_dump(depth), idx->get_dump(depth));
}

// --------------------------------------------------
// NodeExprTuple
// --------------------------------------------------

String NodeExprTuple::get_dump(usize& depth) const {
  return FORMAT("NodeExprTuple({})", __vec_to_string(vals, [](const auto& expr) {
                  return expr->get_dump(DEFAULT_DEPTH);
                }));
}

// --------------------------------------------------
// NodeStmtScope
// --------------------------------------------------

String NodeStmtScope::get_dump(usize& depth) const {
  std::ostringstream oss;
  oss << FORMAT("NodeStmtScope()\n");
  depth++;

  for (const auto& stmt : stmts)
    oss << stmt->get_dump(depth) << "\n";

  depth--;
  oss << FORMAT("End()");
  return oss.str();
}

// --------------------------------------------------
// NodeStmtVar
// --------------------------------------------------

String NodeStmtVar::get_dump(usize& depth) const {
  String rvalue = rval->get_dump(DEFAULT_DEPTH);
  String lvalue =
    lval->kind == LValue::Symbol ? lval->sym->get_dump(DEFAULT_DEPTH) : tpb_to_string(lval->tpb);

  return FORMAT("NodeStmtVar({}, {})", lvalue, rvalue);
}

// --------------------------------------------------
// NodeStmtIf
// --------------------------------------------------

String NodeStmtIf::get_dump(usize& depth) const {
  std::ostringstream oss;
  oss << "NodeStmtIf()\n";
  depth++;

  for (const auto& br : brs) {
    oss << FORMAT("Branch({})\n", br.cnd->get_dump(DEFAULT_DEPTH));
    depth++;

    for (StmtNode* const& stmt : br.br->stmts)
      oss << stmt->get_dump(depth) << "\n";

    depth--;
    oss << FORMAT("End()\n");
  }

  depth--;
  oss << FORMAT("End()");
  return oss.str();
}

// --------------------------------------------------
// NodeStmtFor
// --------------------------------------------------

String NodeStmtFor::get_dump(usize& depth) const {
  std::ostringstream oss;
  oss << FORMAT(
    "NodeStmtFor({}, {}, {})\n",
    init->get_dump(DEFAULT_DEPTH),
    target->get_dump(DEFAULT_DEPTH),
    step->get_dump(DEFAULT_DEPTH)
  );
  depth++;

  for (const auto& stmt : br->stmts)
    oss << stmt->get_dump(depth) << "\n";

  depth--;
  oss << FORMAT("End()");
  return oss.str();
}

// --------------------------------------------------
// NodeStmtForEach
// --------------------------------------------------

String NodeStmtForEach::get_dump(usize& depth) const {
  std::ostringstream oss;
  oss << FORMAT("NodeStmtForEach({}, {})\n", lvalue_to_string(lval), iter->get_dump(DEFAULT_DEPTH));
  depth++;

  for (const auto& stmt : br->stmts)
    oss << stmt->get_dump(depth) << "\n";

  depth--;
  oss << FORMAT("End()");
  return oss.str();
}

// --------------------------------------------------
// NodeStmtWhile
// --------------------------------------------------

String NodeStmtWhile::get_dump(usize& depth) const {
  std::ostringstream oss;
  oss << FORMAT("NodeStmtWhile({})\n", cnd->get_dump(DEFAULT_DEPTH));
  depth++;

  for (StmtNode* const& stmt : br->stmts)
    oss << stmt->get_dump(depth) << "\n";

  depth--;
  oss << FORMAT("End()");
  return oss.str();
}

// --------------------------------------------------
// NodeStmtEmpty
// --------------------------------------------------

String NodeStmtEmpty::get_dump(usize& depth) const {
  return FORMAT("NodeStmtEmpty()");
}

// --------------------------------------------------
// NodeStmtExpr
// --------------------------------------------------

String NodeStmtExpr::get_dump(usize& depth) const {
  return FORMAT("NodeStmtExpr({})", expr->get_dump(depth));
}

void dump_stmt(StmtNode* stmt, usize& depth) {
  std::cout << stmt->get_dump(depth) << "\n";
}

} // namespace via
