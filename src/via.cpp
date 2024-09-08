#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "lexer.hpp"
#include "parser.hpp"

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
        std::cerr << "incorrect usage: no input file specified" << std::endl;
        std::cerr << "  correct usage: via <file.via> <out>" << std::endl;

        return EXIT_FAILURE;
    }

    auto command = Command(argc, argv);

    std::ifstream viaSourceFile(argv[1]);

    std::string via_src;
    std::string via_src_ln;

    while (std::getline(viaSourceFile, via_src_ln))
    {
        if (via_src_ln.starts_with("##"))
            continue;

        via_src += via_src_ln;
    }

    viaSourceFile.close();

    std::cout << command.has_flag("--flag");
    
    /*
    Lexer lexer(via_src);
    auto tokens = lexer.tokenize();

    for (const auto &tok : tokens)
        std::cout << tok << std::endl;

    std::cout << "\n";

    Parser parser(tokens);
    auto global = parser.parse_scope("__global");

    std::cout << Debug::get_scope_string(global.value()) << std::endl;
    std::cout << "Successfuly compiled " << argv[1] << std::endl;
    */

    return EXIT_SUCCESS;
}