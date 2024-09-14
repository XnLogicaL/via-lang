#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdlib>

#include "lexer.hpp"
#include "parser.hpp"
#include "generator.hpp"

const auto VERSION = "0.1.0";

class Command
{
public:

    int argc;
    char** argv;

    Command(auto _Ac, auto _Av)
    : argc(_Ac), argv(_Av) {}

    std::vector<std::string> get_flags()
    {
        std::vector<std::string> flags = {};

        for (int i = 0; i < argc; i++)
        {
            auto arg_string = std::string(argv[i]);

            if (arg_string.starts_with("--"))
                flags.push_back(arg_string);
        }

        return flags;
    }

    bool has_flag(std::string flag)
    {
        auto flags = get_flags();
        return std::find(flags.begin(), flags.end(), flag) != flags.end();
    }

};

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "error: incorrect usage: no input file specified" << std::endl;
        std::cerr << "  correct usage: via <...args>" << std::endl;

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

    Parser parser(tokens);
    auto prog_node = parser.parse_prog();
    prog_node->prog_name = std::string(argv[1]);

    if (!prog_node.has_value())
    {
        Console::CompilerError("Failed to parse program");
        return EXIT_FAILURE;
    }

    Generator generator(prog_node.value());
    generator.generate();

    system("nasm -f elf64 out.asm -o out.o");
    system("ld out.o -o out.out");

    if (command.has_flag("--run"))
        system("./out.out");

    return EXIT_SUCCESS;
}