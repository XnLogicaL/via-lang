/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <format>
#include <iostream>
#include <ostream>

namespace via {

enum class LogLevel
{
    NONE,
    INFO,
    WARN,
    ERROR,
};

class Logger
{
  public:
    Logger() = delete;
    Logger(std::ostream& file)
        : m_file(file)
    {}

    static Logger& stdout_logger()
    {
        static Logger logger(std::cout);
        return logger;
    }

    static Logger& stderr_logger()
    {
        static Logger logger(std::cerr);
        return logger;
    }

  public:
    // clang-format off
    template<typename... Args>
    void log(LogLevel level, std::format_string<Args...> fmt, Args&&... args)
        { log(level, std::format(fmt, std::forward<Args>(args)...)); }

    template <typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args)
        { log(LogLevel::INFO, std::format(fmt, std::forward<Args>(args)...)); }

    template <typename... Args>
    void warn(std::format_string<Args...> fmt, Args&&... args)
        { log(LogLevel::WARN, std::format(fmt, std::forward<Args>(args)...)); }

    template <typename... Args>
    void error(std::format_string<Args...> fmt, Args&&... args)
        { log(LogLevel::ERROR, std::format(fmt, std::forward<Args>(args)...)); }
    // clang-format on

  private:
    void log(LogLevel level, std::string string);

  private:
    std::ostream& m_file;
};

} // namespace via
