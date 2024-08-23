#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <variant>

// Include headers for Variable and Expression if needed
#include "variable.hpp"
#include "expression.hpp"
#include "utils.hpp"

class Scope; // Forward declaration of Scope

typedef std::variant<std::shared_ptr<Variable>, std::shared_ptr<Scope>> ScopeContent;
typedef std::vector<ScopeContent> ScopeContents;

class Scope
{
public:
    std::string id;
    ScopeContents contents;
    Scope *parent = nullptr;

    std::vector<Scope *> get_ancestry() const
    {
        std::vector<Scope *> ancestry;
        Scope *current_scope = this->parent;

        while (current_scope != nullptr)
        {
            ancestry.push_back(current_scope);
            current_scope = current_scope->parent;
        }

        return ancestry;
    }

    std::variant<ScopeContent, int> get_child(std::string c_id)
    {
        for (auto &&_child : contents)
        {
            bool found_child = false;
            ScopeContent fnd_child;

            std::visit([&](auto &&child) {
                if (child->id == c_id)
                {
                    found_child = true;
                    fnd_child = child;
                }
            }, _child);

            if (found_child)
                return fnd_child;
        }

        return NULL;
    }

    // Constructor with both parameters optional
    Scope(const std::string &s_id = "*unknown-scope*", ScopeContents s_contents = {})
        : id(s_id), contents(std::move(s_contents))
    {
        for (auto &content : contents)
        {
            if (std::holds_alternative<std::shared_ptr<Scope>>(content))
            {
                std::shared_ptr<Scope> &child_scope = std::get<std::shared_ptr<Scope>>(content);
                child_scope->parent = this;
            }
        }
    }
};

std::ostream &operator<<(std::ostream &os, const Scope &scope)
{
    std::string parent_scope_id = "<none>";

    if (scope.parent != nullptr)
        parent_scope_id = scope.parent->id;

    const auto ancestors = scope.get_ancestry();
    const int ancestery_size = ancestors.size();
    std::string ext_ws = std::string(ancestery_size * 4, ' ');
    std::string in_ws = ext_ws + "  ";
    std::string inn_ws = in_ws + "  ";

    os << "Scope{\n"
       << in_ws << "ParentID: \"" << parent_scope_id << "\"\n"
       << in_ws << "ID: \"" << scope.id << "\"\n"
       << in_ws << "Contents{\n";

    for (const auto &scope_content : scope.contents)
    {
        std::visit([&os, &inn_ws](const auto child)
        {os << inn_ws << *child << "\n"; }, scope_content);
    }

    os << in_ws << "}\n"
       << ext_ws << "}";
    return os;
}