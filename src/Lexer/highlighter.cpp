// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "highlighter.h"
#include "token.h"

#define ILLFORMED_ERROR \
    "\n  (This is an illformed error, likely caused by an internal compiler bug.\n   If this " \
    "error persists, please " \
    "create an issue at https://github.com/XnLogicaL/via-lang)"

namespace via {

// Splits a source string into lines
std::vector<std::string> Emitter::split_lines()
{
    std::vector<std::string> lines;
    std::istringstream       stream(program.source);
    std::string              line;

    // Loop through lines and split them
    while (std::getline(stream, line))
        lines.push_back(line);

    return lines;
}

// Returns a "title" or "header" for output messages based on severity
std::string Emitter::get_severity_header(OutputSeverity sev)
{
    switch (sev) {
    case OutputSeverity::Info:
        return "\033[1;34minfo:\033[0m "; // Blue color for info
    case OutputSeverity::Warning:
        return "\033[1;33mwarning:\033[0m "; // Yellow color for warning
    case OutputSeverity::Error:
        return "\033[1;31merror:\033[0m "; // Red color for error
    default:
        VIA_UNREACHABLE;
        return "";
    }
}

// Function to underline a portion of a line with a cursor (^) at the offset
std::string Emitter::underline_line(
    int                line_number,
    int                offset,
    int                length,
    const std::string &message,
    OutputSeverity     sev
)
{
    std::vector<std::string> lines = split_lines();

    if (line_number < 1 || line_number > static_cast<int>(lines.size())) {
        return std::format("{} {}", get_severity_header(sev), message);
    }

    // Lines are 1-based, vector is 0-based
    const std::string &line = lines[line_number - 1];
    std::string        underline;

    if (offset < 0 || offset >= static_cast<int>(line.size())) {
        return std::format("{} {}", get_severity_header(sev), message);
    }

    underline = std::string(offset, ' ') + std::string(length, '~');

    if (offset + length > static_cast<int>(line.size())) {
        underline = underline.substr(0, line.size() - offset);
    }

    if (offset < static_cast<int>(underline.size())) {
        underline[offset] = '^';
    }

    // Leetcode mfers will absolutely HATE this
    // "oH iTs NoT O(1) bro" (shut the fuck up, go make your own language buddy)
    std::string line_number_str   = std::to_string(line_number);
    int         line_number_width = line_number_str.length();

    return get_severity_header(sev) + message + "\n" + line_number_str + " | " + line + "\n" +
           std::string(line_number_width, ' ') + " | " + underline;
}

// Emits an output message
void Emitter::out(U64 idx, std::string message, OutputSeverity sev)
{
    VIA_ASSERT(program != nullptr, "Emitter::out called with invalid program pointer");

    // This is an internal "flag" that determines if the file name has been displayed before any
    // errors
    static bool has_printed_file_name = false;

    // Check if file information has been printed
    if (!has_printed_file_name && program.file != "<repl>") {
        has_printed_file_name = true;
        std::cout << std::format("In file {}:\n", program.file);
    }

    // Find token
    Token tok    = program.tokens->tokens.at(idx);
    SIZE  line   = tok.line;
    SIZE  offset = tok.offset;
    SIZE  length = tok.lexeme.length();
    std::cout << underline_line(line, offset, length, message, sev) << "\n";
}

void Emitter::out_flat(std::string message, OutputSeverity sev)
{
    std::cout << get_severity_header(sev) << message << "\n";
}

} // namespace via
