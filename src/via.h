/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

#include "Lexer/lexer.h"
#include "Lexer/preproc.h"
#include "Lexer/syntax_analysis.h"

#include "Parser/parser.h"
#include "Parser/print.h"

#include "Compiler/compiler.h"
#include "Compiler/optimizer.h"

#include "VM/api.h"
#include "VM/debug.h"
#include "VM/vlbase.h"
#include "VM/vltable.h"
#include "VM/vlmath.h"
#include "VM/vlrand.h"
#include "VM/vlbit.h"
#include "VM/vlbuffer.h"
#include "VM/vlthread.h"
#include "VM/vlfs.h"
#include "VM/vlfunction.h"
#include "VM/vlhttp.h"
#include "VM/vlstring.h"
#include "VM/vlutf8.h"
#include "VM/vlutf8.h"
#include "VM/vlbit.h"
#include "VM/vlvec3.h"
#include "VM/execute.h"

namespace via
{

// Minimal interpreter implementation for quick usage without touching the API
class Interpreter
{
    using ExecutionResult = std::pair<int, std::string>;

public:
    Interpreter() = default;
    ~Interpreter() = default;

    inline ExecutionResult run(std::string code)
    {
        Tokenization::Tokenizer tokenizer(code);
        viaSourceContainer container = tokenizer.tokenize();

        return compile_and_run(container);
    }

private:
    inline void load_libraries(viaState *V)
    {
        lib::viaL_loadbaselib(V);
        lib::viaL_loadmathlib(V);
        lib::viaL_loadrandlib(V);
        lib::viaL_loadvec3lib(V);
    }

    inline ExecutionResult compile_and_run(viaSourceContainer &container)
    {
        using namespace via::Parsing;
        using namespace via::Compilation;
        using namespace via::Tokenization;

        Parser parser(container);
        AST::AST *ast = parser.parse_program();

        Compiler compiler(ast);
        compiler.add_default_passes();
        compiler.generate();

        std::vector<Instruction> bytecode = compiler.get();

        viaState *V = viaA_newstate(bytecode);
        load_libraries(V);
        via_execute(V);

        ExecutionResult result = {V->exitc, V->exitm};

        viaA_cleanupstate(V);

        return result;
    }
};

} // namespace via