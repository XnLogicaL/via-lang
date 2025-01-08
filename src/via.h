/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

#include "Lexer/lexer.h"
#include "Lexer/preproc.h"
#include "Lexer/syntax_analysis.h"
#include "Lexer/cache.h"
#include "Lexer/encoder.h"

#include "Parser/parser.h"
#include "Parser/ast.h"

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
#include "VM/execute.h"

namespace via
{

using ExecutionResult = std::pair<int, std::string>;

// Minimal interpreter implementation for quick usage without touching the API
class Interpreter
{
public:
    ~Interpreter() = default;
    ExecutionResult run(std::string);

public:
    ProgramData &program;

private:
    void tokenize();
    void preprocess();
    void analyze_syntax();
    void parse();
    void analyze_semantics();
    void compile();
    void interpret();
};

} // namespace via