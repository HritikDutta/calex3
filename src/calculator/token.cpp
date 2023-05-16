#include "token.h"

#include "containers/darray.h"
#include "containers/string.h"
#include "containers/hash.h"
#include "keywords.h"

namespace Calculator
{

struct OperatorOrBracket
{
    bool is_bracket;
    Operator op_data;
};

inline static bool is_digit(char ch)
{
    return (ch >= '0') && (ch <= '9');
}

inline static bool greater_precedence(const Operator& op1, const Operator& op2)
{
    return op1.operand_count > op2.operand_count || op1.precedence >= op2.precedence;
}

bool infix_expression_to_postfix(const String expression, DynamicArray<ExpressionElement>& elements)
{
    clear(elements);
    resize(elements, max(2Ui64, expression.size / 4));

    DynamicArray<OperatorOrBracket> temp_op_stack = make<DynamicArray<OperatorOrBracket>>(32Ui64);

    Hasher<String> string_hasher;

    // Keeps track if '-' is unary or binary
    bool allow_neg = true;
    u64 current_index = 0;

    bool encountered_error = false;

    while (true)
    {
        // Reached end of expression
        if (current_index >= expression.size)
            break;

        switch (expression[current_index])
        {
            // Skip whitespace
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            case '\0':
            {
                current_index++;
            } break;

            case '(':
            {
                OperatorOrBracket elem = OperatorOrBracket { true };
                append(temp_op_stack, elem);
                current_index++;
            } break;

            case ')':
            {
                while (!temp_op_stack[temp_op_stack.size - 1].is_bracket)
                    append(elements, ExpressionElement(pop(temp_op_stack).op_data));
                
                // Pop bracket
                pop(temp_op_stack);
                
                // '-' after closing bracket is binary
                allow_neg = false;

                current_index++;
            } break;
            
            // Number (integer or float)
            case '.':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            {
                bool encountered_dot = (expression[current_index] == '.');
                u64  number_size = 1;

                while (true)
                {
                    const u64 index = current_index + number_size;
                    
                    // Reached end of expression
                    if (index >= expression.size)
                        break;
                    
                    if (!is_digit(expression[index]))
                        break;

                    if (expression[index] == '.')
                    {
                        // There should be only 1 dot in a number
                        if (encountered_dot)
                        {
                            encountered_error = true;
                            break;
                        }

                        encountered_dot = true;
                    }

                    number_size++;
                }

                const String substring = ref(expression.data + current_index, number_size);
                const f64 value = atof(substring.data);
                append(elements, ExpressionElement(value));

                current_index += number_size;

                // '-' after a number is binary
                allow_neg = false;
            } break;

            default:
            {
                // Loop over all keywords to find best match (longest match)
                u32 keyword_index = keyword_table_size - 1;
                for (u32 i = 0; i < keyword_table_size; i++)
                {
                    const KeywordData& current_keyword = keyword_table[i];

                    // Don't check for negation if allow_neg is false
                    if (current_keyword.type == KeywordData::Type::OPERATOR &&
                        !allow_neg &&
                        current_keyword.op_data.precedence == 0)
                        continue;

                    const u64 length = current_keyword.str.size;
                    const String sub_string = get_substring(expression, current_index, length);
                    const Hash hash = string_hasher(sub_string);

                    if (hash == current_keyword.hash && sub_string == current_keyword.str)
                    {
                        // The longest matching substring is considered the correct keyword
                        const bool is_better_match = keyword_index == (keyword_table_size - 1) || current_keyword.str.size > keyword_table[keyword_index].str.size;
                        keyword_index = is_better_match ? i : keyword_index;
                    }
                }

                const KeywordData& match = keyword_table[keyword_index];
                current_index += match.str.size;

                switch (match.type)
                {
                    // Constants are treated like normal numbers
                    case KeywordData::Type::CONSTANT:
                    {
                        append(elements, ExpressionElement(match.value));
                        allow_neg = false;
                    } break;

                    case KeywordData::Type::OPERATOR:
                    {
                        // Pop all operators with lower or same precedence till a bracket is encountered
                        while (temp_op_stack.size != 0 &&
                               !temp_op_stack[temp_op_stack.size - 1].is_bracket &&
                               greater_precedence(match.op_data, temp_op_stack[temp_op_stack.size - 1].op_data))
                        {
                            append(elements, ExpressionElement(pop(temp_op_stack).op_data));
                        }

                        // Push operator into temp stack
                        OperatorOrBracket elem = OperatorOrBracket { false, match.op_data };
                        append(temp_op_stack, elem);

                        allow_neg = true;
                    } break;

                    case KeywordData::Type::EMPTY:
                    {
                        // Do nothing right now
                        // TODO: Maybe add a check for allowed keywords that get ignored (like comma, colon, etc.)
                    } break;
                }
            } break;
        }
    }

    // Add all remaining operators to expression
    while (temp_op_stack.size > 0)
    {
        append(elements, ExpressionElement(pop(temp_op_stack).op_data));
    }

    free(temp_op_stack);

    return !encountered_error;
}

} // namespace Calculator