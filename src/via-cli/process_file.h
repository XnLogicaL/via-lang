// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CLI_PROCESS_FILE_H_
#define VIA_CLI_PROCESS_FILE_H_

#include <via/via.h>
#include "context.h"
#include "read_file.h"

namespace via {

namespace cli {

enum class EmitType {
  none,
  ttree,
  ast,
  header,
};

inline void process_file(const Context& ctx) {
  String input = read_file(ctx);
  Vec<char> file_buf(input.begin(), input.end());
  Diagnostics diag_ctx{ctx.path, file_buf};

  Lexer lexer(file_buf);
  auto token_buf = lexer.tokenize();

  Parser parser(file_buf, token_buf, diag_ctx);
  auto ast_buf = parser.parse();

  Header header;

  if (diag_ctx.has_errors())
    goto has_errors;

  {
    Generator gen(ast_buf, diag_ctx);
    header = gen.generate();
  }

has_errors:
  diag_ctx.emit();
  diag_ctx.clear();

  switch (magic_enum::enum_cast<EmitType>(ctx.emit).value_or(EmitType::none)) {
    case EmitType::ttree:
      for (const Token* tok : token_buf)
        fmt::println("{}", tok->get_dump());
      break;
    case EmitType::ast:
      usize depth;
      for (const ast::StmtNode* node : ast_buf)
        fmt::println("{}", node->get_dump(depth));
      break;
    case EmitType::header:
      fmt::println("{}", header.get_dump());
      break;
    default:
      break;
  }
}

}  // namespace cli

}  // namespace via

#endif
