#pragma once

#include <optional>
#include <list>
#include <map>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <stack>
#include <variant>
#include <tuple>

#include "../../parser/parser.hpp"

const size_t MAX_STACK_SIZE = 1024 * 1024;

class StackFrame {
public:
    StackFrame()
        : variables({}) {}

    void set(const std::string& name, std::unique_ptr<StmtNode> stmt)
    {
        variables[name] = std::move(stmt);
    }

    StmtNode* get(const std::string& name)
    {
        if (variables.empty())
            throw std::runtime_error("Bad stack frame access: stack frame is empty\n");

        auto it = variables.find(name);

        if (it == variables.end())
            throw std::runtime_error("Bad stack frame access: stack frame does not have member named '" + name + "'\n");

        // Return raw pointer instead of moving
        return it->second.get();
    }

private:
    std::map<std::string, std::unique_ptr<StmtNode>> variables;
};

class Stack {
public:
    void push()
    {
        if (m_stack.size() >= MAX_STACK_SIZE)
            throw std::runtime_error("Stack overflow");

        m_stack.push(StackFrame());
    }

    void pop()
    {
        if (!m_stack.empty())
            m_stack.pop();
        else
            throw std::runtime_error("Stack underflow");
    }

    StackFrame& top()
    {
        return m_stack.top();
    }

private:
    std::stack<StackFrame> m_stack;
};

class Heap {
private:
    std::map<void*, std::size_t> allocations;

public:
    void* alloc(std::size_t size) {
        void* ptr = std::malloc(size);

        if (ptr != nullptr)
            allocations[ptr] = size;

        return ptr;
    }

    void free(void* ptr) {
        auto it = allocations.find(ptr);

        if (it != allocations.end()) {
            std::size_t size = it->second;
            std::free(ptr);

            allocations.erase(it);
        }
    }

    ~Heap() {
        for (const auto& alloc : allocations)
            std::free(alloc.first);

        allocations.clear();
    }
};

class Interpreter
{
public:

    Interpreter(ProgNode prog_node)
        : prog_node(prog_node)
        , m_stack(Stack())
        , m_heap(Heap()) {}

    void run()
    {
        std::cout << "interpreter start\n";

        for (const auto& stmt : prog_node.stmts)
            solve_stmt(stmt);

        std::cout << "interpreter end\n";
    }

private:

    std::tuple<std::string, std::string> solve_param(ExprNode* _arg)
    {
        return std::visit([&](const auto& arg) {
            using T = std::decay_t<decltype(arg)>;

            std::string type;

            if constexpr (std::is_same_v<T, IntLitNode*>)
                type = "int";
            else if constexpr (std::is_same_v<T, StringLitNode*>)
                type = "string";
            else if constexpr (std::is_same_v<T, BoolLitNode*>)
                type = "bool";
            else
                type = "<error-type>";

            return std::make_tuple(type, arg->val.value);
            }, _arg->node);
    }

    void solve_call(FuncCallNode* call)
    {
        auto raw_func_object = m_stack.top().get(call->ident.value);
        auto func = std::get_if<FuncNode*>(&raw_func_object->stmt);

        if (!func)
            error(call->ident.line, "Attempt to call non-function value for identifier '" + call->ident.value + "'.");

        int arg_idx = 0;
        for (auto& _arg : call->args)
        {
            if (arg_idx >= (*func)->params.size())
                error(call->ident.line, "Too many arguments passed to function '" + call->ident.value + "'.");

            auto param_data = solve_param(_arg);

            StmtNode stmt = { _arg };
            StmtNode* p_stmt = &stmt;

            (*func)->body->stmts.push_back(p_stmt);

            arg_idx++;
        }

        solve_scope((*func)->body);
    }

    void solve_decl(LocalDeclNode* decl)
    {
        m_stack.top().set(decl->ident.value, std::make_unique<StmtNode>(decl));
    }

    void solve_stmt(StmtNode* stmt)
    {
        std::visit([&](const auto& stmt) {
            using T = std::decay_t<decltype(stmt)>;

            if constexpr (std::is_same_v<T, LocalDeclNode*>)
                solve_decl(stmt);
            else if constexpr (std::is_same_v<T, FuncCallNode*>)
                solve_call(stmt);
            }, stmt->stmt);
    }

    void solve_scope(ScopeNode* scope)
    {
        m_stack.push();

        for (const auto& stmt : scope->stmts)
            solve_stmt(stmt);

        m_stack.pop();
    }

    void error(int line, std::string msg)
    {
        throw std::runtime_error("at line " + std::to_string(line) + ": " + msg + "\n");
    }

    ProgNode prog_node;
    Stack m_stack;
    Heap m_heap;

};