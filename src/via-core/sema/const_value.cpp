/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "const_value.h"
#include <magic_enum/magic_enum.hpp>
#include "support/conversions.h"

namespace sema = via::sema;

via::Option<sema::ConstValue> sema::ConstValue::from_token(const Token& tok)
{
    switch (tok.kind) {
        case Token::Kind::LIT_NIL:
            return ConstValue();
        case Token::Kind::LIT_TRUE:
            return ConstValue(true);
        case Token::Kind::LIT_FALSE:
            return ConstValue(false);
        case Token::Kind::LIT_INT:
        case Token::Kind::LIT_XINT:
        case Token::Kind::LIT_BINT:
            if (auto val = stoi<i64>(tok.to_string()))
                return ConstValue(*val);
            break;
        case Token::Kind::LIT_FLOAT:
            if (auto val = stof<f64>(tok.to_string()))
                return ConstValue(*val);
            break;
        default:
            break;
    }

    return nullopt;
}

std::string sema::ConstValue::to_string() const
{
    using enum Kind;

    switch (kind()) {
        case NIL:
            return "nil";
        case BOOL:
            return value<BOOL>() ? "true" : "false";
        case INT:
            return std::to_string(value<INT>());
        case FLOAT:
            return std::to_string(value<FLOAT>());
        case STRING:
            return std::format("\"{}\"", value<STRING>());
        default:
            break;
    }

    debug::bug("unmapped cv type");
}

std::string sema::ConstValue::get_dump() const
{
    return std::format("[{} {}]", magic_enum::enum_name(kind()), to_string());
}
