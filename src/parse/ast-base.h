// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file ast-base.h
 * @brief Base classes for abstract syntax tree nodes
 */
#ifndef VIA_HAS_HEADER_AST_BASE_H
#define VIA_HAS_HEADER_AST_BASE_H

#include "common.h"
#include <lex/token.h>
#include <interpreter/instruction.h>

// MSVC is annoying with uninitialized members
#if VIA_COMPILER == C_MSVC
#pragma warning(push)
#pragma warning(disable : 26495)
#endif

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

class TransUnitContext;
class NodeVisitorBase;
struct ExprNodeBase;
struct StmtNodeBase;
struct TypeNodeBase;

/**
 * @struct StmtModifiers
 * @brief Contains modifier flags for a statement or type
 */
struct StmtModifiers {
  bool is_const; ///< Has constant modifier

  /**
   * @brief Returns the modifiers as a string.
   * @return std::string String
   */
  std::string to_string() const;
};

/**
 * @struct StmtAttribute
 * @brief A single statement attribute
 */
struct StmtAttribute {
  Token identifier;             ///< Attribute identifier token
  std::vector<Token> arguments; ///< A list of arguments to the attribute

  /**
   * @brief Returns the attribute as a string.
   * @return std::string String
   */
  std::string to_string() const;
};

/**
 * @struct ExprNodeBase
 * @brief Base class for expression nodes
 */
struct ExprNodeBase {
  size_t begin; ///< Start location of the expression as absolute offset
  size_t end;   ///< End location of the expression as absolute offset

  virtual ~ExprNodeBase() = default;

  /**
   * @brief Returns the expression as a string.
   * @param depth Tree depth
   * @return std::string String
   */
  virtual std::string to_string(uint32_t& depth) = 0;

  /**
   * @brief Invokes the visitor to visit this node
   * @param visitor Visitor object
   * @param dst Destination register
   */
  virtual void accept(NodeVisitorBase& visitor, operand_t dst) = 0;

  /**
   * @brief Infers the type of the expression node
   * @param unit_ctx Translation unit context
   * @return TypeNodeBase* Pointer to newly allocated type object
   */
  virtual TypeNodeBase* infer_type(TransUnitContext& unit_ctx) = 0;

  /**
   * @brief Returns the precedence of the expression node
   * @return int Precedence
   */
  virtual int precedence() const {
    return 0;
  }
};

/**
 * @struct StmtNodeBase
 * @brief Base class for statement nodes
 */
struct StmtNodeBase {
  size_t begin; ///< Start location of the statement as an absolute offset
  size_t end;   ///< End location of the statement as an absolute offset
  std::vector<StmtAttribute> attributes{}; ///< Statement attribute list

  virtual ~StmtNodeBase() = default;

  /**
   * @brief Returns the statement as a string.
   * @param depth Tree depth
   * @return std::string String
   */
  virtual std::string to_string(uint32_t& depth) = 0;
  virtual void accept(NodeVisitorBase&) = 0;
};

struct TypeNodeBase {
  size_t begin;
  size_t end;
  ExprNodeBase* expression = nullptr;

  virtual ~TypeNodeBase() = default;

  /**
   * @brief Returns the expression as a string.
   * @param depth Tree depth
   * @return std::string String
   */
  virtual std::string to_string(uint32_t& depth) = 0;
  virtual std::string to_output_string() = 0;
  virtual void decay(NodeVisitorBase&, TypeNodeBase*&){};
};

} // namespace via

#if VIA_COMPILER == C_MSVC
#pragma warning(pop)
#endif

/** @} */

#endif
