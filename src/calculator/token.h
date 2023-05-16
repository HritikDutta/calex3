#pragma once

#include "containers/darray.h"
#include "containers/string.h"
#include "containers/function.h"
#include "platform/platform.h"

namespace Calculator
{

using Operation = Function<f64(f64[])>;

struct Operator
{
    Operation operation;
    u32 operand_count;
    u32 precedence;
};

struct ExpressionElement
{
    enum struct Type
    {
        NUMBER,
        OPERATOR,
    };

    Type type;

    union
    {
        f64 value;
        Operator op_data;
    };

    ExpressionElement(f64 value)
    :   type(Type::NUMBER), value(value)
    {
    }

    ExpressionElement(Operator op_data)
    :   type(Type::OPERATOR), op_data(op_data)
    {
    }

    ExpressionElement(const ExpressionElement& elem)
    {
        platform_copy_memory(this, &elem, sizeof(ExpressionElement));
    }

    ExpressionElement operator=(const ExpressionElement& elem)
    {
        platform_copy_memory(this, &elem, sizeof(ExpressionElement));
        return *this;
    }
};

bool infix_expression_to_postfix(const String expression, DynamicArray<ExpressionElement>& elements);

} // namespace Calculator
