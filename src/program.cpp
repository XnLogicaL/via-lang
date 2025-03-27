// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "common.h"
#include "ast.h"
#include "token.h"
#include "bytecode.h"
#include "constant.h"
#include "stack.h"

#define DELETE_IF(target)                                                                          \
    if (target) {                                                                                  \
        delete target;                                                                             \
        target = nullptr;                                                                          \
    }

VIA_NAMESPACE_BEGIN

ProgramData::ProgramData(std::string file, std::string file_source)
    : file(file),
      source(file_source),
      token_stream(new TokenStream()),
      ast(new AbstractSyntaxTree()),
      bytecode(new BytecodeHolder()),
      constants(new ConstantHolder()),
      test_stack(new CompilerStack()),
      globals(new GlobalTracker()) {
    globals->declare_builtins();
}

ProgramData::~ProgramData() {
    DELETE_IF(token_stream);
    DELETE_IF(ast);
    DELETE_IF(bytecode);
    DELETE_IF(constants);
    DELETE_IF(test_stack);
    DELETE_IF(globals);
}

VIA_NAMESPACE_END
