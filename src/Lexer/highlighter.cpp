/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "highlighter.h"
#include <cmath>

namespace via::Tokenization
{

// This is an internal "flag" that determines if the file name has been displayed before any errors
static bool has_printed_file_name = false;

// Splits a source string into lines
std::vector<std::string> Emitter::split_lines()
{
    std::vector<std::string> lines;
    std::istringstream stream(container.source);
    std::string line;

    // Loop through lines and split them
    while (std::getline(stream, line))
        lines.push_back(line);

    return lines;
}

// Returns a "title" or "header" for output messages based on severity
std::string Emitter::get_severity_header(Severity sev)
{
    switch (sev)
    {
    case Severity::INFO:
        return "\033[1;34minfo:\033[0m "; // Blue color for info
    case Severity::WARNING:
        return "\033[1;33mwarning:\033[0m "; // Yellow color for warning
    case Severity::ERROR:
        return "\033[1;31merror:\033[0m "; // Red color for error
    default:
        UNREACHABLE();
        return "";
    }
}

// Function to underline a portion of a line with a cursor (^) at the offset
std::string Emitter::underline_line(int line_number, int offset, int length, const std::string &message, Severity sev)
{
    std::vector<std::string> lines = split_lines();

    if (line_number < 1 || line_number > static_cast<int>(lines.size()))
        return "<error-invalid-line>";

    // Lines are 1-based, vector is 0-based
    const std::string &line = lines[line_number - 1];
    std::string underline;

    if (offset < 0 || offset >= static_cast<int>(line.size()))
        return "<error-invalid-line>";

    underline = std::string(offset, ' ') + std::string(length, '~');

    if (offset + length > static_cast<int>(line.size()))
        underline = underline.substr(0, line.size() - offset);

    if (offset < static_cast<int>(underline.size()))
        underline[offset] = '^';

    std::string line_number_str = std::to_string(line_number);
    // Leetcode mfers will absolutely HATE this
    // "oH iTs NoT O(1) bro" (shut the fuck up, go make your own language buddy)
    int line_number_width = line_number_str.length();

    return get_severity_header(sev) + message + "\n" + line_number_str + " | " + line + "\n" + std::string(line_number_width, ' ') + " | " +
           underline;
}

// Emits an output message
void Emitter::out(size_t idx, std::string message, Severity sev)
{
    // Check if file information has been printed
    if (!has_printed_file_name && container.file_name != "<repl>")
    {
        has_printed_file_name = true;
        std::cout << std::format("In file {}:\n", container.file_name);
    }

    // Find token
    Token tok = container.tokens.at(idx);
    size_t line = tok.line;
    size_t offset = tok.offset;
    size_t length = tok.value.length();
    std::cout << underline_line(line, offset, length, message, sev) << "\n";
}

} // namespace via::Tokenization
