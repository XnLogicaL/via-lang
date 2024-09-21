/* error.hpp */

#pragma once

#include <iostream>
#include <string>

#include "../parser/ast/ProgNode.hpp"

class Error
{
public:
    enum class Severity
    {
        Info,
        Warning,
        Error,
        Fatal
    };

    Error(const std::string& callerFile, ProgNode* prog_node)
        : callerFile_(callerFile)
        , progNode_(prog_node) {}

    void log(Severity sev, int caller_line, std::string msg, int line) const
    {
        std::string prefix;
        switch (sev)
        {
        case Severity::Info:    prefix = "\033[32m[INFO] "; break;   // Green
        case Severity::Warning: prefix = "\033[33m[WARNING] "; break; // Yellow
        case Severity::Error:   prefix = "\033[31m[ERROR] "; break;   // Red
        case Severity::Fatal:   prefix = "\033[35m[FATAL] "; break;   // Magenta
        }

        std::cout << callerFile_ << ":" << std::to_string(caller_line) << ": " << prefix
            << progNode_->prog_name << ":" << std::to_string(line) << ": " << msg
            << "\033[0m" << std::endl;

        if (sev == Severity::Fatal)
            exit(1);
    }

private:
    std::string callerFile_;
    ProgNode* progNode_;
};

