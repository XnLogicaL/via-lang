/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "via.h"

namespace via
{

void Interpreter::tokenize()
{
    Tokenizer tokenizer(program);
    tokenizer.tokenize();
}

void Interpreter::analyze_syntax()
{
    SyntaxAnalyzer analyzer(program);
    analyzer.analyze();
}

void Interpreter::parse()
{
    Parser parser(program);
}

} // namespace via