/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "highlighter.h"
#include <cmath>

namespace via::Tokenization
{

// This is an internal "flag" that determines if the file name has been displayed before any errors
static bool has_printed_file_name = false;

// Splits a source string into lines
std::vector<std::string> Emitter::split_lines(const std::string &source)
{
    std::vector<std::string> lines;
    std::istringstream stream(source);
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
std::string Emitter::underline_line(const std::string &source, int line_number, int offset, int length, const std::string &message, Severity sev)
{
    std::vector<std::string> lines = split_lines(source);

    // Check if line number is valid
    if (line_number < 1 || line_number > static_cast<int>(lines.size()))
        return "<ERROR-INVALID-LINE>";

    const std::string &line = lines[line_number - 1]; // Lines are 1-based, vector is 0-based
    std::string underline;

    // Check if offset is valid
    if (offset < 0 || offset >= static_cast<int>(line.size()))
        return "<ERROR-INVALID-OFFSET>";

    // Create the underline with tildes and insert the cursor (^)
    underline = std::string(offset, ' ') + std::string(length, '~');

    // Ensure we don't go past the line's length
    if (offset + length > static_cast<int>(line.size()))
        underline = underline.substr(0, line.size() - offset);

    // Place the cursor (^) at the exact offset
    if (offset < static_cast<int>(underline.size()))
        underline[offset] = '^';

    // Calculate the width of the line number field
    int line_number_width = static_cast<int>(std::ceil(std::log10(line_number)));
    std::string line_number_str = std::to_string(line_number);

    // Adjust alignment by dynamically adding spaces to the line number section
    return get_severity_header(sev) + message + "\n" + line_number_str + " | " + std::string(line_number_width, ' ') + line + "\n" +
           std::string(line_number_width, ' ') + " |  " + underline;
}

// Emits an output message
void Emitter::out(viaSourceContainer vsc, size_t idx, std::string message, Severity sev)
{
    // Check if file information has been printed
    if (!has_printed_file_name)
    {
        has_printed_file_name = true;
        std::cout << std::format("In file {}:\n", vsc.file_name);
    }

    // Find token
    Token tok = vsc.tokens.at(idx);
    size_t line = tok.line;
    size_t offset = tok.offset;
    size_t length = tok.value.length();
    std::cout << underline_line(vsc.source, line, offset, length, message, sev) << "\n";
}

} // namespace via::Tokenization
