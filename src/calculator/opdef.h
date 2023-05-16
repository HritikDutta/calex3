#pragma once

#include "math/common.h"

namespace Calculator
{

// Negation Operator
f64 NEG(f64 operands[])
{
    return -1 * operands[0];
}

// Multiplication Operator
f64 MULTIPLY(f64 operands[])
{
    return operands[0] * operands[1];
}

// Division Operator
f64 DIVIDE(f64 operands[])
{
    return operands[0] / operands[1];
}

// Addition Operator
f64 ADD(f64 operands[])
{
    return operands[0] + operands[1];
}

// Subtraction Operator
f64 SUBTRACT(f64 operands[])
{
    return operands[0] - operands[1];
}

// Power Operator
f64 POW(f64 operands[])
{
    return pow(operands[0], operands[1]);
}

// And Operator
f64 AND(f64 operands[])
{
    return operands[0] && operands[1];
}

// Or Operator
f64 OR(f64 operands[])
{
    return operands[0] || operands[1];
}

// Not Operator
f64 NOT(f64 operands[])
{
    return !operands[0];
}

// Greater Than Operator
f64 GREATER(f64 operands[])
{
    return operands[0] > operands[1];
}

// Lesser Than Operator
f64 LESSER(f64 operands[])
{
    return operands[0] < operands[1];
}

// Greater Than/Equal To Operator
f64 GREATER_EQUAL(f64 operands[])
{
    return operands[0] >= operands[1];
}

// Lesser Than/Equal To Operator
f64 LESSER_EQUAL(f64 operands[])
{
    return operands[0] <= operands[1];
}

// Equal To Operator
f64 EQUAL(f64 operands[])
{
    return operands[0] == operands[1];
}

// Not Equal To Operator
f64 NOT_EQUAL(f64 operands[])
{
    return operands[0] != operands[1];
}

// Natural Log Function
f64 NATURAL_LOG(f64 operands[])
{
    return log(operands[0]);
}

// Log Function
f64 LOG(f64 operands[])
{
    return log(operands[1]) / log(operands[0]);
}

// Sine Function
f64 SIN(f64 operands[])
{
    return sin(operands[0]);
}

// Cosine Function
f64 COS(f64 operands[])
{
    return cos(operands[0]);
}

// Tangent Function
f64 TAN(f64 operands[])
{
    return tan(operands[0]);
}

// Secant Function
f64 SEC(f64 operands[])
{
    return 1 / cos(operands[0]);
}

// Cosecant Function
f64 COSEC(f64 operands[])
{
    return 1 / sin(operands[0]);
}

// Cotangent Function
f64 COT(f64 operands[])
{
    return 1 / tan(operands[0]);
}

// Hyperbolic Sine Function
f64 SINH(f64 operands[])
{
    return sinh(operands[0]);
}

// Hyperbolic Cosine Function
f64 COSH(f64 operands[])
{
    return cosh(operands[0]);
}

// Hyperbolic Tangent Function
f64 TANH(f64 operands[])
{
    return tanh(operands[0]);
}

// Square Root Function
f64 SQRT(f64 operands[])
{
    return sqrt(operands[0]);
}

// Exponentiation (e^x) Function
f64 EXP(f64 operands[])
{
    return exp(operands[0]);
}

// Maximum between two numbers
f64 MAX(f64 operands[])
{
    return (operands[0] > operands[1]) ? operands[0] : operands[1];
}

// Minimum between two numbers
f64 MIN(f64 operands[])
{
    return (operands[0] < operands[1]) ? operands[0] : operands[1];
}

// Conditional Operator
f64 COND(f64 operands[])
{
    return (operands[0]) ? operands[1] : operands[2];
}

} // namespace Calculator
