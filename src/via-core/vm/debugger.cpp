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
#include "value.hpp"

static std::vector<std::string> tokenize_command(const std::string& line)
{
    std::istringstream iss(line);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word) {
        words.push_back(word);
    }
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

    bool has_digit = false;
    bool has_dot = false;
    bool has_exp = false;

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
        std::ostringstream args_ss;
        for (const auto& type: cmd.args)
            args_ss << " [" << to_string(type) << "]";

        std::cout << "  " << std::left << std::setw(static_cast<int>(max_name_len))
                  << name << " " << std::left << std::setw(static_cast<int>(max_args_len))
                  << args_ss.str() << " — " << cmd.help << "\n";
    }
    std::cout << "\nPress CTRL + C to exit...\n" << std::endl;
}

void via::Debugger::register_default_commands() noexcept
{
    m_cmds.add("help", "prints the help menu", {}, [this](const auto& args) {
        std::println(std::cout);
        m_cmds.print_help();
        std::println(std::cout);
    });

    m_cmds.add(
        "step",
        "steps the interpreter n times",
        {ArgumentType::INTEGER},
        [this](const auto& args) {
            size_t n = std::get<(size_t) ArgumentType::INTEGER>(args.at(0));
            for (size_t i = 0; i < n; i++) {
                m_vm.execute_once();
            }
        }
    );

    m_cmds.add("pc", "display program counter information", {}, [this](const auto& args) {
        Snapshot snapshot(&m_vm);

        std::println(std::cout);
        spdlog::info("program counter:");
        std::cout << "- raw:           " << (void*) snapshot.program_counter << "\n";
        std::cout << "- relative:      0x" << std::hex << std::setw(4)
                  << std::setfill('0') << (size_t) (snapshot.rel_program_counter * 8)
                  << std::dec << " (base10: " << snapshot.rel_program_counter << ")\n";
        std::cout << "- disassembly:   [" << snapshot.program_counter->to_string()
                  << "]\n";
        std::println(std::cout);
    });

    m_cmds.add(
        "reg",
        "dumps the given register",
        {ArgumentType::INTEGER},
        [this](const auto& args) {
            std::println(std::cout);

            size_t reg = std::get<(size_t) ArgumentType::INTEGER>(args.at(0));
            if (auto* ptr = m_vm.m_registers[reg]) {
                spdlog::info("register {}:", reg);
                std::cout << "- raw:          " << (void*) ptr << "\n";
                std::cout << "- disassembly:  " << ptr->to_string() << "\n";
            } else {
                spdlog::info("register {} unoccupied", reg);
            }

            std::println(std::cout);
        }
    );

    m_cmds.add(
        "const",
        "dumps the given constant",
        {ArgumentType::INTEGER},
        [this](const auto& args) {
            std::println(std::cout);

            size_t idx = std::get<(size_t) ArgumentType::INTEGER>(args.at(0));
            auto& consts = m_vm.m_exe->constants();
            if (idx < consts.size()) {
                auto konst = consts.at(idx);
                spdlog::info("constant {}:", idx);
                std::cout << "- dissassembly:  " << konst.get_dump() << "\n";
            } else {
                spdlog::info("constant {} not found", idx);
            }

            std::println(std::cout);
        }
    );
}

void via::Debugger::start() noexcept
{
    replxx::Replxx repl;

    m_cmds.print_help();
    m_vm.set_int_hook([](VirtualMachine* vm, Interrupt in, void* arg) {
        spdlog::warn("machine interrupted");
        std::cout << " code: 0x" << std::hex << size_t(in) << std::dec;
        std::cout << " " << std::format("({})\n", via::to_string(in));

        if (in == Interrupt::ERROR) {
            auto* error = reinterpret_cast<ErrorInt*>(arg);
            std::cout << " error info:\n";
            std::cout << "  msg:  " << error->msg << "\n";
            std::cout << "  out:  " << (void*) error->out << "\n";
            std::cout << "  fp:   " << (void*) error->fp << "\n";
            std::cout << "  pc:   " << (void*) error->pc << "\n";
        }
    });

    while (auto* cinput = repl.input("=> ")) {
        std::string input(cinput);

        auto active = parse_command(input);
        if (auto command = m_cmds.find(active.name)) {
            if (validate_command(*command, active))
                command->handler(active.args);
            continue;
        }

        spdlog::error("command not found: '{}'", active.name);
    }
}
