/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "common.h"
#include "core.h"

#include "Utils/reader.h"

#include "flags.hpp"

#include "Lexer/analysis/syntax.h"
#include "Lexer/lexer.h"
#include "Lexer/preproc/preproc.h"

#include "Parser/parser.h"
#include "Compiler/compiler.h"

#include "VM/api.h"
#include "VM/execute.h"
#include "VM/vlbase.h"
#include "VM/vlmath.h"
#include "VM/vlvec3.h"

#define __VIA_VER "0.2.1"

/*
Define custom allocation sizes using these macros:

#define __VIA_LEXER_ALLOC_SIZE
#define __VIA_PARSER_ALLOC_SIZE

Define before the main() function!
*/

int main(int argc, char *argv[])
{
    using namespace via;
    using namespace Tokenization;
    using namespace Parsing;
    using namespace Preprocessing;
    using namespace Compilation;
    using namespace FileReader;

// DO NOT remove
// This program relies on GCC 14.x features and is not compatible with MSVC
#ifdef _MSC_VER
    VIA_ASSERT(false, "MSVC is not supported, compile using GCC/Clang instead!");
#endif

    VIA_ASSERT(argc < 2, "Incorrect usage.\n  Correct usage: via <file> <flags>");

    flags::flags *flag = new flags::flags(argc, argv);
    std::string code;

    try
    {
        code = read_file(argv[1]);
    }
    catch (const BadFileException &e)
    {
        VIA_ASSERT(false, std::format("Failed to read file '{}'\n  No such file or directory", e.file_path).c_str());
    }

    Tokenizer tokenizer(code);
    viaSourceContainer vsc = tokenizer.tokenize();
    vsc.file_name = std::string(argv[1]);

    Preprocessor preproc(&vsc.tokens);
    preproc.preprocess();

    SyntaxAnalyzer syntax_analyzer(vsc);
    bool syntax_fail = syntax_analyzer.analyze();

    VIA_ASSERT(!syntax_fail, "Syntax analysis failed");

    Parser parser(vsc);
    AST::AST *ast = parser.parse_program();

    Compiler compiler(ast);
    auto instrs = compiler.compile();

    viaState *V = viaA_newstate(instrs);
    via_execute(V);

    return 0;
}
