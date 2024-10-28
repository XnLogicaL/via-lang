/*

    via

    A convenient multiparadigm scripting language.

    Author(s): @XnLogicaL, @KasenDaniels
    License: MIT License

*/

#include <fstream>
#include <iostream>
#include <string>
#include "Lexer/lexer.h"
#include "Lexer/analysis/syntax.h"
#include "Lexer/util/highlighter.hpp"

using namespace via::Tokenization;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    // Open the file
    std::ifstream file(argv[1]);
    if (!file)
    {
        std::cerr << "Error opening file: " << argv[1] << "\n";
        return 1;
    }

    // Read file contents
    std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Tokenize and analyze the code
    auto vsc = tokenize_code(code);
    vsc.file_name = "file.via";

    auto fail = SyntaxAnalysis::analyze(vsc);

    if (fail)
    {
        std::cout << "Compilation aborted\n";
        return 1;
    }

    return 0;
}
