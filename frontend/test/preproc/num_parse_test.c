#include "frontend/preproc/num_parse.h"

#include "testing/asserts.h"

#define TEST_INT_LITERAL(constant, expected_val_type)                          \
    do {                                                                       \
        const char* const spell = #constant;                                   \
        const uintmax_t num = constant;                                        \
        struct preproc_err err = create_preproc_err();                         \
        struct value val = parse_num_constant(spell, strlen(spell), &err);     \
        ASSERT_UINTMAX_T(num, val.int_val);                                    \
        ASSERT(val.type == expected_val_type);                                 \
    } while (0)

TEST(integer) {
    TEST_INT_LITERAL(0x100ULL, VALUE_ULLINT);
    TEST_INT_LITERAL(031uLL, VALUE_ULLINT);
    TEST_INT_LITERAL(20000ull, VALUE_ULLINT);
    TEST_INT_LITERAL(1000Ull, VALUE_ULLINT);
    TEST_INT_LITERAL(0x1000l, VALUE_LINT);
    TEST_INT_LITERAL(200L, VALUE_LINT);
    TEST_INT_LITERAL(070u, VALUE_UINT);
    TEST_INT_LITERAL(0xabU, VALUE_UINT);
}

// TODO: weirdly different values
#define TEST_FLOAT_LITERAL(constant, expected_val_type)                        \
    do {                                                                       \
        const char* const spell = #constant;                                   \
        const long double num = constant;                                      \
        struct preproc_err err = create_preproc_err();                         \
        struct value val = parse_num_constant(spell, strlen(spell), &err);     \
        ASSERT_LONG_DOUBLE(num, val.float_val, 0.000000001l);                  \
        ASSERT(val.type == expected_val_type);                                 \
    } while (0)

TEST(floating) {
    TEST_FLOAT_LITERAL(1.0f, VALUE_FLOAT);
    TEST_FLOAT_LITERAL(0.05F, VALUE_FLOAT);
    TEST_FLOAT_LITERAL(.0l, VALUE_LDOUBLE);
    TEST_FLOAT_LITERAL(2.02342L, VALUE_LDOUBLE);
    TEST_FLOAT_LITERAL(1e-10f, VALUE_FLOAT);
    TEST_FLOAT_LITERAL(1e-10, VALUE_DOUBLE);
    TEST_FLOAT_LITERAL(0xabcdef0p-10, VALUE_DOUBLE);
    TEST_FLOAT_LITERAL(0xdeadbeefp-10L, VALUE_LDOUBLE);
    TEST_FLOAT_LITERAL(0xdeadp+10f, VALUE_FLOAT);
}

TEST_SUITE_BEGIN(num_parse, 2) {
    REGISTER_TEST(integer);
    REGISTER_TEST(floating);
}
TEST_SUITE_END()
