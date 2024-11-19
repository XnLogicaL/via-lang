/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include <string>
#include <vector>

#include "common.h"
#include "container.h"

namespace via::Tokenization
{

// Returns the severity of the highlight
// ERROR will affect the returned `fail` value
enum class Severity
{
    INFO,
    WARNING,
    ERROR
};

// I forgot what this does
std::vector<std::string> split_lines(const std::string& source);
// Returns a header based on the given severity
// These headers will vary in terms of color, text, etc.
std::string get_severity_header(Severity sev);
// Draws a horizontal line under the line line_number of source
// The dimensions of the line is determined by the offset and length parameters of this function
// Attaches a message to the top of the underlined line, with the given severity
std::string underline_line(const std::string& source, int line_number, int offset, int length, const std::string& message, Severity sev);
// Main "entry point" for underlined/highlighted errors
void token_error(viaSourceContainer vsc, size_t idx, std::string message, Severity sev);

} // namespace via::Tokenization
