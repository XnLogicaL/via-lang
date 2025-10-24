/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "init.hpp"
#include <cpptrace/cpptrace.hpp>
#include <fmt/base.h>
#include <iostream>
#include <mimalloc.h>
#include <spdlog/common.h>
#include <spdlog/sinks/ansicolor_sink.h>
#include <spdlog/spdlog.h>
#include "debug.hpp"
#include "support/ansi.hpp"

class LabelFormatter: public spdlog::formatter
{
  public:
    void format(const spdlog::details::log_msg& msg, spdlog::memory_buf_t& dest) override
    {
        std::string label;

        switch (msg.level) {
        case spdlog::level::info:
            label = via::ansi::format(
                "info:",
                via::ansi::Foreground::GREEN,
                via::ansi::Background::NONE,
                via::ansi::Style::BOLD
            );
            break;
        case spdlog::level::warn:
            label = via::ansi::format(
                "warning:",
                via::ansi::Foreground::YELLOW,
                via::ansi::Background::NONE,
                via::ansi::Style::BOLD
            );
            break;
        case spdlog::level::err:
            label = via::ansi::format(
                "error:",
                via::ansi::Foreground::RED,
                via::ansi::Background::NONE,
                via::ansi::Style::BOLD
            );
            break;
        case spdlog::level::debug:
            label = via::ansi::format(
                "debug:",
                via::ansi::Foreground::CYAN,
                via::ansi::Background::NONE,
                via::ansi::Style::BOLD
            );
            break;
        default:
            break;
        }

        if (label.empty()) {
            fmt::format_to(std::back_inserter(dest), "{}\n", msg.payload);
        } else {
            fmt::format_to(std::back_inserter(dest), "{} {}\n", label, msg.payload);
        }
    }

    std::unique_ptr<spdlog::formatter> clone() const override
    {
        return std::make_unique<LabelFormatter>();
    }
};

static void init_spdlog() noexcept
{
    auto sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("main", sink);
    logger->set_pattern("%^%l%$ :: %v");
    logger->set_formatter(std::make_unique<LabelFormatter>());

    spdlog::set_default_logger(std::move(logger));
}

static void mimalloc_error_handler(int err, void* arg)
{
    spdlog::error("mimalloc: error code {}", err);
    cpptrace::generate_trace().print(std::cerr);
}

static void init_mimalloc(uint8_t verbosity) noexcept
{
    mi_option_set(mi_option_reserve_os_memory, via::config::PREALLOC_SIZE);

    mi_option_set(mi_option_large_os_pages, 0);
    mi_option_set(mi_option_reserve_huge_os_pages, 0);
    mi_option_set(mi_option_reserve_huge_os_pages_at, -1); // any NUMA node

    mi_option_set(mi_option_eager_commit, 0);
    mi_option_set(mi_option_eager_commit_delay, 4); // commit lazily in 4-page steps

    mi_option_set(mi_option_reset_delay, 0); // Disable page reset delay
    mi_option_set(mi_option_show_errors, via::config::DEBUG_ENABLED);
    mi_option_set(mi_option_show_stats, verbosity > 1);
    mi_option_set(mi_option_verbose, verbosity > 2);

    mi_register_error(mimalloc_error_handler, nullptr);

    if (verbosity > 1) {
        mi_stats_print(nullptr);
    }
}

static void trap_call() noexcept
{
    static bool called = false;
    via::debug::require(!called, "init() called twice");
    called = true;
}

void via::init(uint8_t verbosity) noexcept
{
    trap_call();
    init_mimalloc(verbosity);
    init_spdlog();
}
