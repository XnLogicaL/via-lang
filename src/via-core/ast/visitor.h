// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_AST_VISITOR_H_
#define VIA_CORE_AST_VISITOR_H_

#include <via/config.h>
#include <via/types.h>

namespace via {

namespace ast {

struct NodeExprLit;
struct NodeExprSym;
struct NodeExprUn;
struct NodeExprBin;
struct NodeExprGroup;
struct NodeExprCall;
struct NodeExprSubs;
struct NodeExprTuple;
struct NodeExprLambda;

struct NodeStmtVar;
struct NodeStmtScope;
struct NodeStmtIf;
struct NodeStmtFor;
struct NodeStmtForEach;
struct NodeStmtWhile;
struct NodeStmtAssign;
struct NodeStmtEnum;
struct NodeStmtEmpty;
struct NodeStmtExpr;

struct VisitInfo {
  u16 dst = 0;
};

namespace types {

template <typename T>
struct is_visit_info {
  static constexpr bool value =
      std::is_same_v<VisitInfo, T> || std::is_base_of_v<VisitInfo, T>;
};

template <typename T>
inline constexpr auto is_visit_info_v = is_visit_info<T>::value;

template <typename T>
concept visit_info = is_visit_info_v<T>;

}  // namespace types

class Visitor {
 public:
  virtual void visit(const NodeExprLit& elit, Box<VisitInfo> vi) = 0;
  virtual void visit(const NodeExprSym& esym, Box<VisitInfo> vi) = 0;
  virtual void visit(const NodeExprUn& eun, Box<VisitInfo> vi) = 0;
  virtual void visit(const NodeExprBin& ebin, Box<VisitInfo> vi) = 0;
  virtual void visit(const NodeExprGroup& egrp, Box<VisitInfo> vi) = 0;
  virtual void visit(const NodeExprCall& ecall, Box<VisitInfo> vi) = 0;
  virtual void visit(const NodeExprSubs& esubs, Box<VisitInfo> vi) = 0;
  virtual void visit(const NodeExprTuple& etup, Box<VisitInfo> vi) = 0;
  virtual void visit(const NodeExprLambda& elam, Box<VisitInfo> vi) = 0;

  virtual void visit(const NodeStmtVar&, Box<VisitInfo> vi) = 0;
  virtual void visit(const NodeStmtScope&, Box<VisitInfo> vi) = 0;
  virtual void visit(const NodeStmtIf&, Box<VisitInfo> vi) = 0;
  virtual void visit(const NodeStmtFor&, Box<VisitInfo> vi) = 0;
  virtual void visit(const NodeStmtForEach&, Box<VisitInfo> vi) = 0;
  virtual void visit(const NodeStmtWhile&, Box<VisitInfo> vi) = 0;
  virtual void visit(const NodeStmtAssign&, Box<VisitInfo> vi) = 0;
  virtual void visit(const NodeStmtEmpty&, Box<VisitInfo> vi) = 0;
  virtual void visit(const NodeStmtExpr&, Box<VisitInfo> vi) = 0;
};

template <types::visit_info Vi = VisitInfo, typename... Args>
Box<Vi> make_visit_info(Args&&... args) noexcept {
  return std::make_unique<Vi>(std::forward<Args>(args)...);
}

}  // namespace ast

}  // namespace via

#endif
