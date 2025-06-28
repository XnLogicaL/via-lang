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
  EK_LIST,
  EK_TTREE,
};

static EmitKind get_emit_kind(const char* str) {
  if (std::strcmp(str, "list") == 0)
    return EK_LIST;
  else if (std::strcmp(str, "ttree") == 0)
    return EK_TTREE;

  return EK_NONE;
}

static void list_emit_kinds() {
  spdlog::info("available emit targets:");
  std::cout << "  -e list          opens this menu\n";
  std::cout << "  -e ttree         dumps token tree\n";
}

int main(int argc, char* argv[]) {
  spdlog::set_pattern("%^%l:%$ %v");

  ArgumentParser cli("via", VIA_VERSION);

  cli.add_argument("input").default_value("").help("Target source file");
  cli.add_argument("--emit", "-e")
    .nargs(1)
    .choices("none", "list", "ttree")
    .default_value("none")
    .help("Emission type");

  try {
    cli.parse_args(argc, argv);
  }
  catch (const std::exception& err) {
    spdlog::error(err.what());
    return 1;
  }

  auto input_path = cli.get("input");
  auto emit_type = cli.get("--emit");

  if (input_path.empty())
    goto no_input_files;

  {
    std::ifstream ifs(input_path);
    if (!ifs.is_open()) {
      spdlog::error("no such file or directory: '{}'", input_path);
      goto no_input_files;
    }

    {
      std::string input;
      std::string line;

      while (std::getline(ifs, line))
        input += line + '\n';

      FileBuf file_buf(input.c_str(), input.c_str() + input.size() + 1);
      LexState L(file_buf);
      TokenBuf token_buf = lexer_tokenize(L);

      auto ek_raw = cli.get("--emit");
      auto ek = get_emit_kind(ek_raw.c_str());

      switch (ek) {
      case EK_LIST:
        list_emit_kinds();
        break;
      case EK_TTREE:
        dump_ttree(token_buf);
        break;
      default:
        break;
      }
    }
  }

  return 0;

no_input_files:
  spdlog::error("no input files");
  return 1;
}
