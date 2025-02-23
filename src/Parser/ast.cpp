// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#include "ast.h"
#include "visitor.h"
#include "format_vec.h"

namespace via {

// =============================== |
//          LiteralNode            |
// =============================== |

std::string LiteralNode::to_string()
{
    if (int *integer_value = std::get_if<int>(&value)) {
        return std::format("Literal<{}>", *integer_value);
    }
    else if (float *floating_point_value = std::get_if<float>(&value)) {
        return std::format("Literal<{}>", *floating_point_value);
    }
    else if (bool *boolean_value = std::get_if<bool>(&value)) {
        return std::format("Literal<{}>", *boolean_value ? "true" : "false");
    }
    else if (std::string *string_value = std::get_if<std::string>(&value)) {
        return std::format("Literal<'{}'>", *string_value);
    }
    else {
        return "Literal<nil>";
    }
}

void LiteralNode::accept(NodeVisitor &visitor, U32 dst)
{
    visitor.visit(*this, dst);
}

// =============================== |
//          VariableNode           |
// =============================== |

std::string VariableNode::to_string()
{
    return std::format("Variable<'{}'>", identifier.lexeme);
}

void VariableNode::accept(NodeVisitor &visitor, U32 dst)
{
    visitor.visit(*this, dst);
}

// =============================== |
//            UnaryNode            |
// =============================== |

std::string UnaryNode::to_string()
{
    return std::format("Unary<{}>", expression->to_string());
}

void UnaryNode::accept(NodeVisitor &visitor, U32 dst)
{
    visitor.visit(*this, dst);
}

// =============================== |
//            GroupNode            |
// =============================== |

std::string GroupNode::to_string()
{
    return std::format("Group<{}>", expression->to_string());
};

int GroupNode::precedence() const noexcept
{
    return std::numeric_limits<int>::max();
}

// =============================== |
//             CallNode            |
// =============================== |

std::string CallNode::to_string()
{
    return std::format(
        "CallNode<{}, {}>",
        callee->to_string(),
        utils::format_vector<pExprNode>(
            arguments, [](const pExprNode &expr) { return expr->to_string(); }
        )
    );
}

void CallNode::accept(NodeVisitor &visitor, U32 dst)
{
    visitor.visit(*this, dst);
}

// =============================== |
//            IndexNode            |
// =============================== |

std::string IndexNode::to_string()
{
    return std::format("IndexNode<{}, {}>", object->to_string(), index->to_string());
}

void IndexNode::accept(NodeVisitor &visitor, U32 dst)
{
    visitor.visit(*this, dst);
}

// =============================== |
//            BinaryNode           |
// =============================== |

std::string BinaryNode::to_string()
{
    return std::format(
        "Binary<{} '{}' {}>", lhs_expression->to_string(), op.lexeme, rhs_expression->to_string()
    );
}

void BinaryNode::accept(NodeVisitor &visitor, U32 dst)
{
    visitor.visit(*this, dst);
}

} // namespace via
