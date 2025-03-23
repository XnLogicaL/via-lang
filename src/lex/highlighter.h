// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_HIGHLIGHTER_H
#define _VIA_HIGHLIGHTER_H

#include "common.h"

VIA_NAMESPACE_BEGIN

enum class OutputSeverity {
    Info,
    Warning,
    Error,
};

class ErrorEmitter final {
public:
    ErrorEmitter(ProgramData& program)
        : program(program) {}

    void out(Token, std::string message, OutputSeverity severity);
    void out_range(size_t begin, size_t end, std::string message, OutputSeverity severity);
    void out_flat(std::string message, OutputSeverity severity);

private:
    ProgramData& program;

private:
    std::vector<std::string> split_lines();

    std::string get_severity_header(OutputSeverity);
    std::string underline_line(int, int, int, const std::string&, OutputSeverity);
    std::string underline_range(int, int, const std::string&, OutputSeverity);
};

VIA_NAMESPACE_END

#endif
