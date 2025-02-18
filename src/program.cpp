/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see
 * LICENSE for license information */

#include "common.h"
#include "ast.h"
#include "token.h"
#include "bytecode.h"
#include "types.h"

namespace via {

ProgramData::ProgramData(std::string file_name, std::string file_source)
    : file_name(file_name)
    , source(file_source)
    , tokens(new TokenHolder(0))
    , ast(new AbstractSyntaxTree(0))
    , bytecode(new BytecodeHolder())
    , constants(new kTable())
{
}

ProgramData::~ProgramData()
{
    delete tokens;
    delete ast;
    delete bytecode;
    delete constants;
}

} // namespace via