// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

// thirdparty
#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>

// via
#include <lexer/lexer.h>
#include <parser/parser.h>

using namespace via;
using namespace argparse;

enum EmitKind {
  EK_NONE,
  EK_LIST,
  EK_TTREE,
  EK_AST,
};

static EmitKind get_emit_kind(const char* str) {
  if (std::strcmp(str, "list") == 0)
    return EK_LIST;
  else if (std::strcmp(str, "ttree") == 0)
    return EK_TTREE;
  else if (std::strcmp(str, "ast") == 0)
    return EK_AST;

  return EK_NONE;
}

static void list_emit_kinds() {
  spdlog::info("available emit targets:");
  std::cout << "  -e list          opens this menu\n";
  std::cout << "  -e ttree         dumps token tree\n";
}

static bool read_file(const String& path, String& out_content) {
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    spdlog::error("no such file or directory: '{}'", path);
    return false;
  }

  String line;
  out_content.clear();

  while (std::getline(ifs, line)) {
    out_content += line + '\n';
  }

  return true;
}

static void process_file(const String& input_path, EmitKind emit_kind) {
  String input;
  if (!read_file(input_path, input)) {
    spdlog::error("failed to read input file");
    return;
  }

  core::FileBuf file_buf(input.c_str(), input.c_str() + input.size() + 1);
  core::DiagContext diag_ctx{input_path, file_buf};

  core::lex::LexState lex_state(file_buf);
  core::TokenBuf token_buf = lexer_tokenize(lex_state);

  core::parser::ParseState parse_state(lex_state, token_buf, diag_ctx);
  core::AstBuf ast_buf = parser_parse(parse_state);

  // check for errors
  Vec<const core::Diagnosis*> diags;
  if ((diags = diag_filter(
         diag_ctx, [](const core::Diagnosis& diag) { return diag.kind == core::DK_ERROR; }
       ),
       diags.empty()))
    goto end;

end:
  diag_emit(diag_ctx);
  diag_clear(diag_ctx);

  switch (emit_kind) {
  case EK_LIST:
    list_emit_kinds();
    break;
  case EK_TTREE:
    core::lex::dump_ttree(token_buf);
    break;
  case EK_AST:
    core::parser::dump_ast(ast_buf);
    break;
  default:
    break;
  }
}

int main(int argc, char* argv[]) {
  spdlog::set_pattern("%^%l:%$ %v");

  ArgumentParser cli("via", VIA_VERSION);

  cli.add_argument("input").default_value("").help("Target source file");
  cli.add_argument("--emit", "-e")
    .nargs(1)
    .choices("none", "list", "ttree", "ast")
    .default_value("none")
    .help("Emission type");

  try {
    cli.parse_args(argc, argv);
  }
  catch (const std::exception& err) {
    spdlog::error(err.what());
    return 1;
  }

  String input_path = cli.get("input");
  String emit_type = cli.get("--emit");

  if (input_path.empty()) {
    spdlog::error("no input files");
    return 1;
  }

  EmitKind ek = get_emit_kind(emit_type.c_str());
  process_file(input_path, ek);
  return 0;
}
