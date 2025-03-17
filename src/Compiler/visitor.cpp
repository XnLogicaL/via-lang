// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "visitor.h"

VIA_NAMESPACE_BEGIN

TValue construct_constant(LiteralNode& literal_node) {
    using enum ValueType;
    return std::visit(
        [](auto&& val) -> TValue {
            using T = std::decay_t<decltype(val)>;

            if constexpr (std::is_same_v<T, int>) {
                return TValue(val);
            }
            else if constexpr (std::is_same_v<T, bool>) {
                return TValue(val);
            }
            else if constexpr (std::is_same_v<T, float>) {
                return TValue(val);
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                TString* tstring = new TString(nullptr, val.data());
                return TValue(string, static_cast<void*>(tstring));
            }

            VIA_UNREACHABLE;
        },
        literal_node.value
    );
}

void NodeVisitor::compiler_error(size_t begin, size_t end, const std::string& message) {
    visitor_failed = true;
    emitter.out_range(begin, end, message, OutputSeverity::Error);
}

void NodeVisitor::compiler_error(const Token& token, const std::string& message) {
    visitor_failed = true;
    emitter.out(token, message, OutputSeverity::Error);
}

void NodeVisitor::compiler_error(const std::string& message) {
    visitor_failed = true;
    emitter.out_flat(message, OutputSeverity::Error);
}

void NodeVisitor::compiler_warning(size_t begin, size_t end, const std::string& message) {
    emitter.out_range(begin, end, message, OutputSeverity::Warning);
}

void NodeVisitor::compiler_warning(const Token& token, const std::string& message) {
    emitter.out(token, message, OutputSeverity::Warning);
}

void NodeVisitor::compiler_warning(const std::string& message) {
    emitter.out_flat(message, OutputSeverity::Warning);
}

void NodeVisitor::compiler_info(size_t begin, size_t end, const std::string& message) {
    emitter.out_range(begin, end, message, OutputSeverity::Info);
}

void NodeVisitor::compiler_info(const Token& token, const std::string& message) {
    emitter.out(token, message, OutputSeverity::Info);
}

void NodeVisitor::compiler_info(const std::string& message) {
    emitter.out_flat(message, OutputSeverity::Info);
}

VIA_NAMESPACE_END
