/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <map>
#include <variant>
#include <via/config.hpp>
#include "logger.hpp"
#include "machine.hpp"
#include "support/utility.hpp"

namespace via {

#define FOR_EACH_ARG_TYPE(X) X(INTEGER) X(FLOAT) X(BOOLEAN) X(STRING)

enum class ArgumentType : uint8_t
{
    FOR_EACH_ARG_TYPE(DEFINE_ENUM)
};

DEFINE_TO_STRING(ArgumentType, FOR_EACH_ARG_TYPE(DEFINE_CASE_TO_STRING));

using CommandArgument = std::variant<int, float, bool, std::string>;
using CommandHandler = std::function<bool(const std::vector<CommandArgument>& args)>;

struct Command
{
    std::string name;
    std::string help;
    std::vector<ArgumentType> args;
    CommandHandler handler;
    Logger* logger;
};

class CommandTable
{
  public:
    CommandTable(Logger& logger)
        : m_logger(logger)
    {}

  public:
    void register_command(
        std::string name,
        std::string help,
        std::vector<ArgumentType> args,
        CommandHandler handler
    )
    {
        m_commands[name] = Command{
            .name = name,
            .help = help,
            .args = args,
            .handler = handler,
            .logger = &m_logger,
        };
    }

    const Command* find_command(const std::string& name) const
    {
        auto it = m_commands.find(name);
        if (it != m_commands.end())
            return &it->second;
        return nullptr;
    }

    void print_help() const;

  private:
    Logger& m_logger;
    std::map<std::string, Command> m_commands;
};

class Debugger final
{
  public:
    explicit Debugger(VirtualMachine& vm)
        : m_vm(vm),
          m_cmds(m_logger)
    {}

  public:
    auto& command_table() { return m_cmds; }

    void register_default_commands() noexcept;
    void start() noexcept;

  private:
    Logger& m_logger = Logger::stdout_logger(); // TODO: Modularize
    VirtualMachine& m_vm;
    CommandTable m_cmds;
};

} // namespace via
