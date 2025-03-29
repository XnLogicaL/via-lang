// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_AST_BASE_H
#define _VIA_AST_BASE_H

#include "token.h"

VIA_NAMESPACE_BEGIN

class TransUnitContext;

class NodeVisitor;
struct ExprNode;
struct StmtNode;
struct TypeNode;

using pExprNode = std::unique_ptr<ExprNode>;
using pStmtNode = std::unique_ptr<StmtNode>;
using pTypeNode = std::unique_ptr<TypeNode>;

struct Modifiers {
  bool is_const;

  std::string to_string() const;
};

struct Attribute {
  Token identifier;
  std::vector<Token> arguments;

  std::string to_string() const;
};

struct ExprNode {
  size_t begin;
  size_t end;

  virtual VIA_DEFAULT_DESTRUCTOR(ExprNode);

  virtual std::string to_string(uint32_t&) = 0;
  virtual void accept(NodeVisitor&, uint32_t) = 0;
  virtual pExprNode clone() = 0;
  virtual pTypeNode infer_type(TransUnitContext&) = 0;
  virtual int precedence() const {
    return 0;
  }
};

struct StmtNode {
  std::vector<Attribute> attributes{};

  virtual VIA_DEFAULT_DESTRUCTOR(StmtNode);

  virtual std::string to_string(uint32_t&) = 0;
  virtual void accept(NodeVisitor&) = 0;
  virtual pStmtNode clone() = 0;
};

struct TypeNode {
  ExprNode* expression = nullptr;

  virtual VIA_DEFAULT_DESTRUCTOR(TypeNode);

  virtual std::string to_string(uint32_t&) = 0;
  virtual std::string to_string_x() = 0;
  virtual void decay(NodeVisitor&, pTypeNode&) {};
  virtual pTypeNode clone() = 0;
};

VIA_NAMESPACE_END

#endif
