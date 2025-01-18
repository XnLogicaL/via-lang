/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "via.h"

namespace via
{

void Interpreter::tokenize()
{
    Tokenizer tokenizer(program);
    tokenizer.tokenize();
}

void Interpreter::preprocess()
{
    Preprocessor preprocessor(program);
    preprocessor.declare_default();
    preprocessor.preprocess();
}

void Interpreter::parse()
{
    Parser parser(program);
    parser.parse_program();
}

void Interpreter::analyze() {}

void Interpreter::compile()
{
    Compiler compiler(program);
    compiler.add_default_passes();
    compiler.generate();
}

void Interpreter::interpret()
{
    via::stloadinstructions(rtstate, *program.bytecode);
    via::execute(rtstate);
    via::pausethread(rtstate);
}

ExecutionResult Interpreter::execute(ProgramData &program_data)
{
    program = program_data;

    try
    {
        tokenize();
        preprocess();
        parse();
        analyze();
        compile();
        interpret();
    }
    catch (VRTException &e)
    {
        return {1, e.what()};
    }

    return {0, ""};
}

} // namespace via