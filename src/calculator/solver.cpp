#include "solver.h"

#include "containers/darray.h"
#include "core/logger.h"
#include "core/types.h"
#include "token.h"

namespace Calculator
{

template<typename T>
using Stack = DynamicArray<T>;

f64 solve_postfix_data(const DynamicArray<ExpressionElement>& expression)
{
    Stack<f64> number_stack = make<Stack<f64>>(expression.size / 4);
    
    f64 operands[3];    // Max operand count in functions is 3

    for (u32 i = 0; i < expression.size; i++)
    {
        switch (expression[i].type)
        {
            case ExpressionElement::Type::NUMBER:
            {
                append(number_stack, expression[i].value);
            } break;
            
            case ExpressionElement::Type::OPERATOR:
            {
                const Operator op = expression[i].op_data;

                if (op.operand_count > number_stack.size)
                {
                    // TODO: Show which operator the error is for
                    print_error("Not enough operators for operator!");
                }

                u32 operand_count = op.operand_count;
                while (operand_count--)
                    operands[operand_count] = pop(number_stack);
                
                const f64 result = op.operation(operands);
                append(number_stack, result);
            } break;
        }
    }

    gn_assert_with_message(number_stack.size == 1, "Number stack has extra values! (numbers left: %)", number_stack.size);

    f64 result = pop(number_stack);
    free(number_stack);

    return result;
}

} // namespace Calculator
