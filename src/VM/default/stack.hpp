#pragma once

#include <optional>
#include <map>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <stack>
#include <tuple>

#include "../../parser/parser.hpp"

class StackFrame {
public:
    StackFrame()
        : variables({}) {}

    void set(const std::string& name, std::unique_ptr<ExprNode> expr)
    {
        variables[name] = std::move(expr);  // Transfer ownership
    }

    ExprNode* get(const std::string& name)
    {
        auto it = variables.find(name);

        if (it == variables.end())
        {
            static NilNode nil_expr;
            static ExprNode expr{ &nil_expr };
            return &expr;  // Return reference to static, shared object
        }

        return it->second.get();  // Return raw pointer managed by unique_ptr
    }

private:
    std::map<std::string, std::unique_ptr<ExprNode>> variables;
};

class Stack {
public:
    Stack(int max_size)
        : MAX_STACK_SIZE(max_size)
        , m_global(StackFrame()) {}

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

    StackFrame& global()
    {
        return m_global;
    }

    ExprNode* get(const std::string& name)
    {
        return top().get(name);
    }

    void set(const std::string& name, std::unique_ptr<ExprNode> expr)
    {
        top().set(name, std::move(expr));  // Transfer ownership
    }

    void set_global(const std::string& name, std::unique_ptr<ExprNode> expr)
    {
        global().set(name, std::move(expr));  // Transfer ownership
    }

    ExprNode* get_global(const std::string& name)
    {
        return global().get(name);
    }

private:
    std::stack<StackFrame> m_stack;
    StackFrame m_global;

    size_t MAX_STACK_SIZE;
};