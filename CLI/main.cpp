// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include <via/via.h>
#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>

using namespace via;
using namespace argparse;

enum EmitKind {
  EK_NONE,
  EK_TTREE,
};

static EmitKind get_emit_kind(const char* str) {
  if (std::strcmp(str, "ttree") == 0)
    return EK_TTREE;

  return EK_NONE;
}

int main(int argc, char* argv[]) {
  spdlog::set_pattern("%^%l:%$ %v");

  ArgumentParser cli("via", VIA_VERSION);

  cli.add_argument("input").default_value("").help("Target source file");
  cli.add_argument("-e", "--emit")
    .nargs(1)
    .choices("list", "none", "ttree")
    .default_value("none")
    .help("Emission type");

  try {
    cli.parse_args(argc, argv);
  }
  catch (const std::exception& err) {
    spdlog::error(err.what());
  }

  auto input_path = cli.get("input");
  auto emit_type = cli.get("--emit");

  std::ifstream ifs(input_path);
  if (!ifs.is_open()) {
    spdlog::error("failed to open input path '{}': no such file or directory", input_path);
    return 1;
  }

  std::string input;
  std::string line;
  while (std::getline(ifs, line))
    input += line + '\n';

  std::cout << input << "\n";

  lex::FileBuf B(input.size() + 1);
  strcpy(B.data, input.c_str());

  lex::State L(B);
  auto token_buf = lex::lex(&L);

  return 0;
}
