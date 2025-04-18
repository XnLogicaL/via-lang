// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_GLOBALS_H
#define VIA_HAS_HEADER_GLOBALS_H

#include "ast-base.h"
#include "tvalue.h"
#include "common-defs.h"
#include "token.h"

namespace via {

struct CompilerGlobal {
  Token tok;
  std::string symbol;
  TypeNodeBase* type;
};

class GlobalHolder final {
public:
  // Type aliases
  using index_query_result = std::optional<size_t>;
  using global_query_result = std::optional<CompilerGlobal>;
  using global_vector = std::vector<CompilerGlobal>;
  using builtin_vector = std::vector<CompilerGlobal>;

  inline GlobalHolder()
    : allocator(64 * 1024) {}

  // Returns the size of the global vector.
  size_t size();

  // Declares a new global.
  // Does not perform sanity checks.
  void declare_global(CompilerGlobal);

  // Returns whether if a global has been declared.
  bool was_declared(const CompilerGlobal&);
  bool was_declared(const std::string&);

  // Returns the index of a given global.
  index_query_result get_index(const std::string&);
  index_query_result get_index(const CompilerGlobal&);

  // Returns the global at a given key or index.
  global_query_result get_global(const std::string&);
  global_query_result get_global(size_t);

  // Returns a constant reference to the global vector.
  const global_vector& get();

  // Declares all assumed builtins.
  void declare_builtins();

private:
  ArenaAllocator allocator;
  global_vector globals;
};

} // namespace via

#endif
