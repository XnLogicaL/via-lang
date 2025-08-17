// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CLI_READ_FILE_H_
#define VIA_CLI_READ_FILE_H_

#include <via/types.h>
#include <fstream>
#include "context.h"
#include "panic.h"

namespace via {

namespace cli {

inline String read_file(const Context& ctx) {
  std::ifstream ifs(ctx.path);

  panic_assert(ifs.is_open(),
               fmt::format("no such file or directory: '{}'", ctx.path));

  via::String line;
  via::String content;

  while (std::getline(ifs, line)) {
    content += line + '\n';
  }

  return content;
}

}  // namespace cli

}  // namespace via

#endif
