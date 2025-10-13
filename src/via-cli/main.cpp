/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include <CLI/CLI.hpp>
#include <cstdint>
#include <sstream>
#include <via/via.hpp>

#undef assert

// Local assert function
static void assert(bool cond, std::string msg)
{
    if (!cond) {
        spdlog::error(msg);
        throw 1;
    }
}

// Expand $HOME on Unix, %USERPROFILE% on Windows.
static std::filesystem::path get_home_dir()
{
#ifdef _WIN32
    if (const char* profile = std::getenv("USERPROFILE")) {
        return std::filesystem::path(profile);
    }
    // Fallback: create from HOMEDRIVE + HOMEPATH
    const char* drive = std::getenv("HOMEDRIVE");
    const char* path = std::getenv("HOMEPATH");
    if (drive && path) {
        return std::filesystem::path(std::string(drive) + path);
    }
    return std::filesystem::current_path(); // last resort
#else
    if (const char* home = std::getenv("HOME")) {
        return std::filesystem::path(home);
    }
    return std::filesystem::current_path(); // last resort
#endif
}

// Gets the base directory where via stores core stuff
static std::filesystem::path get_lang_dir()
{
#ifdef _WIN32
    if (const char* local = std::getenv("LOCALAPPDATA")) {
        std::filesystem::path user_dir = std::filesystem::path(local) / "via";
        if (std::filesystem::exists(user_dir))
            return user_dir;
    }
    std::filesystem::path fallback = get_home_dir() / "AppData" / "Local" / "via";
    if (std::filesystem::exists(fallback))
        return fallback;
    return fallback;

#else
    if (const char* xdg = std::getenv("XDG_DATA_HOME")) {
        std::filesystem::path user_dir = std::filesystem::path(xdg) / "via";
        if (std::filesystem::exists(user_dir))
            return user_dir;
    }
    std::filesystem::path user_dir = get_home_dir() / ".local" / "share" / "via";
    if (std::filesystem::exists(user_dir))
        return user_dir;
    std::filesystem::path sys_dir = "/usr/local/share/via";
    if (std::filesystem::exists(sys_dir))
        return sys_dir;
    sys_dir = "/usr/share/via";
    return sys_dir;
#endif
}

int main(int argc, char* argv[])
{
    using via::Module;
    using via::ModuleFlags;
    using via::ModuleManager;
    using via::ModulePerms;
    using namespace via::literals; // For bitwise enum operators

    // Temporary spdlog config
    spdlog::set_pattern("%^%l:%$ %v");

    // Get language core directory
    static auto lang_dir = get_lang_dir();

    try {
        ModuleFlags flags = ModuleFlags::NONE;

        uint8_t verbosity = 0;
        bool no_execute = false, debugger = false;
        std::string dump;
        std::filesystem::path input;
        std::vector<std::string> imports;

        CLI::App app;

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

        app.add_option("input", input, "Input file path")
            ->required()
            ->check(CLI::ExistingFile)
            ->multi_option_policy(CLI::MultiOptionPolicy::Throw);

        app.add_option("--dump,-D", dump, "Compilation dump mode");

        app.add_option("--verbosity,-V", verbosity, "Controls verbosity")
            ->default_val(0)
            ->multi_option_policy(CLI::MultiOptionPolicy::Throw);

        app.add_option(
               "--import-dirs,-I",
               imports,
               "Comma seperated list of import directory paths"
        )
            ->delimiter(',')
            ->multi_option_policy(CLI::MultiOptionPolicy::Join);

        app.add_flag("--no-execute", no_execute, "Disables sequential execution");
        app.add_flag("--debugger", debugger, "Enables interactive VM debugger");

        CLI11_PARSE(app, argc, argv);

        assert(!input.empty(), "no input files");

        // Translate CLI flags to ModuleFlags
        do {
            if (no_execute)
                flags |= ModuleFlags::NO_EXECUTION;
            if (debugger)
                flags |= ModuleFlags::LAUNCH_DEBUGGER;
            if (dump == "token-tree") {
                flags |= ModuleFlags::DUMP_TTREE;
            } else if (dump == "ast") {
                flags |= ModuleFlags::DUMP_AST;
            } else if (dump == "ir") {
                flags |= ModuleFlags::DUMP_IR;
            } else if (dump == "executable") {
                flags |= ModuleFlags::DUMP_EXE;
            } else if (dump == "def-table") {
                flags |= ModuleFlags::DUMP_DEFTABLE;
            }
        }
        while (0);

        // Initialize via
        via::init(verbosity);

        if (verbosity > 0) {
            std::ostringstream oss;

            for (size_t i = 0; i < sizeof(flags) * 8; i++) {
                if (flags & static_cast<ModuleFlags>(1 >> i)) {
                    oss << i;
                    if (i != sizeof(flags) - 1) {
                        oss << ", ";
                    }
                }
            }

            spdlog::info("verbosity: {}", verbosity);

            auto normalize_string = [](std::string str) {
                return str.empty() ? "<default>" : str;
            };

            // clang-format off
            spdlog::info("[entry point] -- input path:    {}", normalize_string(input));
            spdlog::info("[entry point] -- dump mode:     {}", normalize_string(dump));
            spdlog::info("[entry point] -- import dirs:   {}", via::debug::to_string(imports, [](const auto& v) { return v; }));
            spdlog::info("[entry point] -- module flags:  {}", normalize_string(oss.str()));
            // clang-format on
        }

        ModuleManager manager;
        manager.push_import_path(input.parent_path());

        if (std::filesystem::exists(lang_dir)) {
            manager.push_import_path(lang_dir / "lib");
        } else {
            spdlog::warn(
                "Could not find language core directory (search location {})",
                lang_dir.string()
            );
        }

        for (const auto& path: imports) {
            manager.push_import_path(path);
        }

        // Instantiate root module
        auto module = Module::load_source_file(
            manager,
            nullptr,
            input.stem().c_str(),
            input,
            nullptr,
            ModulePerms::ALL,
            flags
        );

        // Validate root module
        assert(
            module.has_value(),
            module.error_or({} /* CRASHES if we dont provide default value */)
        );

        // Dump global symbol table
        if (dump == "symbol-table") {
            std::cout << via::ansi::format(
                             "[global symbol table]",
                             via::ansi::Foreground::YELLOW,
                             via::ansi::Background::NONE,
                             via::ansi::Style::UNDERLINE
                         )
                      << "\n";

            for (const auto& symbol: manager.symbol_table().get_symbols()) {
                std::cout << std::format("  {}: {}\n", symbol.second, symbol.first);
            }
        } else if (dump == "import-dirs") {
            for (const auto& path: manager.get_import_paths()) {
                std::println(std::cout, "  {}", path.string());
            }
        }
    } catch (int code) {
        return code;
    }
    return 0;
}
