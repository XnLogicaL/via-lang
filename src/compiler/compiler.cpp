// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "compiler.h"
#include "state.h"
#include "string-utility.h"

// ===========================================================================================
// compiler.cpp
//
VIA_NAMESPACE_BEGIN

bool Compiler::generate() {
  RegisterAllocator allocator(VIA_REGISTER_COUNT, true);
  ErrorBus          emitter;
  StmtVisitor       visitor(unit_ctx, emitter, allocator);

  for (pStmtNode& stmt : unit_ctx.ast->statements) {
    stmt->accept(visitor);
  }

  if (check_global_collisions()) {
    return true;
  }

  unit_ctx.internal.label_count = visitor.label_counter;

  return visitor.failed();
}

bool Compiler::check_global_collisions() {
  ErrorBus emitter;

  bool failed = false;

  std::unordered_map<uint32_t, Global> global_map;

  for (const Global& global : unit_ctx.internal.globals->get()) {
    uint32_t hash = hash_string_custom(global.symbol.c_str());
    auto     it   = global_map.find(hash);

    if (it != global_map.end()) {
      failed = true;

      emitter.log(
          {false,
           std::format(
               "Global identifier '{}' collides with global identifier '{}'",
               global.symbol,
               it->second.symbol
           ),
           unit_ctx,
           CompilerErrorLevel::ERROR_,
           global.token}
      );

      emitter.log(
          {false,
           std::format("Global '{}' declared here", it->second.symbol),
           unit_ctx,
           CompilerErrorLevel::INFO,
           it->second.token}
      );

      emitter.log(
          {true,
           "This limitation is due to a 32-bit bitspace used to identify "
           "globals during runtime. To fix it, try renaming either global to a non-related "
           "identifier.",
           unit_ctx,
           CompilerErrorLevel::INFO,
           {}}
      );
    }

    global_map[hash] = global;
  }

  return failed;
}

VIA_NAMESPACE_END
