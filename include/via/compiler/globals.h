// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_GLOBALS_H
#define _VIA_GLOBALS_H

#include "ast-base.h"
#include "object.h"
#include "common-defs.h"
#include "token.h"

VIA_NAMESPACE_BEGIN

struct Global {
  Token token;
  std::string symbol;
  pTypeNode type;
};

class GlobalTracker final {
public:
  // Type aliases
  using index_query_result = std::optional<size_t>;
  using global_query_result = std::optional<Global>;
  using global_vector = std::vector<Global>;
  using builtin_vector = std::vector<Global>;

  // Returns the size of the global vector.
  size_t size();

  // Declares a new global.
  // Does not perform sanity checks.
  void declare_global(Global);

  // Returns whether if a global has been declared.
  bool was_declared(const Global&);
  bool was_declared(const std::string&);

  // Returns the index of a given global.
  index_query_result get_index(const std::string&);
  index_query_result get_index(const Global&);

  // Returns the global at a given key or index.
  global_query_result get_global(const std::string&);
  global_query_result get_global(size_t);

  // Returns a constant reference to the global vector.
  const global_vector& get();

  // Declares all assumed builtins.
  void declare_builtins();

private:
  global_vector globals;
};

VIA_NAMESPACE_END

#endif
