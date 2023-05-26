#ifndef VALUE_H
#define VALUE_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>

typedef enum {
    INT_VALUE_C,
    INT_VALUE_S,
    INT_VALUE_I,
    INT_VALUE_L,
    INT_VALUE_LL,
    INT_VALUE_UC,
    INT_VALUE_US,
    INT_VALUE_UI,
    INT_VALUE_UL,
    INT_VALUE_ULL,
} IntValueKind;

typedef struct {
    IntValueKind kind;
    union {
        int64_t int_val;
        uint64_t uint_val;
    };
} IntValue;

typedef enum {
    FLOAT_VALUE_F,
    FLOAT_VALUE_D,
    FLOAT_VALUE_LD,
} FloatValueKind;

typedef struct {
    FloatValueKind kind;
    double val;
} FloatValue;

static_assert(sizeof(double) * CHAR_BIT == 64, "Double is not 64 bits");

bool int_value_is_signed(IntValueKind t);
bool int_value_is_unsigned(IntValueKind t);

IntValue IntValue_create_signed(IntValueKind t, int64_t val);
IntValue IntValue_create_unsigned(IntValueKind t, uint64_t val);

FloatValue FloatValue_create(FloatValueKind t, double val);

const char* IntValueKind_str(IntValueKind k);
const char* FloatValueKind_str(FloatValueKind k);

#endif

