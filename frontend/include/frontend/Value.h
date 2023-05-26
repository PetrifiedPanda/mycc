#ifndef VALUE_H
#define VALUE_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>

typedef enum {
    VALUE_C,
    VALUE_S,
    VALUE_I,
    VALUE_L,
    VALUE_LL,
    VALUE_UC,
    VALUE_US,
    VALUE_UI,
    VALUE_UL,
    VALUE_ULL,
    VALUE_F,
    VALUE_D,
    VALUE_LD,
} ValueKind;

typedef struct {
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

const char* ValueKind_str(ValueKind k);

#endif

