/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "const.hpp"
#include <format>
#include <optional>
#include "debug.hpp"
#include "support/conv.hpp"

std::optional<via::ConstValue> via::ConstValue::from_token(const Token& tok)
{
    switch (tok.kind) {
    case TokenKind::LIT_NIL:
        return ConstValue();
    case TokenKind::LIT_TRUE:
        return ConstValue(true);
    case TokenKind::LIT_FALSE:
        return ConstValue(false);
    case TokenKind::LIT_INT:
    case TokenKind::LIT_XINT:
    case TokenKind::LIT_BINT:
        if (auto val = stoi<int64_t>(tok.to_string()))
            return ConstValue(*val);
        break;
    case TokenKind::LIT_FLOAT:
        if (auto val = stof<double_t>(tok.to_string()))
            return ConstValue(*val);
        break;
    case TokenKind::LIT_STRING: {
        std::string_view view = tok.to_sv();
        std::string literal(view.begin() + 1, view.end() - 1);
        return ConstValue(literal);
    }
    default:
        break;
    }

    return std::nullopt;
}

std::string via::ConstValue::to_string() const
{
    switch (kind()) {
    case ValueKind::NIL:
        return "nil";
    case ValueKind::BOOL:
        return value<ValueKind::BOOL>() ? "true" : "false";
    case ValueKind::INT:
        return std::to_string(value<ValueKind::INT>());
    case ValueKind::FLOAT:
        return std::to_string(value<ValueKind::FLOAT>());
    case ValueKind::STRING:
        return std::format("\"{}\"", value<ValueKind::STRING>());
    default:
        break;
    }

    debug::bug("unmapped cv type");
}
