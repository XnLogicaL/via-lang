#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <memory>
#include <any>

#include "utils.hpp"

enum DeclarationType
{
    VAR,
    FUNCTION,
};

std::ostream &operator<<(std::ostream &os, const DeclarationType &obj)
{
    std::string ss;
    switch (obj)
    {
    case DeclarationType::VAR:
        ss = "VAR";
        break;
    case DeclarationType::FUNCTION:
        ss = "FUNCTION";
        break;
    default:
        ss = "<error-type>";
    }
    return os << ss;
}

enum DeclarationEnvironment
{
    LOCAL,
    GLOBAL
};

std::ostream &operator<<(std::ostream &os, const DeclarationEnvironment &obj)
{
    std::string ss;
    switch (obj)
    {
    case DeclarationEnvironment::LOCAL:
        ss = "LOCAL";
        break;
    case DeclarationEnvironment::GLOBAL:
        ss = "GLOBAL";
        break;
    default:
        ss = "<error-env>";
    }
    return os << ss;
}

enum ValueType
{
    INT,
    REFERENCE,
    PLACEHOLDER,
};

std::ostream &operator<<(std::ostream &os, const ValueType &obj)
{
    std::string ss;
    switch (obj)
    {
    case ValueType::INT:
        ss = "INT";
        break;
    case ValueType::PLACEHOLDER:
        ss = "PLACEHOLDER";
        break;
    case ValueType::REFERENCE:
        ss = "REFERENCE";
        break;
    default:
        ss = "<error-value-type>";
    }
    return os << ss;
}

class FunctionCall;
class Declaration;
class Value;

typedef std::vector<std::any> ScopeType;
typedef std::variant<std::shared_ptr<Declaration>, std::shared_ptr<ScopeType>, std::shared_ptr<FunctionCall>> ScopeContent;

typedef std::variant<std::shared_ptr<std::string>, std::shared_ptr<int>> ValueValue;

ScopeType nullscope = {};
ScopeType* p_nullscope = &nullscope;

class Value
{
public:
    ValueType value_type;
    ValueValue value;

    Value(ValueType v_type = ValueType::PLACEHOLDER, ValueValue v_value = std::make_shared<int>(0))
        : value_type(v_type), value(v_value) {}

    friend std::ostream &operator<<(std::ostream &os, const Value &obj)
    {
        std::string value_string = std::visit([&](const auto &_value) -> std::string
                                              {
        using T = std::decay_t<decltype(_value)>;
        if constexpr (std::is_same_v<T, std::shared_ptr<std::string>>)
            return *_value;
        else if constexpr (std::is_same_v<T, std::shared_ptr<int>>)
            return std::to_string(*_value);
        return "<error-value>"; }, obj.value);

        os << "Value(Type: " << obj.value_type << ", Value: ";
        switch (obj.value_type)
        {
        case ValueType::PLACEHOLDER:
            os << "<placeholder>";
            break;
        default:
            os << value_string;
        }
        os << ")";
        return os;
    }
};

class Declaration
{
public:
    std::string id;
    DeclarationType type;
    DeclarationEnvironment env;
    Value val;
    bool is_const;
    std::vector<Declaration> parameters;
    std::optional<std::shared_ptr<ScopeType>> body;
    ScopeType *parent; // Parent scope, initialized as nullptr

    Declaration(
        const std::string &declr_id = "<error-id>",
        ScopeType *parent = nullptr, // Initialize the parent as nullptr by default
        bool is_const = false,
        DeclarationType declr_type = DeclarationType::VAR,
        DeclarationEnvironment declr_env = DeclarationEnvironment::LOCAL,
        Value declr_val = Value())
        : id(declr_id), parent(parent), is_const(is_const), type(declr_type), env(declr_env), val(declr_val)
    {
    }

    Declaration(
        const std::string &declr_id = "<error-id>",
        ScopeType *parent = nullptr, // Initialize the parent as nullptr by default
        bool is_const = false,
        DeclarationEnvironment declr_env = DeclarationEnvironment::LOCAL,
        const std::vector<Declaration> &params = std::vector<Declaration>(),
        std::shared_ptr<ScopeType> func_body = std::make_shared<ScopeType>(std::vector<std::any>()))
        : id(declr_id), parent(parent), is_const(is_const), type(DeclarationType::FUNCTION), env(declr_env), parameters(params), body(func_body)
    {
    }

    friend std::ostream &operator<<(std::ostream &os, const Declaration &obj)
    {
        os << "Declaration(ID: " << obj.id
           << ", Type: " << obj.type
           << ", Env: " << obj.env;

        if (obj.type == DeclarationType::FUNCTION)
        {
            os << ", Parameters: [";
            for (const auto &param : obj.parameters)
            {
                os << param.id << " ";
            }
            os << "], Body: " << (obj.body ? "defined" : "null");
        }
        else
        {
            os << ", Val: " << obj.val;
        }

        os << ", Parent: " << (obj.parent ? std::any_cast<std::string>((*obj.parent).at(0)) : "<null>") << ")";
        return os;
    }
};

class FunctionCall
{
public:
    std::string id;
    std::vector<Value> args;

    FunctionCall(std::string func_id, std::vector<Value> args)
        : id(func_id), args(args) {}

    friend std::ostream &operator<<(std::ostream &os, const FunctionCall &obj)
    {
        os << "FunctionCall(Func: " + obj.id + ", Args: [";
        for (const auto &arg : obj.args)
        {
            os << arg << ", ";
        }
        os << "<args-end>])";
        return os;
    }
};

namespace Scope
{
    ScopeType create_scope(std::string id, std::vector<ScopeContent> contents, ScopeType *parent = nullptr)
    {
        ScopeType scope;
        scope.push_back(id);
        scope.push_back(parent);
        scope.push_back(contents);
        return scope;
    }

    std::string get_scope_id(ScopeType &scope)
    {
        return std::any_cast<std::string>(scope.at(0));
    }

    std::string get_scope_id(ScopeType* scope)
    {
        return std::any_cast<std::string>(scope->at(0));
    }

    ScopeType *get_scope_parent(ScopeType &scope)
    {
        auto parent = std::any_cast<ScopeType *>(scope.at(1));
        return parent;
    }

    std::vector<ScopeContent> *get_scope_contents(ScopeType &scope)
    {
        auto &content = std::any_cast<std::vector<ScopeContent> &>(scope.at(2));
        return &content;
    }

    std::vector<ScopeType *> get_scope_ancestry(ScopeType &scope)
    {
        std::vector<ScopeType *> ancestors;
        ScopeType *current_ancestor = &scope;

        while (current_ancestor != nullptr)
        {
            ancestors.push_back(current_ancestor);
            current_ancestor = Scope::get_scope_parent(*current_ancestor);

            if (current_ancestor == nullptr)
                break;

            std::cout << "Next ancestor address: " << current_ancestor << std::endl;
            std::cout << "Next ancestor ID: " << Scope::get_scope_id(current_ancestor) << std::endl;
        }

        return ancestors;
    }

    std::optional<ScopeContent> get_scope_child(ScopeType scope, std::string id)
    {
        for (const auto &content : *Scope::get_scope_contents(scope))
        {
            bool is_valid = std::visit([&id](auto &obj) -> bool
                                       {
                using T = std::decay_t<decltype(obj)>;

                if constexpr (std::is_same_v<T, std::shared_ptr<Declaration>>)
                    return obj->id == id;
                else if constexpr (std::is_same_v<T, std::shared_ptr<ScopeType>>)
                    return Scope::get_scope_id(*obj) == id;
                else if constexpr (std::is_same_v<T, std::shared_ptr<FunctionCall>>)
                    return obj->id == id;

                return false; }, content);

            if (is_valid)
                return content;
        }

        return std::nullopt;
    }
}

class Parser
{
public:
    std::vector<Token> toks;
    int i = 0;

    Parser(std::vector<Token> tokens) : toks(tokens) {}

    std::optional<ScopeType> parse_scope(std::string scope_id, ScopeType *parent = nullptr)
    {
        if (peek().type != TokenType::L_CR_BRACKET)
        {
            unexpected_token_error(peek(), "Expected '{' to start scope body");
            return std::nullopt;
        }

        consume();

        ScopeType new_scope = Scope::create_scope(scope_id, {}, parent);
        auto scope_contents = Scope::get_scope_contents(new_scope);

        while (true)
        {
            Token current_token = peek();

            std::cout << current_token << std::endl;

            if (current_token.type == TokenType::END)
            {
                unexpected_token_error(current_token, "Unexpected end of input inside scope");
                return std::nullopt;
            }
            else if (current_token.type == TokenType::R_CR_BRACKET)
            {
                consume();
                break;
            }
            else if (current_token.type == TokenType::L_CR_BRACKET)
            {
                auto nested_scope = parse_scope(UUID::generate_uuid_v4(), &new_scope);

                if (nested_scope.has_value())
                {
                    scope_contents->push_back((ScopeContent)std::make_shared<ScopeType>(nested_scope.value()));
                    continue;
                }
                else
                {
                    parse_failed_error("malformed or invalid scope body");
                    return std::nullopt;
                }
            }
            else if (current_token.type == TokenType::IDENTIFIER && peek(1).type == TokenType::L_PAR)
            {
                auto func_call = parse_func_call(&new_scope);
                if (func_call.has_value())
                {
                    scope_contents->push_back((ScopeContent)func_call.value());
                    continue;
                }
                else
                {
                    parse_failed_error("malformed or invalid function call");
                    return std::nullopt;
                }
            }
            else if (
                current_token.type == TokenType::KEYWORD && current_token.value == "local" 
                && (peek(1).type == TokenType::IDENTIFIER || peek(1).type == TokenType::EXCLAMATION))
            {
                auto declaration = parse_declr(&new_scope);

                if (declaration.has_value())
                {
                    scope_contents->push_back((ScopeContent)declaration.value());
                    continue;
                }
                else
                {
                    parse_failed_error("malformed or invalid declaration");
                    return std::nullopt;
                }
            }
            else
            {
                unexpected_token_error(current_token, "expected expression or declaration");
                return std::nullopt;
            }

            consume();
        }

        return new_scope;
    }

    std::optional<std::vector<Declaration>> get_arguments()
    {
        std::vector<Declaration> args;
        bool expecting_identifier = true;

        if (peek().type != TokenType::L_PAR)
        {
            unexpected_token_error(peek(), "Expected '(' for argument list");
            return std::nullopt;
        }

        consume();

        while (peek().type != TokenType::R_PAR)
        {
            if (peek().type == TokenType::IDENTIFIER && expecting_identifier)
            {
                std::string current_identifier = peek().value;
                consume();

                if (peek().type != TokenType::COLON)
                {
                    unexpected_token_error(peek(), "Expected ':' after argument identifier");
                    return std::nullopt;
                }

                consume();

                if (peek().type != TokenType::TYPE)
                {
                    unexpected_token_error(peek(), "Expected type after ':' in argument");
                    return std::nullopt;
                }

                std::string current_type = peek().value;
                consume();

                ValueType arg_type = (current_type == "int") ? ValueType::INT : ValueType::PLACEHOLDER;
                args.push_back(Declaration(current_identifier, nullptr, true, DeclarationType::VAR, DeclarationEnvironment::LOCAL, Value(arg_type)));

                if (peek().type == TokenType::COMMA)
                    consume();
            }
            else
            {
                unexpected_token_error(peek(), "Unexpected token in argument list");
                return std::nullopt;
            }
        }

        consume();

        return args;
    }

    std::optional<std::shared_ptr<Declaration>> parse_declr(ScopeType *parent = nullptr)
    {
        if (peek().type != TokenType::KEYWORD || (peek().value != "local" && peek().value != "global"))
        {
            unexpected_token_error(peek(), "Expected environment specifier keyword");
            return std::nullopt;
        }

        DeclarationEnvironment env = (peek().value == "local") ? DeclarationEnvironment::LOCAL : DeclarationEnvironment::GLOBAL;
        consume();

        bool is_const = false;
        if (peek().type == TokenType::EXCLAMATION)
        {
            is_const = true;
            consume();
        }

        if (peek().type != TokenType::IDENTIFIER)
        {
            unexpected_token_error(peek(), "Expected an identifier");
            return std::nullopt;
        }

        std::string var_id = peek().value;
        consume();

        if (peek().type == TokenType::COLON)
        {
            consume();

            if (peek().type != TokenType::TYPE)
            {
                unexpected_token_error(peek(), "Expected a type identifier after ':'");
                return std::nullopt;
            }

            consume();
        }

        if (peek().type == TokenType::EQUALS)
        {
            consume();

            if (peek().type == TokenType::KEYWORD && peek().value == "function")
            {
                consume();
                std::vector<Declaration> params = get_arguments().value_or(std::vector<Declaration>());

                if (peek().type != TokenType::L_CR_BRACKET)
                {
                    unexpected_token_error(peek(), "Expected '{' for function body");
                    return std::nullopt;
                }

                auto body_scope_opt = parse_scope("__func_" + var_id, parent);

                if (!body_scope_opt.has_value())
                {
                    parse_failed_error("malformed or invalid function body");
                    return std::nullopt;
                }

                auto func_body_ptr = std::make_shared<ScopeType>(body_scope_opt.value());

                return std::make_shared<Declaration>(Declaration(var_id, parent, is_const, env, params, func_body_ptr));
            }
            else if (peek().type == TokenType::INT_LIT)
            {
                Value var_val(ValueType::INT, std::make_shared<int>(std::stoi(peek().value)));
                consume();

                return std::make_shared<Declaration>(Declaration(var_id, parent, is_const, DeclarationType::VAR, env, var_val));
            }
        }

        return std::nullopt;
    }

    std::optional<std::shared_ptr<FunctionCall>> parse_func_call(ScopeType *parent = nullptr)
    {
        if (peek().type != TokenType::IDENTIFIER)
        {
            unexpected_token_error(peek(), "Expected identifier for function call");
            return std::nullopt;
        }

        std::string func_id = peek().value;
        consume();

        if (peek().type != TokenType::L_PAR)
        {
            unexpected_token_error(peek(), "Expected '(' to open function arguments");
            return std::nullopt;
        }

        consume();

        std::vector<Value> func_args;

        while (peek().type != TokenType::R_PAR)
        {
            if (peek().type == TokenType::END)
            {
                unexpected_token_error(peek(), "Expected ')' to close function arguments");
                return std::nullopt;
            }

            Value value;
            if (peek().type == TokenType::INT_LIT)
            {
                value.value_type = ValueType::INT;
                value.value = std::make_shared<int>(std::stoi(peek().value));
            }
            else if (peek().type == TokenType::IDENTIFIER)
            {
                value.value_type = ValueType::REFERENCE;
                value.value = std::make_shared<std::string>(peek().value);
            }
            else
            {
                unexpected_token_error(peek(), "Unexpected token in function arguments");
                return std::nullopt;
            }

            func_args.push_back(value);
            consume();

            if (peek().type == TokenType::COMMA)
            {
                consume();
            }
            else if (peek().type != TokenType::R_PAR)
            {
                unexpected_token_error(peek(), "Expected ',' or ')' after argument");
                return std::nullopt;
            }
        }

        consume();

        return std::make_shared<FunctionCall>(FunctionCall(func_id, func_args));
    }

private:
    Token peek(int ahead = 0)
    {
        return toks.at(i + ahead);
    }

    void consume(int ahead = 1)
    {
        i += ahead;
    }

    void unexpected_token_error(const Token &token, const std::string &message)
    {
        std::cout << "Parse error: " << message << " got '" << token.value << "' Token(" << token.type << ")" << std::endl;
        std::cerr << "  at line " << token.line << " column " << token.column << std::endl;
    }

    void parse_failed_error(const std::string message)
    {
        std::cout << "Parse error: cannot parse scope: " << message << std::endl;
        std::cerr << "  at line " << peek().line << " column " << peek().column << std::endl;
    }
};

namespace Debug
{
    std::string get_scope_string(ScopeType &scope)
    {
        const int TAB_SIZE = 2;

        const auto ancestry = Scope::get_scope_ancestry(scope);
        const int ancestry_size = ancestry.size();

        std::string tab(TAB_SIZE, ' ');

        std::string scope_white_space(ancestry_size * TAB_SIZE, ' ');
        std::string member_white_space = scope_white_space + tab;
        std::string content_white_space = member_white_space + tab;

        std::ostringstream scope_stream;

        scope_stream << scope_white_space << "Scope(\n"
                     << member_white_space << "ID: " << Scope::get_scope_id(scope) << "\n"
                     << member_white_space << "Parent: ";

        //auto parent_scope = Scope::get_scope_parent(scope);
        scope_stream << "<undefined>" << "\n";
        scope_stream << member_white_space << "Contents: {\n";

        for (const auto &content : (*Scope::get_scope_contents(scope)))
        {
            auto object_string = std::visit([](auto &obj)
                                            {
                using T = std::decay_t<decltype(obj)>;
                std::ostringstream output_stream;

                std::cout << typeid(obj).name() << std::endl;

                if constexpr (std::is_same_v<T, std::shared_ptr<Declaration>>)
                {
                    std::cout << "is a declaration\n";
                    output_stream << *obj;
                    std::cout << *obj << std::endl;
                }
                else if constexpr (std::is_same_v<T, std::shared_ptr<ScopeType>>)
                {
                    std::cout << "is a scope\n";
                    output_stream << get_scope_string(*obj);
                    std::cout << get_scope_string(*obj) << std::endl;
                }
                else if constexpr (std::is_same_v<T, std::shared_ptr<FunctionCall>>)
                {
                    std::cout << "is a func call\n";
                    output_stream << *obj;
                    std::cout << *obj << std::endl;
                }
                else
                {
                    return "<unknown content>";
                } return output_stream.str(); }, content);

            std::cout << object_string << std::endl;

            scope_stream << content_white_space << object_string << "\n";
        }

        scope_stream << member_white_space << "}\n"
                     << scope_white_space << ")";

        return scope_stream.str();
    }
}