/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "const_value.h"
#include <format>
#include <optional>
#include "debug.h"
#include "support/conversions.h"

namespace sema = via::sema;

std::optional<sema::ConstValue> sema::ConstValue::from_token(const Token& tok)
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
        if (auto val = stoi<i64>(tok.to_string()))
            return ConstValue(*val);
        break;
    case TokenKind::LIT_FLOAT:
        if (auto val = stof<f64>(tok.to_string()))
            return ConstValue(*val);
        break;
    default:
        break;
    }

    return std::nullopt;
}

std::string sema::ConstValue::to_string() const
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

std::string sema::ConstValue::get_dump() const
{
    return std::format("{}({})", via::to_string(kind()), to_string());
}
