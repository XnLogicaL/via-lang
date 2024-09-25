#pragma once

#include <optional>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>
#include <functional>
#include <memory>
#include <vector>

#include "../../parser/parser.hpp"
#include "../../parser/type.hpp"
#include "heap.hpp"
#include "stack.hpp"

const size_t MAX_STACK_SIZE = 1024 * 1024;

namespace BuiltIn
{
    FuncNode print{
        .ident = Token{TokenType::IDENTIFIER, "print"},
        .params = {ParamNode{Token{TokenType::IDENTIFIER, "__out"}, Token{TokenType::TYPE, "string"}}},

        .c_hook = [](std::vector<ExprNode*> args) {
            auto msg = TypeConverter::to_string(args.at(0));
            std::cout << msg.val.value << std::endl;
        }
    };

    template <typename _Ty>
    ExprNode* as_expr_ptr(_Ty expr)
    {
        auto _expr = ExprNode{ &expr };
        return &_expr;
    }
};

class Interpreter
{
public:
    Interpreter(ProgNode prog_node)
        : prog_node(prog_node)
        , m_stack(Stack(MAX_STACK_SIZE))
        , m_heap(Heap()) {}

    void init()
    {
        declare_global("print", std::make_unique<ExprNode>(*BuiltIn::as_expr_ptr(BuiltIn::print)));
    }

    void run()
    {
        for (const auto &stmt : prog_node.stmts)
            eval_stmt(stmt);
    }

    void declare(const std::string& name, std::unique_ptr<ExprNode> expr)
    {
        m_stack.set(name, std::move(expr));
    }

    void declare_global(const std::string& name, std::unique_ptr<ExprNode> expr)
    {
        m_stack.set_global(name, std::move(expr));
    }

    void mutate(const std::string &name, std::string new_value)
    {
        if (auto _ = std::get_if<NilNode*>(&m_stack.get_global(name)->node))
        {
            std::visit([&new_value](const auto& expr) {
                expr->val.value = new_value;
            }, m_stack.get(name)->node);
            return;
        }

        vm_error_inline("global variable cannot be assigned to", m_stack.get(name)->get_line());
    }

    void call(FuncCallNode* call_data, FuncNode* func)
    {
        if (func->c_hook)
        {
            func->c_hook(call_data->args);
            return;
        }

        m_stack.push();

        for (size_t arg_idx = 0; arg_idx < call_data->args.size(); ++arg_idx)
        {
            auto& arg = call_data->args.at(arg_idx);
            const auto& param = func->params.at(arg_idx).ident;

            if (TypeChecker::as_itype(param) != TypeChecker::as_itype(param))
                vm_error_inline("cannot pass parameter to function: type mismatch", param.line);

            declare(param.value, std::make_unique<ExprNode>(*arg));
        }

        for (const auto& scope_stmt : func->body->stmts)
            eval_stmt(scope_stmt);

        m_stack.pop();
    }

private:
    ProgNode prog_node;
    Stack m_stack;
    Heap m_heap;

    void eval_stmt(StmtNode* stmt)
    {
        if (auto expr_stmt = std::get_if<ExprNode*>(&stmt->stmt))
            eval_expr(*expr_stmt);
        else if (auto decl_stmt = std::get_if<LocalDeclNode*>(&stmt->stmt))
        {
            auto decl = *decl_stmt;
            auto ident = decl->ident.value;
            auto expr = decl->expr;

            declare(ident, std::make_unique<ExprNode>(*expr));
        }
    }

    int eval_expr(ExprNode* expr)
    {
        if (auto func_call = std::get_if<FuncCallNode*>(&expr->node))
        {
            eval_func_call(*func_call);
            return NULL;
        }
        else if (auto bin_expr = std::get_if<BinExprNode*>(&expr->node))
            return eval_binop(*bin_expr);
        else if (auto int_lit = std::get_if<IntLitNode*>(&expr->node))
            return std::stoi((*int_lit)->val.value);

        vm_error_generic("Unsupported expression type");
    }

    int eval_binop(BinExprNode* bin_expr)
    {
        // Evaluate left and right operands
        int lhs = eval_expr(bin_expr->lhs);
        int rhs = eval_expr(bin_expr->rhs);

        switch (bin_expr->op.type)
        {
        case TokenType::ADD:
            return lhs + rhs;
        case TokenType::SUB:
            return lhs - rhs;
        case TokenType::MUL:
            return lhs * rhs;
        case TokenType::DIV:
            if (rhs == 0)
                vm_error_inline("Division by zero", bin_expr->op.line);
            return lhs / rhs;
        default:
            vm_error_inline("Unknown binary operator", bin_expr->op.line);
        }

        return NULL;
    }

    void eval_func_call(FuncCallNode* call_data)
    {
        // Retrieve the function object from the function call
        auto callee = m_stack.top().get(call_data->ident.value)->node;

        if (auto func = std::get_if<FuncNode*>(&callee))
            call(call_data, *func);
        else
            vm_error_inline("attempt to call non-callable value", call_data->ident.line);
    }

    void vm_error_generic(std::string msg)
    {
        throw std::runtime_error(msg);
    }

    void vm_error_inline(std::string msg, int line, int column = -1)
    {
        vm_error_generic(prog_node.prog_name + ":" + std::to_string(line) + ": " + msg);
    }
};
