/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "init.h"
#include <cpptrace/cpptrace.hpp>
#include <iostream>
#include <spdlog/sinks/ansicolor_sink.h>
#include <spdlog/spdlog.h>
#include "debug.h"
#include "mimalloc.h"

static void init_spdlog() noexcept
{
    std::shared_ptr<spdlog::sinks::ansicolor_stdout_sink_mt> console =
        std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();

    std::string info_color = console->cyan.data();
    info_color += console->bold.data();
    console->set_color(spdlog::level::info, std::string_view(info_color.c_str()));

    spdlog::set_default_logger(std::make_shared<spdlog::logger>("main", console));
    spdlog::set_pattern("%^%l:%$ %v");
}

static void mimalloc_error_handler(int err, void* arg)
{
    spdlog::error("mimalloc: error code {}", err);
    cpptrace::generate_trace().print(std::cerr);
}

static void init_mimalloc() noexcept
{
    mi_register_error(mimalloc_error_handler, nullptr);
    mi_option_set(mi_option_show_errors, via::config::DEBUG_ENABLED);
}

static void call_once_trap() noexcept
{
    static bool called = false;
    via::debug::require(!called, "init() called twice");
    called = true;
}

void via::init() noexcept
{
    call_once_trap();
    init_mimalloc();
    init_spdlog();
}
