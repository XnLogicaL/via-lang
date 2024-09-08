#pragma once

#include <string>
#include <fstream>
#include <vector>

#include "parser.hpp"

class Generator
{
public:
    ScopeType global_scope;
    std::string write_file;

    Generator(ScopeType global_scope, std::string write_file = "out.asm")
    : global_scope(global_scope), write_file(write_file) {};

    std::ofstream create_output_file(std::string out_src)
    {
        std::ofstream out_file(write_file);
        out_file << out_src;
        out_file.close();
    }

    void generate()
    {
        std::string src;

        // TODO: Generate asm code from parsed tokens!!!!!

        create_output_file(src);
    }
};