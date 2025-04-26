// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include <linenoise.hpp>
#include <argparse/argparse.hpp>
#include <via/via.h>
#include <utility/file-io.h>

#define SET_PROFILER_POINT(id)     [[maybe_unused]] const auto id = std::chrono::steady_clock::now();
#define GET_PROFILER_DIFF_MS(l, r) std::chrono::duration<double, std::milli>(r - l).count()

namespace handlers {

using namespace argparse;
using namespace via;

struct CompileResult {
  bool failed;
  TransUnitContext unit;
};

static inline CErrorBus err_bus;
static inline TransUnitContext dummy_unit_ctx("<unavailable>", "");

std::unique_ptr<ArgumentParser> get_standard_parser(std::string name);

CompileResult handle_compile([[maybe_unused]] argparse::ArgumentParser& subcommand_parser);
CompileResult handle_run(argparse::ArgumentParser& subcommand_parser);

void handle_repl(argparse::ArgumentParser&);
void handle_debugger(argparse::ArgumentParser& parser);

} // namespace handlers
