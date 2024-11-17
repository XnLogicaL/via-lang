/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath> // For log10 and pow

#include "common.h"
#include "container.h"

namespace via::Tokenization::SourceLineHighlighter
{

enum class Severity
{
    INFO,
    WARNING,
    ERROR
};

std::vector<std::string> split_lines(const std::string& source);
std::string get_severity_header(Severity sev);
std::string underline_line(const std::string& source, int line_number, int offset, int length, const std::string& message, Severity sev);
void token_error(viaSourceContainer vsc, size_t idx, std::string message, Severity sev);

} // namespace via::Tokenization::SourceLineHighlighter
