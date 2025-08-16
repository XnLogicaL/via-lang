// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include <spdlog/spdlog.h>
#include <via/via.h>
#include <argparse/argparse.hpp>
#include <fstream>

#define CLI_ASSERT(cond, msg) \
  if (!(cond)) {              \
    spdlog::error(msg);       \
    return -1;                \
  }

using via::String;
using via::Vec;
using namespace argparse;

enum class EmitType {
  none,
  ttree,
  ast,
};

static EmitType get_emit_kind(const char* str) {
  auto emit = magic_enum::enum_cast<EmitType>(str);
  return emit.value_or(EmitType::none);
}

static bool read_file(const String& path, String& out_content) {
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    spdlog::error("no such file or directory: '{}'", path);
    return false;
  }

  String line;
  out_content.clear();

  while (std::getline(ifs, line))
    out_content += line + '\n';

  return true;
}

static void process_file(const String& input_path, EmitType emit_kind) {
  String input;
  if (!read_file(input_path, input)) {
    spdlog::error("failed to read input file");
    return;
  }

  via::FileBuf file_buf(input.c_str(), input.c_str() + input.size() + 1);
  via::Diagnostics diag_ctx{input_path, file_buf};

  via::Lexer lexer(file_buf);
  auto token_buf = lexer.tokenize();

  via::Parser parser(file_buf, token_buf, diag_ctx);
  auto ast_buf = parser.parse();

  // check for errors
  Vec<via::Diagnosis> diags;
  if ((diags = diag_ctx.collect(
           [](const auto& diag) { return diag.kind == via::Diag::Error; }),
       diags.empty()))
    return;

  diag_ctx.emit();
  diag_ctx.clear();

  switch (emit_kind) {
    case EmitType::ttree:
      fmt::println("{}", token_buf);
      break;
    case EmitType::ast:
      via::dump_ast(ast_buf);
      break;
    default:
      break;
  }
}

int main(int argc, char* argv[]) {
  spdlog::set_pattern("%^%l:%$ %v");

  ArgumentParser cli("via", fmt::format("{}", via::get_semantic_version()));

  cli.add_argument("input").default_value("").help("Target source file");
  cli.add_argument("--emit", "-e")
      .nargs(1)
      .choices("none", "list", "ttree", "ast")
      .default_value("none")
      .help("Emission type");

  String path, emit;

  try {
    cli.parse_args(argc, argv);
    path = cli.get("input");
    emit = cli.get("--emit");
  } catch (const std::bad_any_cast&) {
    CLI_ASSERT(false, "bad emission type")
  } catch (const std::exception& err) {
    CLI_ASSERT(false, err.what())
  }

  CLI_ASSERT(!path.empty(), "no input file");
  process_file(path, get_emit_kind(emit.c_str()));
  return 0;
}
