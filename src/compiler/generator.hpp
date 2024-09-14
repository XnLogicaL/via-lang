#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <variant>
#include <optional>
#include <iostream>
#include <functional>
#include <map>
#include <memory>
#include <algorithm>

#include "parser.hpp"
#include "utils.hpp"
#include "lexer.hpp"
#include "../include/color.hpp"

const std::string SPACE = " ";
const std::string LNBR = "\n";

class Generator
{
public:
    ProgNode prog;
    std::string write_file;

    std::string _section_text;
    std::string _section_data;
    std::string _func_main;
    std::string _func_start;

    int temp_label_counter = 0; // For unique temporary labels

    Generator(ProgNode prog, std::string write_file = "out.asm")
        : prog(prog), write_file(write_file) {};

    void create_output_file(const std::string& out_src)
    {
        std::ofstream out_file(write_file);
        if (out_file.is_open())
        {
            out_file << out_src;
            out_file.close();
        }
        else
            std::cerr << "Error opening file: " << write_file << std::endl;
    }

    void _generate_template()
    {
        _section_text = "section .text\nglobal _start\n";
        _section_data = "section .data\n";

        _func_main = "_main:\n";
        _func_start = "_start:\n    call _main\n";
    }

    std::string _build_source()
    {
        return _section_text + LNBR + get_std() + LNBR + _func_main + "    ret\n" + LNBR + _func_start + LNBR + _section_data;
    }

    std::string get_std()
    {
        std::string _std_src;
        std::string _std_src_dump;
        std::ifstream std_file("../std/std.asm");

        if (!std_file.is_open())
        {
            std::cerr << "Error opening file: ../std/std.asm" << std::endl;
            return "";
        }

        while (std::getline(std_file, _std_src_dump))
            _std_src += _std_src_dump + LNBR;

        std_file.close();

        return _std_src;
    }

    std::string generate_declaration(LocalDeclrNode* declaration)
    {
        return std::visit([this, &declaration](const auto& obj)
            {
                using T = std::decay_t<decltype(obj)>;

                std::string asm_code;

                if constexpr (std::is_same_v<T, TermNode*>)
                {
                    std::visit([this, &declaration, &asm_code](const auto& term) {
                        using T_term = std::decay_t<decltype(term)>;

                        std::string env_string = "local";
                        std::string const_string = declaration->is_const ? "const " : "";

                        if constexpr (std::is_same_v<T_term, IntLitNode*>)
                        {
                            asm_code += "    " + declaration->ident.value + " dd " + term->int_lit.value;
                            asm_code += " ; " + env_string + " " + const_string + declaration->ident.value + " = " + term->int_lit.value + "\n\n";
                        }
                        }, obj->node);
                }

                /*std::string str_value = value_string.value();
                str_value.erase(std::remove(str_value.begin(), str_value.end(), '\"'), str_value.end()); // Remove quotes

                asm_code += "    " + declaration->id + " db \"" + str_value + "\", 0xA";
                asm_code += " ; " + env_string + " " + const_string + declaration->id + " = " + value_string.value() + "\n";
                asm_code += "    " + declaration->id + "_len equ $ - " + declaration->id + "\n\n"; }*/

                return asm_code; }, declaration->expr->node);
    }

    std::optional<std::string> interpolate_string(std::string raw_string)
    {
        // TODO
        return raw_string;
    }

    std::optional<std::string> generate_func_call(FuncCallNode* call)
    {
        static const std::map<std::string, std::function<std::string(FuncCallNode*)>> func_dispatch{
            {"exit", [this](FuncCallNode* call)
             {
                 if (call->args.size() < 1)
                 {
                     Console::CompilerError("Expected return code (int arg0) for exit()");
                     return std::string();
                 }

                 auto arg0 = call->args.at(0);

                 return "    mov rdi, " + arg0->ident.value 
                    + "\n    call __via_exit\n";
             }},
            {"print", [this](FuncCallNode* call)
             {
                 if (call->args.size() < 1)
                 {
                     Console::CompilerError("Expected print message (string arg0) for print()");
                     return std::string();
                 }

                 auto arg0 = call->args.at(0);
                 std::string var;
                 std::string var_id;

                 if (arg0->ident.type != TokenType::IDENTIFIER)
                 {
                     var_id = "__temp_" + std::to_string(temp_label_counter);
                     _section_data += ("\n    " + var_id + " db '" + arg0->ident.value) + "', 0xA ; " + call->ident.value + "('" + arg0->ident.value + "')";
                     temp_label_counter++;
                 }
                 else
                     var_id = arg0->ident.value;

                 return "    mov rsi, " + var_id 
                    + "\n    mov rdx, " + std::to_string(arg0->ident.value.length() + 1) 
                    + "\n    call __via_std_out\n";
             }},
            {"error", [this](FuncCallNode* call)
             {
                 std::string error_prefix("error: ");

                 auto arg0 = call->args.at(0);

                 auto line = call->ident.line;
                 auto column = call->ident.column;
                 auto error_msg = arg0->ident.value;

                 arg0->ident.value = prog.prog_name + ":" + std::to_string(line) + ": " + dye::red(error_prefix) + error_msg;

                 IdentNode exit_code = {TokenType::INT_LIT, "1", line, column};
                 IdentNode* exit_code_ptr = &exit_code;

                 std::vector<IdentNode*> args = {exit_code_ptr};

                 FuncCallNode exit_call = {{TokenType::IDENTIFIER, "__indirect_exit", line, column}, args};
                 FuncCallNode* exit_call_ptr = &exit_call;

                 return func_dispatch.find("print")->second(call) +
                        func_dispatch.find("exit")->second(exit_call_ptr);
             }},
             {"warn", [this](FuncCallNode* call)
             {
                 std::string warn_prefix("warning: ");

                 auto arg0 = call->args.at(0);

                 auto line = call->ident.line;
                 auto column = call->ident.column;
                 auto warn_msg = arg0->ident.value;

                 arg0->ident.value = prog.prog_name + ":" + std::to_string(line) + ": " + dye::yellow(warn_prefix) + warn_msg;

                 return func_dispatch.find("print")->second(call);
             }}
        };

        auto it = func_dispatch.find(call->ident.value);

        if (it != func_dispatch.end())
            return it->second(call);

        Console::CompilerWarning("Unsupported function call: " + call->ident.value);

        return std::nullopt;
    }

    void generate()
    {
        _generate_template();

        for (auto& content : prog.prog_scope)
        {
            std::visit([this](const auto& obj)
                {
                    using T = std::decay_t<decltype(obj)>;

                    if constexpr (std::is_same_v<T, LocalDeclrNode*>)
                        _section_data += generate_declaration(obj);
                    else if constexpr (std::is_same_v<T, FuncCallNode*>)
                        _func_main += generate_func_call(obj).value();
                    else assert(false); }, content->stmt);
        }

        create_output_file(_build_source());
    }
};
