// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_AST_BASE_H
#define VIA_HAS_HEADER_AST_BASE_H

#include "common-includes.h"
#include "common-macros.h"
#include "token.h"

// MSVC is annoying with uninitialized members
#if VIA_COMPILER == C_MSVC
#pragma warning(push)
#pragma warning(disable : 26495)
#endif

namespace via {

class TransUnitContext;

class NodeVisitorBase;
struct ExprNodeBase;
struct StmtNodeBase;
struct TypeNodeBase;

struct StmtModifiers {
  bool is_const;

  std::string to_string() const;
};

struct StmtAttribute {
  Token identifier;
  std::vector<Token> arguments;

  std::string to_string() const;
};

struct ExprNodeBase {
  size_t begin;
  size_t end;

  virtual ~ExprNodeBase() = default;

  virtual std::string to_string(uint32_t&) = 0;
  virtual void accept(NodeVisitorBase&, uint32_t) = 0;
  virtual TypeNodeBase* infer_type(TransUnitContext&) = 0;
  virtual int precedence() const {
    return 0;
  }
};

struct StmtNodeBase {
  size_t begin;
  size_t end;
  std::vector<StmtAttribute> attributes{};

  virtual ~StmtNodeBase() = default;

  virtual std::string to_string(uint32_t&) = 0;
  virtual void accept(NodeVisitorBase&) = 0;
};

struct TypeNodeBase {
  size_t begin;
  size_t end;
  ExprNodeBase* expression = nullptr;

  virtual ~TypeNodeBase() = default;

  virtual std::string to_string(uint32_t&) = 0;
  virtual std::string to_output_string() = 0;
  virtual void decay(NodeVisitorBase&, TypeNodeBase*&) {};
};

} // namespace via

#if VIA_COMPILER == C_MSVC
#pragma warning(pop)
#endif

#endif
