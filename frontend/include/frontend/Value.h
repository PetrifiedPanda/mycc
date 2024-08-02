#ifndef MYCC_FRONTEND_VALUE_H
#define MYCC_FRONTEND_VALUE_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>

#include "util/Str.h"

typedef enum {
    INT_VAL_CHAR,
    INT_VAL_SHORT,
    INT_VAL_INT,
    INT_VAL_LINT,
    INT_VAL_LLINT,
    INT_VAL_UCHAR,
    INT_VAL_USHORT,
    INT_VAL_UINT,
    INT_VAL_ULINT,
    INT_VAL_ULLINT,
} IntValKind;

typedef struct IntVal {
    IntValKind kind;
    union {
        int64_t sint_val;
        uint64_t uint_val;  
    };
} IntVal;

typedef enum {
    FLOAT_VAL_FLOAT,
    FLOAT_VAL_DOUBLE,
    FLOAT_VAL_LDOUBLE,
} FloatValKind;

typedef struct FloatVal {
    FloatValKind kind;
    double val;
} FloatVal;

static_assert(sizeof(double) * CHAR_BIT == 64, "Double is not 64 bits");

IntVal IntVal_create_sint(IntValKind k, int64_t val);
IntVal IntVal_create_uint(IntValKind k, uint64_t val);
FloatVal FloatVal_create(FloatValKind k, double val);

bool IntValKind_is_sint(IntValKind k);
bool IntValKind_is_uint(IntValKind k);

Str IntValKind_str(IntValKind k);
Str FloatValKind_str(FloatValKind k);

#endif

