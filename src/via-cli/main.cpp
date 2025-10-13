/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include <string>
#include <via/via.hpp>
#include "app.hpp"
#include "module/module.hpp"
#include "tooling.hpp"

static void
translate_module_flags(via::ModuleFlags& flags, via::cli::ProgramOptions& options)
{
    if (options.no_execute)
        flags |= via::ModuleFlags::NO_EXECUTION;
    if (options.debugger)
        flags |= via::ModuleFlags::LAUNCH_DEBUGGER;
    if (options.dump.contains("token-tree"))
        flags |= via::ModuleFlags::DUMP_TTREE;
    if (options.dump.contains("ast"))
        flags |= via::ModuleFlags::DUMP_AST;
    if (options.dump.contains("ir"))
        flags |= via::ModuleFlags::DUMP_IR;
    if (options.dump.contains("executable"))
        flags |= via::ModuleFlags::DUMP_EXE;
    if (options.dump.contains("def-table"))
        flags |= via::ModuleFlags::DUMP_DEFTABLE;
}

int main(int argc, char* argv[])
{
    using via::Module;
    using via::ModuleFlags;
    using via::ModuleManager;
    using via::ModulePerms;
    using namespace via::cli;
    using namespace via::operators;

    // Temporary spdlog config
    spdlog::set_pattern("%^%l:%$ %v");

    ModuleFlags flags = ModuleFlags::NONE;
    ProgramOptions options;

    if (int code = initialize_app(argc, argv, options); code != 0) {
        return code;
    }

    translate_module_flags(flags, options);

    via::init(options.verbosity);

    if (options.verbosity > 0) {
        std::println(std::cout, "{}", options.to_string());
    }

    ModuleManager manager;
    manager.push_import_path(options.input.parent_path());

    static auto lang_dir = get_lang_dir();

    if (std::filesystem::exists(lang_dir)) {
        manager.push_import_path(lang_dir / "lib");
    } else {
        spdlog::warn(
            "Could not find language core directory (search location {})",
            lang_dir.string()
        );
    }

    for (const auto& path: options.imports) {
        manager.push_import_path(path);
    }

    // Instantiate root module
    auto module = Module::load_source_file(
        manager,
        nullptr,
        options.input.stem().c_str(),
        options.input,
        nullptr,
        ModulePerms::ALL,
        flags
    );

    if (!module.has_value()) {
        spdlog::error(module.error());
        return 1;
    }

    if (options.dump.contains("symbol-table")) {
        std::println(std::cout, "{}", manager.symbol_table().to_string());
    } else if (options.dump.contains("import-dirs")) {
        for (const auto& path: manager.get_import_paths()) {
            std::println(std::cout, "  {}", path.string());
        }
    }
    return 0;
}
