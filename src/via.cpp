#include <string>
#include <vector>
#include <iostream>
#include <print>

#include "lexer.hpp"
#include "parser.hpp"
#include "variable.hpp"
#include "scope.hpp"

int main()
{
    std::string code = "local z = 0.1 function x(a: w, b: y) { return a + b }";
    Lexer lexer(code);

    std::vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    parser.parse();

    Scope _global = Scope(
        std::string("__global_scope"),
        ScopeContents{}
    );

    std::cout << _global << std::endl;

    return 0;
}