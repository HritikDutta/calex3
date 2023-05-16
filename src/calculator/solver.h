#pragma once

#include "containers/darray.h"
#include "core/types.h"
#include "token.h"

namespace Calculator
{

f64 solve_postfix_data(const DynamicArray<ExpressionElement>& expression);

} // namespace Calculator
