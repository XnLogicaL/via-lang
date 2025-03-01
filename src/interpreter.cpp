// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "interpreter.h"

#define INTP_CHECK_FAILED_INTERNAL(stage) \
    if (failed) \
        std::cerr << "Interpreter: " #stage " returned status code 1 (ERROR)\n";

#define INTP_CHECK_FAILED(failed) \
    if (failed) \
        return 1;

namespace via {

void Interpreter::tokenize()
{
    Tokenizer tokenizer(program);
    tokenizer.tokenize();
}

bool Interpreter::preprocess()
{
    Preprocessor preprocessor(program);
    preprocessor.declare_default();
    bool failed = preprocessor.preprocess();

    INTP_CHECK_FAILED_INTERNAL(Preprocessor);

    return failed;
}

bool Interpreter::parse()
{
    Parser parser(program);
    bool failed = parser.parse_program();

    INTP_CHECK_FAILED_INTERNAL(Parser);

    return failed;
}

bool Interpreter::compile()
{
    Compiler compiler(program);
    bool failed = compiler.generate();

    INTP_CHECK_FAILED_INTERNAL(Compiler);

    return failed;
}

void Interpreter::interpret()
{
    state->load(*program->bytecode);
    via::execute(state);
    via::pause_thread(state);
}

int Interpreter::execute(ProgramData *program_data)
{
    program = program_data;

    // Tokenization is always non-volatile. The tokenizer will never fail.
    tokenize();

    // Volatile stages.
    INTP_CHECK_FAILED(preprocess());
    INTP_CHECK_FAILED(parse());
    INTP_CHECK_FAILED(compile());

    // This is still volatile, however it uses a built-in error handler.
    interpret();

    return impl::__has_error(state);
}

} // namespace via