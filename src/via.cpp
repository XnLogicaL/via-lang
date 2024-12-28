/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "via.h"

namespace via
{

ExecutionResult Interpreter::run(std::string code)
{
    Tokenization::Tokenizer tokenizer(code);
    SrcContainer container = tokenizer.tokenize();

    Tokenization::Preprocessor preprocessor(container);
    bool failed = preprocessor.preprocess();

    VIA_ASSERT_SILENT(!failed, "Preprocessor fail");

    return compile_and_run(container);
}

void Interpreter::load_libraries(RTState *V)
{
    lib::loadbaselib(V);
    lib::loadmathlib(V);
    lib::loadrandlib(V);
    lib::loadvec3lib(V);
}

ExecutionResult Interpreter::compile_and_run(SrcContainer &container)
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

    RTState *V = stnewstate(bytecode);
    load_libraries(V);
    execute(V);

    ExecutionResult result = {V->exitc, V->exitm};

    stcleanupstate(V);

    return result;
}

} // namespace via