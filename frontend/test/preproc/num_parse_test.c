#include "frontend/preproc/num_parse.h"

#include "testing/asserts.h"

#define TEST_INT_LITERAL(constant, expected_val_type)                          \
    do {                                                                       \
        const char* const spell = #constant;                                   \
        const uintmax_t num = constant;                                        \
        struct preproc_err err = create_preproc_err();                         \
        const struct arch_type_info info = get_arch_type_info(ARCH_X86_64);    \
        const struct value val = parse_num_constant(spell,                     \
                                                    strlen(spell),             \
                                                    &err,                      \
                                                    &info.int_info);           \
        ASSERT(err.type == PREPROC_ERR_NONE);                                  \
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
        const struct arch_type_info info = get_arch_type_info(ARCH_X86_64);    \
        const struct value val = parse_num_constant(spell,                     \
                                                    strlen(spell),             \
                                                    &err,                      \
                                                    &info.int_info);           \
        ASSERT(err.type == PREPROC_ERR_NONE);                                  \
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

// TODO: too large error
TEST(int_min_fitting_type_decimal) {
    TEST_INT_LITERAL(10, VALUE_INT);
    TEST_INT_LITERAL(2147483647, VALUE_INT);
    TEST_INT_LITERAL(2147483648, VALUE_LINT);
    TEST_INT_LITERAL(9223372036854775807, VALUE_LINT);

    TEST_INT_LITERAL(10u, VALUE_UINT);
    TEST_INT_LITERAL(4294967295u, VALUE_UINT);
    TEST_INT_LITERAL(4294967296u, VALUE_ULINT);
    TEST_INT_LITERAL(18446744073709551615u, VALUE_ULINT);
}

TEST_SUITE_BEGIN(num_parse, 3) {
    REGISTER_TEST(integer);
    REGISTER_TEST(floating);
    REGISTER_TEST(int_min_fitting_type_decimal);
}
TEST_SUITE_END()
