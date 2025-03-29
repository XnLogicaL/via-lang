// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _vl_globals_h
#define _vl_globals_h

#include "ast-base.h"
#include "object.h"
#include "common-defs.h"
#include "token.h"

namespace via {

struct global_obj {
  token token;
  std::string symbol;
  p_type_node_t type;
};

class global_holder final {
public:
  // Type aliases
  using index_query_result = std::optional<size_t>;
  using global_query_result = std::optional<global_obj>;
  using global_vector = std::vector<global_obj>;
  using builtin_vector = std::vector<global_obj>;

  // Returns the size of the global vector.
  size_t size();

  // Declares a new global.
  // Does not perform sanity checks.
  void declare_global(global_obj);

  // Returns whether if a global has been declared.
  bool was_declared(const global_obj&);
  bool was_declared(const std::string&);

  // Returns the index of a given global.
  index_query_result get_index(const std::string&);
  index_query_result get_index(const global_obj&);

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

} // namespace via

#endif
