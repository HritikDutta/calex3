#pragma once

#include "containers/string.h"
#include "core/types.h"

namespace Calculator
{

// Returns integer difference between number of open and closed brackets
inline s32 balanced_brackets(const String str)
{
    s32 open = 0;

    for (s32 i = 0; i < str.size; i++)
        open += (str[i] == '(') - (str[i] == ')');

    return open;
}

} // namespace Calculator
