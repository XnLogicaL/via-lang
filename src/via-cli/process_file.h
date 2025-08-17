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
  FileBuf file_buf(input.c_str(), input.c_str() + input.size() + 1);
  Diagnostics diag_ctx{ctx.path, file_buf};

  Lexer lexer(file_buf);
  auto token_buf = lexer.tokenize();

  Parser parser(file_buf, token_buf, diag_ctx);
  auto ast_buf = parser.parse();

  Generator gen(ast_buf, diag_ctx);
  auto header = gen.generate();

  // check for errors
  auto diags = diag_ctx.get_diagnostics();
  auto pred = [](const Diagnosis& diag) {
    return diag.kind == Diagnosis::Kind::Error;
  };

  if (auto it = std::ranges::find_if(diags, pred); it != diags.end())
    return;

  diag_ctx.emit();
  diag_ctx.clear();

  switch (magic_enum::enum_cast<EmitType>(ctx.emit).value_or(EmitType::none)) {
    case EmitType::ttree:
      fmt::println("{}", Convert<TokenBuf>::to_string(token_buf));
      break;
    case EmitType::ast:
      fmt::println("{}", Convert<AstBuf>::to_string(ast_buf));
      break;
    case EmitType::header:
      fmt::println("{}", Convert<Header>::to_string(header));
      break;
    default:
      break;
  }
}

}  // namespace cli

}  // namespace via

#endif
