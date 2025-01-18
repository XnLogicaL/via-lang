/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "repl.h"

namespace via
{

void REPLEngine::tokenize()
{
    stage = "Tokenization";

    Tokenizer tokenizer(program);
    tokenizer.tokenize();
}

void REPLEngine::preprocess()
{
    stage = "Preprocessing";

    Preprocessor preprocessor(program);
    preprocessor.preprocess();
}

void REPLEngine::parse()
{
    stage = "Parsing";

    Parser parser(program);
    parser.parse_program();
}

void REPLEngine::analyze()
{
    stage = "Analysis";
}

void REPLEngine::compile()
{
    stage = "Compilation";

    Compiler compiler(program);
    compiler.add_default_passes();
    compiler.generate();
}

void REPLEngine::interpret()
{
    stage = "Interpreting";

    via::stloadinstructions(rtstate, *program.bytecode);
    via::execute(rtstate);
    via::pausethread(rtstate);
}

void REPLEngine::execute(const std::string &code, bool print_bytecode_flag)
{
    // Setup program data object
    program.file_name = "<repl>";
    program.source = code;

    try
    {
        tokenize();
        preprocess();
        parse();
        analyze();
        compile();

        if (print_bytecode_flag)
            for (Instruction instr : program.bytecode->get())
                std::cout << to_string(instr) << "\n";

        interpret();
    }
    catch (VRTException &e)
    {
        std::cout << std::format("REPL Failed during stage {}\n  ", stage) << e.what();
    }
}

} // namespace via