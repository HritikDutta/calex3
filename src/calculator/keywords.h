#pragma once

#include "containers/hash.h"
#include "containers/function.h"
#include "containers/string.h"
#include "core/types.h"
#include "token.h"

namespace Calculator
{

struct KeywordData
{
    enum struct Type
    {
        CONSTANT,
        OPERATOR,
        EMPTY
    };

    const Type   type;
    const String str;
    const Hash   hash;

    union
    {
        // Constant
        const f64 value;

        // Operator
        const Operator op_data;
    };

    KeywordData()
    : type(Type::EMPTY), str(ref("", 1)), hash(0)
    {
    }

    KeywordData(const String str, f64 value)
    :   type(Type::CONSTANT), str(str), hash(Hasher<String>()(str))
    ,   value(value)
    {
    }

    KeywordData(const String str, Operation operation, u32 operand_count, u32 precedence)
    :   type(Type::OPERATOR), str(str), hash(Hasher<String>()(str))
    ,   op_data({ operation, operand_count, precedence })
    {
    }
};

extern const KeywordData keyword_table[];
extern const u32 keyword_table_size;

} // namespace Calculator
