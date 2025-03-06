/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see
 * LICENSE for license information */

#include "via.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, SIZE size)
{
    std::string input(reinterpret_cast<const char*>(data));

    via::ProgramData program("<fuzz>", input);
    via::Tokenizer   tokenizer(program);
    tokenizer.tokenize();

    via::Parser parser(program);
    parser.parse();

    return 0;
}
