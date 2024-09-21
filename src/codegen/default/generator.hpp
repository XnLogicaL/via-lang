#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <variant>
#include <optional>
#include <iostream>
#include <functional>
#include <map>
#include <algorithm>
#include <type_traits>

#include "parser.hpp"
#include "lexer.hpp"

// Constants
const std::string SPACE = " ";
const std::string LNBR = "\n";

// Generator class definition
class Generator
{
public:
    ProgNode prog;
    std::string write_file;

    // Constructor
    Generator(ProgNode prog, std::string write_file = "out.asm")
        : prog(prog), write_file(write_file) {}

    // Main generation function
    void generate()
    {
        generate_template();

        // Process each content node in the program scope
        for (auto& content : prog.stmts)
        {
            std::visit([this](const auto& obj) {
                using T = std::decay_t<decltype(obj)>;

                if constexpr (std::is_same_v<T, LocalDeclNode*>)
                    section_data += generate_declaration(obj);
                else if constexpr (std::is_same_v<T, FuncCallNode*>)
                    func_main += generate_func_call(obj).value_or("");
                else if constexpr (std::is_same_v<T, IfStmtNode*>)
                    func_main += generate_if_stmt(obj); // Handle the if statement
                else if constexpr (std::is_same_v<T, ScopeNode*>)
                    func_main += generate_scope(obj).value_or("");
                }, content->stmt);
        }

        create_output_file(build_source());
    }

private:
    // Assembly code sections
    std::string section_text;
    std::string section_data;
    std::string func_main;
    std::string func_start;

    int temp_label_counter = 0; // For unique temporary labels

    std::string generate_function_prologue()
    {
        return "    push rbx\n"   // Save rbx
            "    push rbp\n"   // Save base pointer
            "    mov rbp, rsp\n" // Set up base pointer
            "    sub rsp, 16\n"; // Allocate space for local variables
    }

    std::string generate_function_epilogue()
    {
        return "    add rsp, 16\n"
            "    mov rsp, rbp\n" // Restore stack pointer
            "    pop rbp\n"      // Restore base pointer
            "    pop rbx\n"      // Restore rbx
            "    ret\n";
    }

    std::string parse_term(ExprNode* term, LocalDeclNode* declaration)
    {
        if (!term || !declaration)
            return std::string();

        return std::visit([&](const auto& node) {
            using T = std::decay_t<decltype(node)>;

            if constexpr (std::is_same_v<T, IntLitNode*>)
                return "    " + declaration->ident.value + " db " + node->val.value + "\n";
            else if constexpr (std::is_same_v<T, BoolLitNode*>)
                return "    " + declaration->ident.value + " db " + (node->val.value == "true" ? "1" : "0") + "\n";
            else if constexpr (std::is_same_v<T, StringLitNode*>)
                return "    " + declaration->ident.value + " db '" + node->val.value + "', 0xA\n";
            else if constexpr (std::is_same_v<T, FuncNode*>)
                return generate_function(node).value_or("");
            else if constexpr (std::is_same_v<T, ParenExprNode*>)
                return __parse_term(node->expr, declaration);
            else
                std::cout << typeid(node).name() << std::endl;

            return std::string();
            }, term->node);
    }

    std::string __parse_term(ExprNode* expr, LocalDeclNode* declaration)
    {
        return std::visit([&](const auto& term) {
            using T = std::decay_t<decltype(term)>;
            return parse_term(expr, declaration);
            }, expr->node);
    }

    std::string generate_declaration(LocalDeclNode* declaration)
    {
        std::string asm_code;

        std::visit([this, &declaration, &asm_code](const auto& obj) {
            asm_code += __parse_term(declaration->expr, declaration);
            }, declaration->expr->node);

        return asm_code;
    }

    std::optional<std::string> generate_arg_assign(const std::vector<ExprNode*>& args)
    {
        if (args.size() > 16)
        {
            Console::CompilerError("Maximum argument count (16) exceeded");
            return std::nullopt;
        }

        // Register map for arguments
        std::map<int, std::string> registers = {
            {0, "rdi"}, {1, "rsi"}, {2, "rdx"}, {3, "rcx"},
            {4, "r8"},  {5, "r9"},  {6, "r10"}, {7, "r11"},
            {8, "r12"}, {9, "r13"}, {10, "r14"}, {11, "r15"},
        };

        std::string instructions;
        int arg_idx = 0;

        for (const auto& __arg : args)
        {
            if (!__arg)
                continue;

            std::visit([&](auto& arg) {
                auto it = registers.find(arg_idx);
                if (it == registers.end()) return;

                auto is_string = arg->val.type == TokenType::STRING_LIT;

                std::string ident = is_string ?
                    generate_temporary_string(__arg, arg_idx) : arg->val.value;

                instructions += "    mov " + it->second + ", " + ident + LNBR;

                if (is_string)
                {
                    arg_idx++;
                    auto len_register = registers.find(arg_idx);

                    if (len_register == registers.end())
                        Console::CompilerError("illformed argument assignment");

                    instructions += "    mov" + len_register->second + ", " + std::to_string(arg->val.value.length() + 1) + LNBR;
                }

                arg_idx++;
                }, __arg->node);
        }

        return instructions;
    }

    std::string generate_temporary_string(ExprNode* arg, int arg_idx)
    {
        if (!arg)
            return "";

        return std::visit([&](const auto& obj) -> std::string {
            using T = std::decay_t<decltype(obj)>;

            if constexpr (std::is_same_v<T, StringLitNode*>)
            {
                std::string var_id = "__" + std::to_string(temp_label_counter++) + "_arg" + std::to_string(arg_idx);

                auto* decl = new LocalDeclNode{
                    {TokenType::IDENTIFIER, var_id, obj->val.line, obj->val.column}, nullptr, false
                };

                decl->expr = new ExprNode{
                    ExprNode(new StringLitNode{obj->val})
                };

                section_data += generate_declaration(decl);

                return var_id;
            }
            else
                return "";
            }, arg->node);
    }

    std::optional<std::string> generate_func_call(FuncCallNode* call)
    {
        if (!call)
            return std::nullopt;

        std::string arg_assignments = generate_arg_assign(call->args).value_or("");
        std::string func_call = "    call " + call->ident.value + "\n";

        return arg_assignments + func_call;
    }

    std::optional<std::string> generate_scope(ScopeNode* scope)
    {
        std::string scope_str;

        for (size_t i = 0; i < scope->stmts.size(); ++i)
        {
            auto& content = scope->stmts[i];

            if (!content)
                continue;

            std::function<void(StmtNode*)> handle_stmt = [&](StmtNode* stmt) -> void {
                if (!stmt || stmt->stmt.valueless_by_exception())
                    return;

                std::visit([&](auto& obj) {
                    using T = std::decay_t<decltype(obj)>;

                    if constexpr (std::is_same_v<T, LocalDeclNode*>)
                        scope_str += generate_declaration(obj);
                    else if constexpr (std::is_same_v<T, FuncCallNode*>)
                        scope_str += generate_func_call(obj).value_or("");
                    else if constexpr (std::is_same_v<T, IfStmtNode*>)
                        scope_str += generate_if_stmt(obj);
                    else if constexpr (std::is_same_v<T, ScopeNode*>)
                        scope_str += generate_scope(obj).value_or("");
                    }, stmt->stmt);
                };

            handle_stmt(content);
        }

        return scope_str;
    }

    std::optional<std::string> generate_function(FuncNode* func)
    {
        if (!func)
            return std::nullopt;

        temp_label_counter++;
        std::string scope_id = func->ident.value;

        section_text += LNBR + scope_id + ":\n";
        section_text += generate_function_prologue();
        section_text += generate_scope(func->body).value_or("");
        section_text += generate_function_epilogue();

        return "    call " + scope_id + LNBR;
    }

    // Generate if statement
    std::string generate_if_stmt(IfStmtNode* if_stmt)
    {
        auto if_pred = if_stmt->if_pred;
        auto _condition = if_pred->condition; // condition must be a pointer to a bitbuf
        auto then_scope = if_pred->then_scope;
        auto else_scope = if_pred->else_scope;

        std::string then_id = ".then" + std::to_string(temp_label_counter++);
        std::string else_id = ".else" + std::to_string(temp_label_counter++);

        auto condition = std::visit([&](const auto& expr) {
            return expr->val.value;
            }, _condition->node);

        std::string head =
            "    movzx eax, byte [cond]\n"
            "    test eax, eax\n"
            "    jz " + else_id + "\n\n"
            + generate_scope(then_scope).value_or("")
            + "    ret\n";

        std::string else_instr = "    " + else_id + ":\n"
            + generate_scope(else_scope.value()).value_or("");

        std::cout << head << else_instr << std::endl;

        return head + else_instr;
    }

    // Other utility functions 
    void generate_template()
    {
        section_text = "section .text\n    global _start\n";
        section_data = "section .data\n";
        func_main = "_main:\n";
        func_start = "_start:\n    call _main\n    mov rdi, 0\n    call exit\n";
    }

    std::string build_source()
    {
        return get_std() + section_text + LNBR + LNBR + func_main + "    ret\n" + LNBR + func_start + LNBR + section_data;
    }

    std::string get_std()
    {
        return "%include '../std/std.asm'\n\n";
    }

    void create_output_file(const std::string& out_src)
    {
        std::ofstream out_file(write_file);
        if (out_file.is_open())
        {
            out_file << out_src;
            out_file.close();
        }
        else
        {
            std::cerr << "Error opening file: " << write_file << std::endl;
        }
    }
};

