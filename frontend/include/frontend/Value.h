#ifndef MYCC_FRONTEND_VALUE_H
#define MYCC_FRONTEND_VALUE_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>

#include "util/Str.h"

typedef enum {
    VALUE_CHAR,
    VALUE_SHORT,
    VALUE_INT,
    VALUE_LINT,
    VALUE_LLINT,
    VALUE_UCHAR,
    VALUE_USHORT,
    VALUE_UINT,
    VALUE_ULINT,
    VALUE_ULLINT,
    VALUE_FLOAT,
    VALUE_DOUBLE,
    VALUE_LDOUBLE,
} ValueKind;

typedef struct Value {
    ValueKind kind;
    union {
        int64_t sint_val;
        uint64_t uint_val;
        double float_val;
    };
} Value;

static_assert(sizeof(double) * CHAR_BIT == 64, "Double is not 64 bits");

Value Value_create_sint(ValueKind k, int64_t val);
Value Value_create_uint(ValueKind k, uint64_t val);
Value Value_create_float(ValueKind k, double val);

bool ValueKind_is_sint(ValueKind k);
bool ValueKind_is_uint(ValueKind k);
bool ValueKind_is_float(ValueKind k);

Str ValueKind_str(ValueKind k);

#endif

