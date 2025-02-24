// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "ast.h"
#include "visitor.h"
#include "format_vec.h"

#define DEPTH_TAB ' '
#define DEPTH_TAB_SPACE std::string(depth, DEPTH_TAB)

namespace via {

std::string Modifiers::to_string() const noexcept
{
    return std::format("{}", is_const ? "const" : "");
}

// =============================== |
//          LiteralNode            |
// =============================== |

std::string LiteralNode::to_string(U32 &)
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
//            SymbolNode           |
// =============================== |

std::string SymbolNode::to_string(U32 &)
{
    return std::format("Symbol<{}>", identifier.lexeme);
}

void SymbolNode::accept(NodeVisitor &visitor, U32 dst)
{
    visitor.visit(*this, dst);
}

// =============================== |
//            UnaryNode            |
// =============================== |

std::string UnaryNode::to_string(U32 &depth)
{
    return std::format("Unary<{}>", expression->to_string(depth));
}

void UnaryNode::accept(NodeVisitor &visitor, U32 dst)
{
    visitor.visit(*this, dst);
}

// =============================== |
//            GroupNode            |
// =============================== |

std::string GroupNode::to_string(U32 &depth)
{
    return std::format("Group<{}>", expression->to_string(depth));
};

int GroupNode::precedence() const noexcept
{
    return std::numeric_limits<int>::max();
}

// =============================== |
//             CallNode            |
// =============================== |

std::string CallNode::to_string(U32 &depth)
{
    return std::format(
        "CallNode<callee {}, args {}>",
        callee->to_string(depth),
        utils::format_vector<pExprNode>(
            arguments, [&depth](const pExprNode &expr) { return expr->to_string(depth); }
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

std::string IndexNode::to_string(U32 &depth)
{
    return std::format(
        "IndexNode<object {}, index {}>", object->to_string(depth), index->to_string(depth)
    );
}

void IndexNode::accept(NodeVisitor &visitor, U32 dst)
{
    visitor.visit(*this, dst);
}

// =============================== |
//            BinaryNode           |
// =============================== |

std::string BinaryNode::to_string(U32 &depth)
{
    return std::format(
        "Binary<{} {} {}>",
        lhs_expression->to_string(depth),
        op.lexeme,
        rhs_expression->to_string(depth)
    );
}

void BinaryNode::accept(NodeVisitor &visitor, U32 dst)
{
    visitor.visit(*this, dst);
}

// =============================== |
//        DeclarationNode          |
// =============================== |

std::string DeclarationNode::to_string(U32 &depth)
{
    return std::format(
        "{}Declaration<{} {} {} = {}>",
        DEPTH_TAB_SPACE,
        is_global ? "global" : "local",
        modifiers.to_string(),
        identifier.lexeme,
        value_expression->to_string(depth)
    );
}

void DeclarationNode::accept(NodeVisitor &visitor)
{
    visitor.visit(*this);
}

// =============================== |
//        DeclarationNode          |
// =============================== |

std::string ScopeNode::to_string(U32 &depth)
{
    std::ostringstream oss;
    oss << DEPTH_TAB_SPACE << "Scope<>\n";

    depth++;

    for (const pStmtNode &pstmt : statements) {
        oss << pstmt->to_string(depth) << "\n";
    }

    depth--;

    oss << DEPTH_TAB_SPACE << "End<>";
    return oss.str();
}

void ScopeNode::accept(NodeVisitor &visitor)
{
    visitor.visit(*this);
}

// =============================== |
//          FunctionNode           |
// =============================== |

std::string FunctionNode::to_string(U32 &depth)
{
    std::ostringstream oss;
    oss << std::format(
        "{}Function<{} {} {}>\n",
        DEPTH_TAB_SPACE,
        is_global ? "global" : "local",
        modifiers.to_string(),
        identifier.lexeme
    );

    for (const ParameterNode &parameter : parameters) {
        oss << DEPTH_TAB_SPACE << std::format(" Parameter<{}>", parameter.identifier.lexeme)
            << "\n";
    }

    oss << body->to_string(depth) << "\n";
    oss << DEPTH_TAB_SPACE << "End<>";
    return oss.str();
}

void FunctionNode::accept(NodeVisitor &visitor)
{
    visitor.visit(*this);
}

// =============================== |
//           AssignNode            |
// =============================== |

std::string AssignNode::to_string(U32 &depth)
{
    return std::format(
        "{}Assign<{} {}= {}>",
        DEPTH_TAB_SPACE,
        augmentation_operator.lexeme,
        identifier.lexeme,
        value->to_string(depth)
    );
}

void AssignNode::accept(NodeVisitor &visitor)
{
    visitor.visit(*this);
}

// =============================== |
//          FunctionNode           |
// =============================== |

std::string IfNode::to_string(U32 &depth)
{
    std::ostringstream oss;
    oss << DEPTH_TAB_SPACE << std::format("IfNode<{}>", condition->to_string(depth)) << "\n";

    depth++;

    oss << scope->to_string(depth) << "\n";

    for (const ElseIfNode &elseif_node : elseif_nodes) {
        oss << DEPTH_TAB_SPACE << std::format("ElseIf<{}>", elseif_node.condition->to_string(depth))
            << "\n";
        depth++;
        oss << elseif_node.scope->to_string(depth) << "\n";
        depth--;
        oss << DEPTH_TAB_SPACE << "End<>\n";
    }

    if (else_node.has_value()) {
        oss << DEPTH_TAB_SPACE << "Else<>" << "\n";
        depth++;
        oss << else_node.value()->to_string(depth) << "\n";
        depth--;
        oss << DEPTH_TAB_SPACE << "End<>" << "\n";
    }

    depth--;

    oss << DEPTH_TAB_SPACE << "End<>" << "\n";
    return oss.str();
}

void IfNode::accept(NodeVisitor &visitor)
{
    visitor.visit(*this);
}

// =============================== |
//            WhileNode            |
// =============================== |

std::string WhileNode::to_string(U32 &depth)
{
    std::ostringstream oss;
    oss << DEPTH_TAB_SPACE << std::format("While<{}>", condition->to_string(depth)) << "\n";
    depth++;
    oss << body->to_string(depth) << "\n";
    depth--;
    oss << "End<>" << "\n";
    return oss.str();
}

void WhileNode::accept(NodeVisitor &visitor)
{
    visitor.visit(*this);
}

// =============================== |
//          ExprStmtNode           |
// =============================== |

std::string ExprStmtNode::to_string(U32 &depth)
{
    return std::format("{}ExpressionStatement<{}>", DEPTH_TAB_SPACE, expression->to_string(depth));
}

void ExprStmtNode::accept(NodeVisitor &visitor)
{
    visitor.visit(*this);
}

} // namespace via
