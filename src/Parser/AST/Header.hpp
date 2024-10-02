#pragma once

#ifndef via_header_hpp
#define via_header_hpp

#include "../../common.hpp"
#include "../../Lexer/token.hpp"

struct FileHeader;

typedef std::variant<
    FileHeader
> Header;

struct FileHeader
{
    Token ident;
};

#endif