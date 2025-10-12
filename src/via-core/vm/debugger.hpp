/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <replxx.hxx>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <variant>
#include <via/config.hpp>
#include "machine.hpp"
#include "support/utility.hpp"

namespace via {

#define FOR_EACH_ARG_TYPE(X) X(INTEGER) X(FLOAT) X(BOOLEAN) X(STRING)

enum class ArgumentType : uint8_t
{
    FOR_EACH_ARG_TYPE(DEFINE_ENUM)
};

DEFINE_TO_STRING(ArgumentType, FOR_EACH_ARG_TYPE(DEFINE_CASE_TO_STRING));

using Argument = std::variant<int, float, bool, std::string>;

struct Command
{
    using Handler = std::function<void(const std::vector<Argument>& args)>;
    std::string name;
    std::string help;
    std::vector<ArgumentType> args;
    Handler handler;
};

class CommandTable
{
  public:
    void
    add(std::string name,
        std::string help,
        std::vector<ArgumentType> args,
        Command::Handler handler)
    {
        commands[name] = Command{
            .name = name,
            .help = help,
            .args = args,
            .handler = handler,
        };
    }

    const Command* find(const std::string& name) const
    {
        auto it = commands.find(name);
        if (it != commands.end()) {
            return &it->second;
        }
        return nullptr;
    }

    void print_help() const;

  private:
    std::unordered_map<std::string, Command> commands;
};

class Debugger final
{
  public:
    Debugger(VirtualMachine& vm)
        : m_vm(vm)
    {}

  public:
    auto& command_table() { return m_cmds; }

    void register_default_commands() noexcept;
    void start() noexcept;

  private:
    VirtualMachine& m_vm;
    CommandTable m_cmds;
};

} // namespace via
