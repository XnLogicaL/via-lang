/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "via.h"

namespace via
{

ExecutionResult Interpreter::run(std::string code)
{
    Tokenization::Tokenizer tokenizer(code);
    viaSourceContainer container = tokenizer.tokenize();

    Tokenization::Preprocessor preprocessor(container);
    bool failed = preprocessor.preprocess();

    VIA_ASSERT_SILENT(!failed, "Preprocessor fail");

    return compile_and_run(container);
}

void Interpreter::load_libraries(viaState *V)
{
    lib::viaL_loadbaselib(V);
    lib::viaL_loadmathlib(V);
    lib::viaL_loadrandlib(V);
    lib::viaL_loadvec3lib(V);
}

ExecutionResult Interpreter::compile_and_run(viaSourceContainer &container)
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

} // namespace via