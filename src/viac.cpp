/*

    via

    A convenient multiparadigm scripting language.

    Author(s): @XnLogicaL, @KasenDaniels
    License: MIT License

*/

#include "common.h"
#include "core.h"

#include "Utils/reader.hpp"

#include "flags.hpp"

#include "Lexer/lexer.h"
#include "Lexer/analysis/syntax.h"
#include "Lexer/analysis/semantic.h"

#include "Parser/parser.h"
#include "Parser/print.h"

#define __VIA_VER "0.2.1"

/*
Define custom allocation sizes using these macros:

#define __VIA_LEXER_ALLOC_SIZE
#define __VIA_PARSER_ALLOC_SIZE

Define before the main() function!
*/

int main(int argc, char* argv[])
{

// DO NOT remove
// This program relies on GCC 14.x features and is not compatible with MSVC
#ifdef _MSC_VER
    VIA_ASSERT(false, "MSVC is not supported, use GCC/Clang instead.");
#endif

    VIA_ASSERT(argc < 2, "Incorrect usage.\n  Correct usage: via <file> <flags>");

    auto flag = flags::flags(argc, argv);
    std::string code;

    try {
        code = reader::read_file(argv[1]);
    } catch(const reader::BadFileException& e) {
        VIA_ASSERT(false, std::format("Failed to read file '{}'\n  No such file or directory", e.file_path).c_str());
    }

    auto tokenizer = new via::Tokenization::Tokenizer(code);
    auto vsc = tokenizer->tokenize();
    vsc.file_name = std::string(argv[1]);

    auto syntax_analyzer = new via::Tokenization::SyntaxAnalysis::SyntaxAnalyzer(vsc);
    auto syntax_fail = syntax_analyzer->analyze();

    VIA_ASSERT(!syntax_fail, "Syntax analysis failed");

    auto parser = new via::Parsing::Parser(vsc);
    auto ast = parser->parse_program();

    std::cout << via::Parsing::AST::stringify_ast(*ast) << "\n";
    
    delete tokenizer;
    delete syntax_analyzer;
    delete parser;

    return 0;

#undef ERROR
}
