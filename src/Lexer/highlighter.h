/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

namespace via {

// Returns the severity of the highlight
// ERROR will affect the returned `fail` value
enum class OutputSeverity {
    INFO,
    WARNING,
    ERROR_,
};

class Emitter {
public:
    ~Emitter() = default;
    Emitter(ProgramData &program)
        : program(program)
    {
    }

    // Main "entry point" for underlined/highlighted errors
    void out(size_t, std::string, OutputSeverity);

private:
    ProgramData &program;
    // I forgot what this does
    std::vector<std::string> split_lines();
    // Returns a header based on the given severity
    // These headers will vary in terms of color, text, etc.
    std::string get_severity_header(OutputSeverity);
    // Draws a horizontal line under the line line_number of source
    // The dimensions of the line is determined by the offset and length parameters of this function
    // Attaches a message to the top of the underlined line, with the given severity
    std::string underline_line(int, int, int, const std::string &, OutputSeverity);
};

} // namespace via
