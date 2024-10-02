#pragma once

#include <optional>
#include <map>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <stack>
#include <vector>
#include <iostream>

#include "../../parser/parser.hpp"

#include "runtime/variable.hpp"

class StackFrame {
public:

    void set(const std::string ident, std::unique_ptr<Variable> expr)
    {
        variables[ident] = std::move(expr);
    }

    std::optional<Variable*> get(const std::string ident)
    {
        auto it = variables.find(ident);

        if (it == variables.end())
            crash("Bad StackFrame access");

        return it->second.get();
    }

private:
    std::map<std::string, std::unique_ptr<Variable>> variables;
};

class Stack {
public:
    Stack(size_t max_size)
        : MAX_STACK_SIZE(max_size) {}

    void push()
    {
        if (m_stack.size() >= MAX_STACK_SIZE)
            crash("Stack overflow");

        m_stack.push(StackFrame());
    }

    void pop()
    {
        if (!m_stack.empty())
            m_stack.pop();
        else
            crash("Stack underflow");
    }

    StackFrame& top()
    {
        if (m_stack.empty())
            crash("Stack underflow");

        return m_stack.top();
    }

private:
    std::stack<StackFrame> m_stack;
    size_t MAX_STACK_SIZE;
};