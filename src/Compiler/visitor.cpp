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

VIA_NAMESPACE_END
