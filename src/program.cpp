// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "common.h"
#include "ast.h"
#include "token.h"
#include "bytecode.h"
#include "constant.h"
#include "types.h"
#include "stack.h"

#define DELETE_IF(target) \
    if (target) { \
        delete target; \
    }

#ifndef VIA_PARSER_ALLOC_SIZE
    #define VIA_PARSER_ALLOC_SIZE 8 * 1024 * 1024 // 8MB
#endif

namespace via {

ProgramData::ProgramData(std::string file, std::string file_source)
    : file(file)
    , source(file_source)
    , tokens(new TokenHolder())
    , ast(new AbstractSyntaxTree())
    , bytecode(new BytecodeHolder(this))
    , constants(new ConstantHolder())
    , test_stack(new TestStack())
    , globals(new GlobalTracker())
{
}

ProgramData::~ProgramData()
{
    DELETE_IF(tokens);
    DELETE_IF(ast);
    DELETE_IF(bytecode);
    DELETE_IF(constants);
    DELETE_IF(test_stack);
    DELETE_IF(globals)
}

} // namespace via