// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "highlighter.h"
#include "token.h"

VIA_NAMESPACE_BEGIN

static bool has_printed_file_name = false;

// Splits a source string into lines
std::vector<std::string> ErrorEmitter::split_lines() {
    std::vector<std::string> lines;
    std::istringstream       stream(program.source);
    std::string              line;

    // Loop through lines and split them
    while (std::getline(stream, line))
        lines.push_back(line);

    return lines;
}

// Returns a "title" or "header" for output messages based on severity
std::string ErrorEmitter::get_severity_header(OutputSeverity sev) {
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
std::string ErrorEmitter::underline_line(
    int line_number, int offset, int length, const std::string& message, OutputSeverity sev
) {
    std::vector<std::string> lines = split_lines();

    if (line_number < 1 || line_number > static_cast<int>(lines.size())) {
        return std::format("{} {}", get_severity_header(sev), message);
    }

    // Lines are 1-based, vector is 0-based
    const std::string& line = lines[line_number - 1];
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

std::string ErrorEmitter::underline_range(
    int begin_pos, int end_pos, const std::string& message, OutputSeverity sev
) {
    // If the range is invalid, just return a basic message.
    if (begin_pos < 0 || end_pos < 0 || begin_pos >= end_pos) {
        return std::format("{} {}", get_severity_header(sev), message);
    }

    // Use the entire source.
    const std::string& source = program.source;

    // Find the start of the line containing begin_pos.
    size_t line_start = source.rfind('\n', begin_pos);
    if (line_start == std::string::npos)
        line_start = 0;
    else
        line_start += 1; // move past the newline

    // Find the end of the line.
    size_t line_end = source.find('\n', begin_pos);
    if (line_end == std::string::npos)
        line_end = source.size();

    // Extract the specific line.
    std::string line = source.substr(line_start, line_end - line_start);

    // Compute the column offsets relative to the start of the line.
    int column_begin = begin_pos - static_cast<int>(line_start);
    int column_end   = end_pos - static_cast<int>(line_start);
    if (column_end > static_cast<int>(line.size()))
        column_end = static_cast<int>(line.size());

    // Build an underline for only the error span.
    // First, create a string of spaces matching the line.
    std::string underline(line.size(), ' ');
    // Fill in the error region with tildes.
    for (int i = column_begin; i < column_end; ++i) {
        underline[i] = '~';
    }
    // Place a caret at the beginning of the error span.
    if (column_begin < column_end && column_begin < static_cast<int>(underline.size()))
        underline[column_begin] = '^';

    // Calculate the line number by counting '\n' before begin_pos.
    int line_number = 1;
    for (size_t i = 0; i < static_cast<size_t>(begin_pos); ++i) {
        if (source[i] == '\n')
            ++line_number;
    }
    std::string line_number_str   = std::to_string(line_number);
    size_t      line_number_width = line_number_str.length();

    // Assemble the final message.
    std::string result;
    result += get_severity_header(sev) + message + "\n";
    result += line_number_str + " | " + line + "\n";
    result += std::string(line_number_width, ' ') + " | " + underline;

    return result;
}

// Emits an output message
void ErrorEmitter::out(Token tok, std::string message, OutputSeverity sev) {
    // Check if file information has been printed
    if (!has_printed_file_name && program.file != "<repl>") {
        has_printed_file_name = true;
        std::cout << std::format("In file {}:\n", program.file);
    }

    size_t line   = tok.line;
    size_t offset = tok.offset;
    size_t length = tok.lexeme.length();
    std::cout << underline_line(line, offset, length, message, sev) << "\n";
}

void ErrorEmitter::out_range(size_t begin, size_t end, std::string message, OutputSeverity sev) {
    // Check if file information has been printed
    if (!has_printed_file_name && program.file != "<repl>") {
        has_printed_file_name = true;
        std::cout << std::format("In file {}:\n", program.file);
    }

    std::cout << underline_range(begin, end, message, sev) << "\n";
}

void ErrorEmitter::out_flat(std::string message, OutputSeverity sev) {
    std::cout << get_severity_header(sev) << message << "\n";
}

VIA_NAMESPACE_END
