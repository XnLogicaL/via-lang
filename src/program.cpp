/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see
 * LICENSE for license information */

#include "common.h"
#include "ast.h"
#include "token.h"
#include "bytecode.h"
#include "types.h"

#define DELETE_IF(target) \
    if (target) { \
        delete target; \
    }

#ifndef VIA_PARSER_ALLOC_SIZE
    #define VIA_PARSER_ALLOC_SIZE 8 * 1024 * 1024 // 8MB
#endif

namespace via {

ProgramData::ProgramData(std::string file_name, std::string file_source)
    : file_name(file_name)
    , source(file_source)
    , tokens(new TokenHolder())
    , ast(new AbstractSyntaxTree(VIA_PARSER_ALLOC_SIZE))
    , bytecode(new BytecodeHolder())
    , constants(new kTable())
{
}

ProgramData::~ProgramData()
{
    DELETE_IF(tokens);
    DELETE_IF(ast);
    DELETE_IF(bytecode);
    DELETE_IF(constants);
}

} // namespace via