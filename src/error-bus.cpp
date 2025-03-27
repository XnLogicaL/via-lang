// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "error-bus.h"

VIA_NAMESPACE_BEGIN

using enum CompilerErrorLevel;

struct LocalOffset {
    size_t line;
    size_t offset;
};

// ==========================================================================================
// Utility
//
// Returns each line in the source as a vector of strings.
std::vector<std::string> get_lines(const std::string& source) {
    std::istringstream       stream(source);
    std::vector<std::string> lines;
    std::string              line;

    while (std::getline(stream, line)) {
        lines.push_back(line);
    }

    return lines;
}

// Returns a slice (range) of lines from the source given zero-indexed begin and end.
// 'end' is exclusive.
std::vector<std::string> get_lines_in_range(const std::string& source, size_t begin, size_t end) {
    auto lines = get_lines(source);
    if (begin > lines.size())
        begin = lines.size();
    if (end > lines.size())
        end = lines.size();
    return std::vector<std::string>(lines.begin() + begin, lines.begin() + end);
}

// Converts an absolute character offset to a (line, column) pair.
LocalOffset localize_offset(const std::string& source, size_t abs_offset) {
    size_t line_number = 1;
    size_t line_start  = 0;

    for (size_t i = 0; i < source.size() && i < abs_offset; ++i) {
        if (source[i] == '\n') {
            ++line_number;
            line_start = i + 1;
        }
    }

    size_t localized_offset = abs_offset >= line_start ? abs_offset - line_start : 0;
    return {line_number, localized_offset};
}

// Returns a header string (with ANSI colors) for a given error level.
std::string get_error_level_header(const CompilerErrorLevel& lvl) {
    switch (lvl) {
    case INFO:
        return "\033[1;34minfo:\033[0m "; // Blue color for info
    case WARNING:
        return "\033[1;33mwarning:\033[0m "; // Yellow color for warning
    case ERROR_:
        return "\033[1;31merror:\033[0m "; // Red color for error
    default:
        VIA_UNREACHABLE;
    }
}

// ==========================================================================================
// CompilerError
std::string CompilerError::to_string() const {
    std::string header = get_error_level_header(level);

    if (is_flat) {
        return header + message + '\n';
    }

    // Print header with file, line, and column information.
    std::ostringstream oss;
    oss << header
        << std::format("{}:{}:{} - {}\n", ctx.file_path, position.line, position.column, message);

    // Localize absolute offsets into (line, offset) values.
    LocalOffset local_begin = localize_offset(ctx.file_source, position.begin);
    LocalOffset local_end   = localize_offset(ctx.file_source, position.end);

    // Adjust line indices: our get_lines_in_range expects 0-indexed line numbers.
    // Note: local_begin.line and local_end.line are 1-indexed.
    auto lines = get_lines_in_range(ctx.file_source, local_begin.line - 1, local_end.line);

    size_t pos = 0;
    for (const std::string& line : lines) {
        // Calculate the actual line number in the source.
        size_t      current_line_number = local_begin.line + pos;
        std::string line_number_str     = std::to_string(current_line_number);

        std::string underline;
        if (lines.size() == 1) {
            // Single-line error: underline from local_begin.offset to local_end.offset.
            underline = std::string(local_begin.offset, ' ') +
                        std::string(local_end.offset - local_begin.offset, '^') +
                        std::string(
                            line.size() > local_end.offset ? line.size() - local_end.offset : 0, ' '
                        );
        }
        else {
            // Multi-line error.
            if (pos == 0) {
                // First line: underline from local_begin.offset to end of line.
                underline = std::string(local_begin.offset, ' ') +
                            std::string(line.size() - local_begin.offset, '^');
            }
            else if (pos == lines.size() - 1) {
                // Last line: underline from beginning of line up to local_end.offset.
                underline =
                    std::string(local_end.offset, '^') +
                    std::string(
                        line.size() > local_end.offset ? line.size() - local_end.offset : 0, ' '
                    );
            }
            else {
                // Intermediate lines: fully underline the entire line.
                underline = std::string(line.size(), '^');
            }
        }

        oss << std::format("{} | {}\n", line_number_str, line);
        oss << std::format("{} | {}\n", std::string(line_number_str.size(), ' '), underline);

        ++pos;
    }

    return oss.str();
}

// ==========================================================================================
// ErrorBus
void ErrorBus::log(const CompilerError& err) {
    buffer.push_back(err);
}

bool ErrorBus::has_error() {
    for (const CompilerError& err : buffer) {
        if (err.level == ERROR_) {
            return true;
        }
    }

    return false;
}

void ErrorBus::clear() {
    buffer.clear();
}

void ErrorBus::emit() {
    for (const CompilerError& err : buffer) {
        std::cout << err.to_string();
    }

    std::cout << std::flush;

    clear();
}

ErrorBus::~ErrorBus() {
    emit();
}

VIA_NAMESPACE_END
