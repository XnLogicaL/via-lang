/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <CLI/CLI.hpp>
#include <via/via.hpp>

namespace via {
namespace cli {

static struct DumpValidator: public CLI::Validator
{
    DumpValidator()
    {
        name_ = "dump-mode";
        func_ = [](const std::string& str) -> std::string {
            static const std::vector<std::string> valid_modes{
                "token-tree",
                "ast",
                "ir",
                "executable",
                "def-table",
                "symbol-table",
                "import-dirs"
            };

            // Validate each comma-separated token individually
            std::istringstream ss(str);
            std::string token;
            while (std::getline(ss, token, ',')) {
                if (std::find(valid_modes.begin(), valid_modes.end(), token) ==
                    valid_modes.end()) {
                    return std::format("unknown dump mode '{}'", token);
                }
            }
            return {};
        };
    }
} dump_validator;

struct ProgramOptions
{
    uint8_t verbosity = 0;
    bool no_execute = false;
    bool debugger = false;
    std::filesystem::path input;
    std::set<std::string> dump;
    std::vector<std::string> imports;

    inline std::string to_string() const noexcept
    {
        return std::format(
            "ProgramOptions:\n"
            "  verbosity:   {}\n"
            "  no_execute:  {}\n"
            "  debugger:    {}\n"
            "  input:       {}\n"
            "  dump:        [{}]\n"
            "  imports:     [{}]",
            verbosity,
            no_execute,
            debugger,
            input.string(),
            dump.empty() ? ""
                         : std::accumulate(
                               std::next(dump.begin()),
                               dump.end(),
                               *dump.begin(),
                               [](std::string a, const std::string& b) {
                                   return std::move(a) + ", " + b;
                               }
                           ),
            imports.empty() ? ""
                            : std::accumulate(
                                  std::next(imports.begin()),
                                  imports.end(),
                                  *imports.begin(),
                                  [](std::string a, const std::string& b) {
                                      return std::move(a) + ", " + b;
                                  }
                              )
        );
    }
};

inline CLI::App& initialize_app(ProgramOptions& options) noexcept
{
    static CLI::App app("via Compiler CLI");

    app.failure_message([](auto, const CLI::Error& e) {
        return std::format(
            "{}{}\n",
            via::ansi::format(
                "error: ",
                via::ansi::Foreground::RED,
                via::ansi::Background::NONE,
                via::ansi::Style::BOLD
            ),
            e.what()
        );
    });

    app.add_option("input", options.input, "Input file path")
        ->required()
        ->check(CLI::ExistingFile);

    app.add_option("--dump,-D", options.dump, "Compilation dump mode(s)")
        ->check(dump_validator)
        ->delimiter(',')
        ->type_name("MODE")
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);

    app.add_option("--verbosity,-V", options.verbosity, "Controls verbosity level (0–3)")
        ->default_val(0);

    app.add_option(
           "--import-dir,-I",
           options.imports,
           "Comma-separated list of import directories"
    )
        ->delimiter(',');

    app.add_flag("--no-execute", options.no_execute, "Disables sequential execution");
    app.add_flag("--debugger", options.debugger, "Enables interactive VM debugger");

    return app;
}

} // namespace cli
} // namespace via
