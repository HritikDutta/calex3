#include "platform/platform.h"
#include "calculator/misc.h"
#include "calculator/solver.h"
#include "calculator/token.h"
#include "containers/string.h"
#include "core/logger.h"

const char help_string[] =
"Calculate expressions.\n"
"   usage: % <expression>\n"
;

int main(int argc, char** argv)
{
    // Exit if no string is given
    if (argc < 2 || ref("help", 4) == ref(argv[1]))
    {
        print(help_string, argv[0]);
        return 0;
    }
    
    const String expression = ref(argv[1]);

    {   // Check for balanced brackets
        s32 diff = Calculator::balanced_brackets(expression);

        if (diff > 0)
        {
            print_error("Unbalanced brackets! There are more open brackets than closed brackets.\n");
            return 1;
        }
        else if (diff < 0)
        {
            print_error("Unbalanced brackets! There are more closed brackets than open brackets.\n");
            return 1;
        }
    }

    DynamicArray<Calculator::ExpressionElement> elements = {};
    bool success = Calculator::infix_expression_to_postfix(expression, elements);
    if (!success)
        return 1;

    f64 result = Calculator::solve_postfix_data(elements);
    print("Result: %\n", result);

    free(elements);
}