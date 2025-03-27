/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see
 * LICENSE for license information */

#include "via.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size_t) {
    std::string input(reinterpret_cast<const char*>(data), size_t);

    via::ProgramData program("<fuzz>", input);
    via::Tokenizer   tokenizer(program);
    tokenizer.tokenize();

    via::Parser parser(program);
    parser.parse();

    return 0;
}
