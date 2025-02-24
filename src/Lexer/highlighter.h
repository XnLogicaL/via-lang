// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"

namespace via {

// Returns the severity of the highlight
// ERROR will affect the returned `fail` value
enum class OutputSeverity {
    Info,
    Warning,
    Error,
};

class Emitter {
public:
    Emitter(ProgramData *program)
        : program(program)
    {
    }

    void out(size_t, std::string, OutputSeverity);

private:
    ProgramData *program;

private:
    std::vector<std::string> split_lines();
    std::string get_severity_header(OutputSeverity);
    std::string underline_line(int, int, int, const std::string &, OutputSeverity);
};

} // namespace via
