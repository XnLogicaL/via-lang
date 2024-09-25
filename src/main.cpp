#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdlib>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
// #include "generator.hpp"
#include "VM/default/interpreter.hpp"

#include "utils/command.hpp"

const auto VERSION = "0.1.0";

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "error: incorrect usage: no input file specified" << std::endl;
        std::cerr << "  correct usage: via <file.via> <flags...>" << std::endl;

        return EXIT_FAILURE;
    }

    auto command = Command(argc, argv);

    if (command.has_flag("--version") || command.has_flag("-v"))
    {
        std::cout << "via " + std::string(VERSION) << std::endl;
        return EXIT_SUCCESS;
    }

    std::ifstream viaSourceFile(argv[1]);

    std::string via_src;
    std::string via_src_ln;

    int i = 1;

    while (std::getline(viaSourceFile, via_src_ln))
    {
        if (via_src_ln.starts_with("##"))
            continue;

        via_src += via_src_ln + "\n";
        i += 1;
    }

    viaSourceFile.close();

    Lexer lexer(via_src);
    auto tokens = lexer.tokenize();

    if (command.has_flag("--debug"))
        lexer.print_tokens(tokens);

    Parser parser(tokens, argv[1]);
    auto prog_node = parser.parse_prog();
    prog_node->prog_name = std::string(argv[1]);

    if (!prog_node.has_value())
    {
        std::cerr << "via compiler error: failed to parse program\n";
        return EXIT_FAILURE;
    }

    if (command.has_flag("-c"))
    {
        /*Generator generator(prog_node.value());
        generator.generate();

        system("nasm -f elf64 out.asm -o out.o -gdwarf");
        system("ld out.o -o out.out");

        if (command.has_flag("--run"))
            system("./out.out");*/
    }
    else if (command.has_flag("-i"))
    {
        try
        {
            Interpreter interpreter(prog_node.value());
            interpreter.init();
            interpreter.run();
        }
        catch(const std::exception &e)
        {
            std::cout << "via VM error: " << e.what() << std::endl;
            std::cout << "  terminating VM" << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}