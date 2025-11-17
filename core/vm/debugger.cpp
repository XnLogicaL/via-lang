/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "debugger.hpp"
#include <iomanip>
#include <print>
#include <spdlog/spdlog.h>
#include <sstream>
#include "support/ansi.hpp"
#include "value.hpp"
#include "vm/instruction.hpp"
#include "vm/machine.hpp"

static std::vector<std::string> tokenize_command(const std::string& line)
{
    std::istringstream iss(line);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word)
        words.push_back(word);
    return words;
}

static via::Argument parse_argument(const std::string& tok)
{
    if (tok == "true" || tok == "on")
        return true;
    if (tok == "false" || tok == "off")
        return false;

    if (tok.size() >= 2 && ((tok.front() == '"' && tok.back() == '"') ||
                            (tok.front() == '\'' && tok.back() == '\''))) {
        return tok.substr(1, tok.size() - 2); // Strip quotes
    }

    bool has_digit = false, has_dot = false, has_exp = false;
    for (size_t i = 0; i < tok.size(); ++i) {
        unsigned char c = tok[i];
        if (std::isdigit(c))
            has_digit = true;
        else if (c == '.' && !has_dot)
            has_dot = true;
        else if ((c == 'e' || c == 'E') && i > 0 && i + 1 < tok.size())
            has_exp = true;
    }

    if (has_digit && (has_dot || has_exp)) {
        char* end = nullptr;
        float val = std::strtof(tok.c_str(), &end);
        if (end != tok.c_str() && *end == '\0')
            return val;
    }

    if (tok.size() > 2 && tok[0] == '0' && (tok[1] == 'x' || tok[1] == 'X')) {
        try {
            return std::stoi(tok, nullptr, 16);
        } catch (...) {
            via::debug::bug();
        }
    }

    bool numeric =
        !tok.empty() && std::all_of(tok.begin(), tok.end(), [](unsigned char c) {
            return std::isdigit(c) || c == '-' || c == '+';
        });
    if (numeric && tok.find_first_of("0123456789") != std::string::npos) {
        try {
            return std::stoi(tok);
        } catch (...) {
            via::debug::bug();
        }
    }
    return tok;
}

static std::vector<via::Argument> parse_arguments(const std::vector<std::string>& tokens)
{
    std::vector<via::Argument> args;

    for (size_t i = 0; i < tokens.size(); i++) {
        if (i == 0) // Skip command name
            continue;
        auto token = tokens.at(i);
        auto arg = parse_argument(token);
        args.push_back(arg);
    }
    return args;
}

struct ActiveCommand
{
    std::string name;
    std::vector<via::Argument> args;
};

static ActiveCommand parse_command(const std::string& command)
{
    auto tokens = tokenize_command(command);
    auto args = parse_arguments(tokens);
    return {
        .name = tokens.at(0),
        .args = std::move(args),
    };
}

static bool validate_command(const via::Command& command, const ActiveCommand& active)
{
    // This should never happen in practice but we check for it anyways
    if (command.name != active.name)
        return false;
    if (command.args.size() != active.args.size()) {
        spdlog::error(
            "missing arguments (expected {}, got {})",
            command.args.size(),
            active.args.size()
        );
        return false;
    }

    for (size_t i = 0; const auto& type: command.args) {
        auto arg = active.args.at(i++);
        if (static_cast<size_t>(type) != arg.index()) {
            return false;
        }
    }
    return true;
}

void via::CommandTable::print_help() const
{
    size_t max_name_len = 0;
    for (const auto& [name, _]: commands) {
        max_name_len = std::max(max_name_len, name.size());
    }

    size_t max_args_len = 0;
    for (const auto& [_, cmd]: commands) {
        std::ostringstream oss;
        for (const auto& type: cmd.args)
            oss << " [" << to_string(type) << "]";
        max_args_len = std::max(max_args_len, oss.str().size());
    }

    spdlog::info("available commands:\n");

    for (const auto& [name, cmd]: commands) {
        std::ostringstream oss;
        for (const auto& type: cmd.args)
            oss << " [" << to_string(type) << "]";
        std::cout << "  " << std::left << std::setw(static_cast<int>(max_name_len))
                  << name << " " << std::left << std::setw(static_cast<int>(max_args_len))
                  << oss.str() << "     ->  " << cmd.help << "\n";
    }

    std::cout << ansi::format(
                     "\nPress [CTRL+C] to exit...\n",
                     ansi::Foreground::NONE,
                     ansi::Background::NONE,
                     ansi::Style::ITALIC
                 )
              << std::endl;
}

void via::Debugger::register_default_commands() noexcept
{
    m_cmds.register_command("help", "prints the help menu", {}, [this](const auto& args) {
        m_cmds.print_help();
    });

    m_cmds.register_command(
        "step",
        "steps the interpreter a given times",
        {ArgumentType::INTEGER},
        [this](const auto& args) {
            size_t n = std::get<size_t(ArgumentType::INTEGER)>(args.at(0));
            for (size_t i = 0; i < n; i++) {
                m_vm.execute_once();
            }
        }
    );

    m_cmds.register_command(
        "continue",
        "contniously steps the interpreter while dumping instruction data",
        {},
        [this](const auto& args) {
            while (true) {
                size_t counter = m_vm.m_pc - m_vm.m_bp;
                std::println(
                    "0x{:0>4x}  {}",
                    counter * 8,
                    m_vm.m_pc->to_string(true, counter)
                );

                if (m_vm.m_pc->op == OpCode::HALT)
                    break;
                m_vm.execute_once();
            }
        }
    );

    m_cmds.register_command(
        "pc",
        "display program counter information",
        {},
        [this](const auto& args) {
            Snapshot snapshot(&m_vm);

            std::cout << ansi::format(
                std::format("0x{:0>4x}  ", snapshot.rel_program_counter * 8),
                ansi::Foreground::NONE,
                ansi::Background::NONE,
                ansi::Style::FAINT
            );

            std::cout
                << snapshot.program_counter->to_string(true, snapshot.rel_program_counter)
                << "\n";
        }
    );

    m_cmds.register_command(
        "pcat",
        "display program counter information at the given address",
        {ArgumentType::INTEGER},
        [this](const auto& args) {
            size_t pc = std::get<size_t(ArgumentType::INTEGER)>(args.at(0));
            if (pc % 8 == 0) {
                auto realpc = pc / 8;
                auto bytecode = m_vm.m_exe->bytecode();
                if (realpc < bytecode.size()) {
                    auto* ptr = bytecode.data() + realpc;
                    std::cout << ptr->to_string(true, m_vm.m_pc - m_vm.m_bp) << "\n";
                } else {
                    spdlog::error("invalid pc 0x{:0>4x}: out of range", pc);
                }
            } else {
                spdlog::error("invalid pc 0x{:0>4x}: not a valid address", pc);
            }
        }
    );

    m_cmds.register_command(
        "reg",
        "dumps the given register",
        {ArgumentType::INTEGER},
        [this](const auto& args) {
            size_t reg = std::get<size_t(ArgumentType::INTEGER)>(args.at(0));
            if (auto* ptr = m_vm.m_registers[reg]) {
                std::cout << (void*) ptr << "\n";
                std::cout << ptr->to_string() << "\n";
            } else {
                std::cout << "unoccupied\n";
            }
        }
    );

    m_cmds.register_command(
        "regs",
        "dumps all occupied registers",
        {},
        [this](const auto& args) {
            bool debounce = true;
            size_t index = 0;
            Snapshot snapshot(&m_vm);

            for (const auto& reg: snapshot.registers) {
                if (reg != nullptr) {
                    std::println(
                        "R{} [{}]: {}",
                        index,
                        ansi::format(
                            std::format("@0x{:0>16x}", (uintptr_t) reg),
                            ansi::Foreground::NONE,
                            ansi::Background::NONE,
                            ansi::Style::FAINT
                        ),
                        reg->to_string()
                    );
                    debounce = true;
                } else if (debounce) {
                    std::println("...");
                    debounce = false;
                }
                ++index;
            }
        }
    );

    m_cmds.register_command(
        "const",
        "dumps the given constant",
        {ArgumentType::INTEGER},
        [this](const auto& args) {
            size_t index = std::get<size_t(ArgumentType::INTEGER)>(args.at(0));
            auto& consts = m_vm.m_exe->constants();
            if (index < consts.size()) {
                auto konst = consts.at(index);
                std::cout << konst.to_string() << "\n";
            } else {
                std::cout << "not found\n";
            }
        }
    );

    m_cmds.register_command(
        "jump",
        "jumps to the given program counter",
        {ArgumentType::INTEGER},
        [this](const auto& args) {
            size_t pc = std::get<size_t(ArgumentType::INTEGER)>(args.at(0));
            if (pc % 8 == 0) {
                auto realpc = pc / 8;
                auto bytecode = m_vm.m_exe->bytecode();
                if (realpc < bytecode.size()) {
                    m_vm.m_pc = m_vm.m_bp + realpc;
                } else {
                    spdlog::error("invalid pc 0x{:0>4x}: out of range", pc);
                }
            } else {
                spdlog::error("invalid pc 0x{:0>4x}: not a valid address", pc);
            }
        }
    );
}

void via::Debugger::start() noexcept
{
    static std::string cursor = ansi::format(
        ">> ",
        ansi::Foreground::GREEN,
        ansi::Background::NONE,
        ansi::Style::BOLD
    );

    m_cmds.print_help();
    m_vm.set_interrupt_hook([](VirtualMachine* vm, Interrupt inte, void* arg) {
        spdlog::warn("machine interrupted");
        std::cout << " code: 0x" << std::hex << size_t(inte) << std::dec;
        std::cout << " " << std::format("({})\n", via::to_string(inte));

        switch (inte) {
        case Interrupt::ERROR: {
            auto* error = reinterpret_cast<ErrorInt*>(arg);
            std::cout << " error info:\n";
            std::cout << "  msg:  " << error->msg << "\n";
            std::cout << "  out:  " << (void*) error->out << "\n";
            std::cout << "  fp:   " << (void*) error->fp << "\n";
            std::cout << "  pc:   " << (void*) error->pc << "\n";
        }
        default:
            break;
        }
    });

    replxx::Replxx repl;

    while (auto* cinput = repl.input(cursor)) {
        std::string input(cinput);

        auto active = parse_command(input);
        if (auto command = m_cmds.find_command(active.name)) {
            if (validate_command(*command, active))
                command->handler(active.args);
            continue;
        }

        spdlog::error("command not found: '{}'", active.name);
    }
}
