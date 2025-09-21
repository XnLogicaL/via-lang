/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include <csv2/reader.hpp>
#include <replxx.hxx>
#include <spdlog/sinks/ansicolor_sink.h>
#include <spdlog/spdlog.h>
#include <via/via.h>
#include "app.h"

#undef assert

namespace fs = std::filesystem;

static void assert(bool cond, std::string msg)
{
    if (!cond) {
        spdlog::error(msg);
        throw 1;
    }
}

// Expand $HOME on Unix, %USERPROFILE% on Windows.
static fs::path get_home_dir()
{
#ifdef _WIN32
    if (const char* profile = std::getenv("USERPROFILE")) {
        return fs::path(profile);
    }
    // Fallback: construct from HOMEDRIVE + HOMEPATH
    const char* drive = std::getenv("HOMEDRIVE");
    const char* path = std::getenv("HOMEPATH");
    if (drive && path) {
        return fs::path(std::string(drive) + path);
    }
    return fs::current_path(); // last resort
#else
    if (const char* home = std::getenv("HOME")) {
        return fs::path(home);
    }
    return fs::current_path(); // last resort
#endif
}

// Gets the base directory where via stores core stuff
static fs::path get_lang_dir()
{
#ifdef _WIN32
    if (const char* local = std::getenv("LOCALAPPDATA")) {
        return fs::path(local) / "via";
    }
    return get_home_dir() / "AppData" / "Local" / "via";
#else
    if (const char* xdg = std::getenv("XDG_DATA_HOME")) {
        return fs::path(xdg) / "via";
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
    using namespace via::literals;

    std::shared_ptr<spdlog::sinks::ansicolor_stdout_sink_mt> console =
        std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();

    std::string info_color = console->cyan.data();
    info_color += console->bold.data();
    console->set_color(spdlog::level::info, std::string_view(info_color.c_str()));

    spdlog::set_default_logger(std::make_shared<spdlog::logger>("main", console));
    spdlog::set_pattern("%^%l:%$ %v");

    static auto lang_dir = get_lang_dir();

    try {
        auto& cli = via::cli::app_instance();

        csv2::Reader<
            csv2::delimiter<','>,
            csv2::quote_character<'\''>,
            csv2::first_row_is_header<false>,
            csv2::trim_policy::trim_whitespace>
            csv;

        std::string raw_dump_mode, raw_include_dirs;
        fs::path input_path;

        try {
            cli.parse_args(argc, argv);
            input_path = cli.get("input");
            raw_dump_mode = cli.get("--dump");
            raw_include_dirs = cli.get("--include-dirs");
        }
        catch (const std::bad_any_cast&) {
            assert(false, "bad argument");
        }
        catch (const std::exception& err) {
            assert(false, err.what());
        }

        assert(!input_path.empty(), "no input files");

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

        if (cli.get<bool>("--no-execute")) {
            flags |= ModuleFlags::NO_EXECUTION;
        }

        if (cli.get<bool>("--debug")) {
            flags |= ModuleFlags::DEBUG;
        }

        if (!fs::exists(lang_dir)) {
            spdlog::warn(
                "Could not find language core directory (search location {})",
                lang_dir.string()
            );
        }

        ModuleManager manager;
        manager.push_import_path(input_path.parent_path());

        if (!raw_include_dirs.empty()) {
            assert(csv.parse(raw_include_dirs), "bad import path list");

            for (const auto row: csv) {
                for (const auto cell: row) {
                    std::string path;
                    cell.read_value(path);
                    manager.push_import_path(path);
                }
            }
        }

        auto module = Module::load_source_file(
            &manager,
            nullptr,
            input_path.stem().c_str(),
            input_path,
            nullptr,
            ModulePerms::ALL,
            flags
        );

        assert(
            module.has_value(),
            module.error_or(via::Error::fail("<no-error>")).to_string()
        );

        if (raw_dump_mode == "symtab") {
            std::println(
                std::cout,
                "{}",
                via::ansi::format(
                    "[global symbol table]",
                    via::ansi::Foreground::Yellow,
                    via::ansi::Background::Black,
                    via::ansi::Style::Bold
                )
            );

            for (const auto& symbol: manager.get_symbol_table().get_symbols()) {
                std::println(std::cout, "  {}: {}", symbol.second, symbol.first);
            }
        }
    }
    catch (int code) {
        return code;
    }

    return 0;
}
