/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include <cstdint>
#include <csv2/reader.hpp>
#include <replxx.hxx>
#include <sstream>
#include <via/via.h>
#include "app.h"

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
        return std::filesystem::path(local) / "via";
    }
    return get_home_dir() / "AppData" / "Local" / "via";
#else
    if (const char* xdg = std::getenv("XDG_DATA_HOME")) {
        return std::filesystem::path(xdg) / "via";
    }
    return get_home_dir() / ".local" / "share" / "via";
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
        // Instantiate CLI application
        auto& cli = via::cli::app_instance();

        // Instantiate Comma Separated Value reader
        csv2::Reader<
            csv2::delimiter<','>, // Obviously
            csv2::quote_character<'\''>,
            csv2::first_row_is_header<false>,
            csv2::trim_policy::trim_whitespace>
            csv;

        uint8_t verbosity;
        std::string raw_dump_mode, raw_import_dirs;
        std::filesystem::path input_path;

        try {
            cli.parse_args(argc, argv);
            verbosity = cli.get<uint8_t>("--verbose");
            input_path = cli.get<std::string>("input");
            raw_dump_mode = cli.get<std::string>("--dump");
            raw_import_dirs = cli.get<std::string>("--include-dirs");
        } catch (const std::bad_any_cast&) {
            assert(false, "bad argument");
        } catch (const std::exception& err) {
            assert(false, err.what());
        }

        assert(!input_path.empty(), "no input files");

        // Translate CLI flags to ModuleFlags
        ModuleFlags flags = ModuleFlags::NONE;
        {
            if (raw_dump_mode == "ttree")
                flags |= ModuleFlags::DUMP_TTREE;
            else if (raw_dump_mode == "ast")
                flags |= ModuleFlags::DUMP_AST;
            else if (raw_dump_mode == "ir")
                flags |= ModuleFlags::DUMP_IR;
            else if (raw_dump_mode == "exe")
                flags |= ModuleFlags::DUMP_EXE;
            else if (raw_dump_mode == "deftab")
                flags |= ModuleFlags::DUMP_DEFTABLE;
        }

        // Translate --no-execute flag
        if (cli.get<bool>("--no-execute")) {
            flags |= ModuleFlags::NO_EXECUTION;
        }

        // Look for --debug flag
        if (cli.get<bool>("--debug")) {
            flags |= ModuleFlags::DEBUG;
        }

        // Validate language core directory
        if (!std::filesystem::exists(lang_dir)) {
            spdlog::warn(
                "Could not find language core directory (search location {})",
                lang_dir.string()
            );
        }

        // Initialize via
        via::init(verbosity);

        if (verbosity > 0) {
            std::ostringstream oss;

            for (size_t i = 0; i < sizeof(flags); i++) {
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
            spdlog::info("[entry point] -- input path:    {}", normalize_string(input_path.string()));
            spdlog::info("[entry point] -- dump mode:     {}", normalize_string(raw_dump_mode));
            spdlog::info("[entry point] -- import dirs:   {}", normalize_string(raw_import_dirs));
            spdlog::info("[entry point] -- module flags:  {}", normalize_string(oss.str()));
            // clang-format on
        }

        // Instantiate ModuleManager
        ModuleManager manager;
        // Push adjacent import path
        manager.push_import_path(input_path.parent_path());

        // Process import paths
        if (!raw_import_dirs.empty()) {
            // Validate import paths
            assert(csv.parse(raw_import_dirs), "bad import path list");

            // Load import paths
            for (const auto row: csv) {
                for (const auto cell: row) {
                    std::string path;
                    cell.read_value(path);
                    manager.push_import_path(path);
                }
            }
        }

        // Instantiate root module
        auto module = Module::load_source_file(
            manager,
            nullptr,
            input_path.stem().c_str(),
            input_path,
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
        if (raw_dump_mode == "symtab") {
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
        }
    } catch (int code) {
        return code;
    }
    return 0;
}
