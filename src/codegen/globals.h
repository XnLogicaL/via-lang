// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file globals.h
 * @brief Declares all necessary components for the compiler to track global variables
 */
#ifndef VIA_HAS_HEADER_GLOBALS_H
#define VIA_HAS_HEADER_GLOBALS_H

#include "common-defs.h"

#include <lex/token.h>
#include <parse/ast-base.h>
#include <interpreter/tvalue.h>

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

/**
 * @struct CompilerGlobal
 * @brief A pure-data structure that represents a global variable
 * @todo This structure needs to be redesigned
 */
struct CompilerGlobal {
  Token tok;
  std::string symbol;
  TypeNodeBase* type;
};

/**
 * @class GlobalHolder
 * @brief Contains globals
 * @todo This class needs to be redesigned
 */
class GlobalHolder final {
public:
  using index_query_result = std::optional<size_t>;
  using global_query_result = std::optional<CompilerGlobal>;
  using global_vector = std::vector<CompilerGlobal>;
  using builtin_vector = std::vector<CompilerGlobal>;

  inline GlobalHolder()
    : allocator(64 * 1024) {}

  /**
   * @brief Returns the amount of globals inside the container
   * @return Global amount
   */
  size_t size();

  /**
   * @brief Declares a new global variable
   * @param global Global to declare
   */
  void declare_global(CompilerGlobal global);

  /**
   * @brief Returns whether if the given global has been declared
   *
   * @param global Global to check for
   * @return Was global declared
   */
  bool was_declared(const CompilerGlobal& global);
  bool was_declared(const std::string& global_symbol);


  /**
   * @brief Returns the (nullable) index of the given global
   *
   * @param global Global to get index of
   * @return Global index query result
   */
  index_query_result get_index(const std::string& global_symbol);
  index_query_result get_index(const CompilerGlobal& global);

  /**
   * @brief Returns the (nullable) global that lives in the given position
   *
   * @param global Global to find
   * @return Global query result
   */
  global_query_result get_global(const std::string& global_symbol);
  global_query_result get_global(size_t global_index);

  /**
   * @brief Returns a constant reference to the internal global container.
   * @return Global vector reference
   */
  const global_vector& get();

  /**
   * @brief Declares built-in globals like `print`
   */
  void declare_builtins();

private:
  ArenaAllocator allocator;
  global_vector globals;
};

} // namespace via

/** @} */

#endif
