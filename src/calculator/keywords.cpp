#include "keywords.h"

#include "containers/hash.h"
#include "containers/function.h"
#include "containers/string.h"
#include "core/types.h"
#include "math/constants.h"
#include "token.h"
#include "opdef.h"

namespace Calculator
{

const KeywordData keyword_table[] =
{
    // Constants
    KeywordData(ref("pi"),     Math::PI),
    KeywordData(ref("e"),      Math::E),
    KeywordData(ref("true"),   true),
    KeywordData(ref("false"),  false),

    // Operators
    KeywordData(ref("-"), NEG, 1, 0),

    KeywordData(ref("sqrt"), SQRT,     1, 1),
    KeywordData(ref("exp"),  EXP,      1, 1),
    KeywordData(ref("*"),    MULTIPLY, 2, 1),
    KeywordData(ref("/"),    DIVIDE,   2, 1),
    KeywordData(ref("^"),    POW,      2, 1),

    KeywordData(ref("+"), ADD,      2, 2),
    KeywordData(ref("-"), SUBTRACT, 2, 2),

    KeywordData(ref("=="), EQUAL,         2, 3),
    KeywordData(ref(">"),  GREATER,       2, 3),
    KeywordData(ref(">="), GREATER_EQUAL, 2, 3),
    KeywordData(ref("<"),  LESSER,        2, 3),
    KeywordData(ref("<="), LESSER_EQUAL,  2, 3),

    KeywordData(ref("!"),   NOT, 1, 4),
    KeywordData(ref("not"), NOT, 1, 4),
    KeywordData(ref("&&"),  AND, 2, 4),
    KeywordData(ref("and"), AND, 2, 4),
    KeywordData(ref("||"),  OR,  2, 4),
    KeywordData(ref("or"),  OR,  2, 4),

    KeywordData(ref("ln"),  NATURAL_LOG, 1, 5),
    KeywordData(ref("log"), LOG,         1, 5),

    KeywordData(ref("sin"),   SIN,   1, 6),
    KeywordData(ref("cos"),   COS,   1, 6),
    KeywordData(ref("tan"),   TAN,   1, 6),
    KeywordData(ref("sec"),   SEC,   1, 6),
    KeywordData(ref("cosec"), COSEC, 1, 6),
    KeywordData(ref("cot"),   COT,   1, 6),
    KeywordData(ref("sinh"),  SINH,  1, 6),
    KeywordData(ref("cosh"),  COSH,  1, 6),
    KeywordData(ref("tanh"),  TANH,  1, 6),

    KeywordData(ref("max"), MAX, 2, 7),
    KeywordData(ref("min"), MIN, 2, 7),

    KeywordData(ref("if"), COND, 3, 8),

    // Empty (to find end of list)
    KeywordData()
};

const u32 keyword_table_size = sizeof(keyword_table) / sizeof(KeywordData);

} // namespace Calculator
