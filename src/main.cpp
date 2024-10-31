/*

    via

    A convenient multiparadigm scripting language.

    Author(s): @XnLogicaL, @KasenDaniels
    License: MIT License

*/

#include "common.h"

#include "Utils/reader.hpp"

#include "Lexer/lexer.h"
#include "Lexer/analysis/syntax.h"
#include "Lexer/analysis/semantic.h"

#include "Parser/parser.h"
#include "Parser/print.h"

/*
Define custom allocation sizes using these macros:

#define __VIA_LEXER_ALLOC_SIZE
#define __VIA_PARSER_ALLOC_SIZE

Define before the main() function!
*/

#define ERROR(message) \
    { \
        std::cerr << message << std::endl; \
        std::cerr << "Compilation aborted\n"; \
        exit(1); \
    } \

#define RUN_CLEANUP() \
    delete tokenizer; \
    delete syntax_analyzer; \
    delete parser; \

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        ERROR("Incorrect usage.\n  Correct usage: via <file> <flags>");
    }

    std::string code;

    try {
        code = reader::read_file(argv[1]);
    } catch(const reader::BadFileException& e) {
        ERROR(std::format("Failed to read file '{}'\n  No such file or directory", e.file_path));
    }

    auto tokenizer = new via::Tokenization::Tokenizer(code);
    auto vsc = tokenizer->tokenize();
    vsc.file_name = std::string(argv[1]);

    for (const auto &tok : vsc.tokens)
    {
        std::cout << tok.to_string() << std::endl;
    }

    auto syntax_analyzer = new via::Tokenization::SyntaxAnalysis::SyntaxAnalyzer(vsc);
    auto syntax_fail = syntax_analyzer->analyze();

    if (syntax_fail)
    {
        ERROR("Syntax analysis failed");
    }

    auto parser = new via::Parsing::Parser(vsc.tokens);
    auto ast = parser->parse_prog();

    via::Parsing::AST::print_ast(ast);

    RUN_CLEANUP();

    return 0;
}
