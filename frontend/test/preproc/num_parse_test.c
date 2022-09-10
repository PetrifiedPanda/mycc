#include "frontend/preproc/num_parse.h"

#include "testing/asserts.h"

#define TEST_INT_LITERAL(constant, expected_val_type)                          \
    do {                                                                       \
        const char spell[] = #constant;                                        \
        const uintmax_t num = constant;                                        \
        const struct arch_type_info info = get_arch_type_info(ARCH_X86_64);    \
        const struct parse_int_const_res res = parse_int_const(                \
            spell,                                                             \
            (sizeof spell) - 1,                                                \
            &info.int_info);                                                   \
        ASSERT(res.err.type == INT_CONST_ERR_NONE);                            \
        ASSERT_UINTMAX_T(num, res.res.int_val);                                \
        ASSERT(res.res.type == expected_val_type);                             \
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
        const char spell[] = #constant;                                        \
        const long double num = constant;                                      \
        const struct parse_float_const_res res = parse_float_const(            \
            spell,                                                             \
            (sizeof spell) - 1);                                               \
        ASSERT(res.err.type == FLOAT_CONST_ERR_NONE);                          \
        ASSERT_LONG_DOUBLE(num, res.res.float_val, 0.000000001l);              \
        ASSERT(res.res.type == expected_val_type);                             \
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

TEST(int_min_fitting_type_hex) {
    TEST_INT_LITERAL(0xa, VALUE_INT);
    TEST_INT_LITERAL(0x7FFFFFFF, VALUE_INT);
    TEST_INT_LITERAL(0x80000000, VALUE_UINT);
    TEST_INT_LITERAL(0xFFFFFFFF, VALUE_UINT);
    TEST_INT_LITERAL(0x100000000, VALUE_LINT);
    TEST_INT_LITERAL(0x7FFFFFFFFFFFFFFF, VALUE_LINT);
    TEST_INT_LITERAL(0x8000000000000000, VALUE_ULINT);
    TEST_INT_LITERAL(0xFFFFFFFFFFFFFFFF, VALUE_ULINT);

    TEST_INT_LITERAL(0xau, VALUE_UINT);
    TEST_INT_LITERAL(0xFFFFFFFFu, VALUE_UINT);
    TEST_INT_LITERAL(0x100000000u, VALUE_ULINT);
    TEST_INT_LITERAL(0xFFFFFFFFFFFFFFFFu, VALUE_ULINT);
}

TEST(int_min_fitting_type_oct) {
    TEST_INT_LITERAL(012, VALUE_INT);
    TEST_INT_LITERAL(017777777777, VALUE_INT);
    TEST_INT_LITERAL(020000000000, VALUE_UINT);
    TEST_INT_LITERAL(037777777777, VALUE_UINT);
    TEST_INT_LITERAL(040000000000, VALUE_LINT);
    TEST_INT_LITERAL(0777777777777777777777, VALUE_LINT);
    TEST_INT_LITERAL(01000000000000000000000, VALUE_ULINT);
    TEST_INT_LITERAL(01777777777777777777777, VALUE_ULINT);

    TEST_INT_LITERAL(012u, VALUE_UINT);
    TEST_INT_LITERAL(037777777777u, VALUE_UINT);
    TEST_INT_LITERAL(040000000000u, VALUE_ULINT);
    TEST_INT_LITERAL(01777777777777777777777u, VALUE_ULINT);
}

static void test_parse_int_err(const char* spell, enum int_const_err_type err) {
    const struct arch_type_info info = get_arch_type_info(ARCH_X86_64);
    const struct parse_int_const_res res = parse_int_const(spell,
                                                           strlen(spell),
                                                           &info.int_info);
    ASSERT(res.err.type == err);
}
static void test_parse_float_err(const char* spell,
                                 enum float_const_err_type err) {
    const struct parse_float_const_res res = parse_float_const(spell,
                                                               strlen(spell));
    ASSERT(res.err.type == err);
}

static void test_parse_int_invalid_char(const char* spell, char invalid_char) {
    const struct arch_type_info info = get_arch_type_info(ARCH_X86_64);
    const struct parse_int_const_res res = parse_int_const(spell,
                                                           strlen(spell),
                                                           &info.int_info);
    ASSERT(res.err.type == INT_CONST_ERR_INVALID_CHAR);
    ASSERT_CHAR(res.err.invalid_char, invalid_char);
}

static void test_parse_float_invalid_char(const char* spell,
                                          char invalid_char) {
    const struct parse_float_const_res res = parse_float_const(spell,
                                                               strlen(spell));
    ASSERT(res.err.type == FLOAT_CONST_ERR_INVALID_CHAR);
    ASSERT_CHAR(res.err.invalid_char, invalid_char);
}

TEST(int_too_large) {
    test_parse_int_err("18446744073709551616u", INT_CONST_ERR_TOO_LARGE);
    test_parse_int_err("9223372036854775808", INT_CONST_ERR_TOO_LARGE);

    test_parse_int_err("0xffffffffffffffff0", INT_CONST_ERR_TOO_LARGE);
    test_parse_int_err("0xffffffffffffffff0U", INT_CONST_ERR_TOO_LARGE);

    test_parse_int_err("017777777777777777777770", INT_CONST_ERR_TOO_LARGE);
    test_parse_int_err("017777777777777777777770u", INT_CONST_ERR_TOO_LARGE);
}

TEST(int_suffix_error) {
    test_parse_int_err("10lul", INT_CONST_ERR_U_BETWEEN_LS);
    test_parse_int_err("10lUl", INT_CONST_ERR_U_BETWEEN_LS);

    test_parse_int_err("0x12lL", INT_CONST_ERR_CASE_MIXING);
    test_parse_int_err("0x12lLU", INT_CONST_ERR_CASE_MIXING);
    test_parse_int_err("0x12uLl", INT_CONST_ERR_CASE_MIXING);

    test_parse_int_err("0234234luu", INT_CONST_ERR_DOUBLE_U);
    test_parse_int_err("0234234UUl", INT_CONST_ERR_DOUBLE_U);

    test_parse_int_err("69lll", INT_CONST_ERR_TRIPLE_LONG);
    test_parse_int_err("69LLL", INT_CONST_ERR_TRIPLE_LONG);

    test_parse_int_err("69jdksaflajsdflk", INT_CONST_ERR_SUFFIX_TOO_LONG);
    test_parse_int_err("69ullabcdefghi", INT_CONST_ERR_SUFFIX_TOO_LONG);

    test_parse_int_invalid_char("420ub", 'b');
    test_parse_int_invalid_char("420ld", 'd');
}

TEST(float_suffix_error) {
    test_parse_float_err("0xabcdefp-10ll", FLOAT_CONST_ERR_SUFFIX_TOO_LONG);
    test_parse_float_err("0xabcdefp-10fl", FLOAT_CONST_ERR_SUFFIX_TOO_LONG);

    test_parse_float_invalid_char("12.5g", 'g');
    test_parse_float_invalid_char("12.5r", 'r');
}

// TODO: float too large

TEST_SUITE_BEGIN(num_parse, 8) {
    REGISTER_TEST(integer);
    REGISTER_TEST(floating);
    REGISTER_TEST(int_min_fitting_type_decimal);
    REGISTER_TEST(int_min_fitting_type_hex);
    REGISTER_TEST(int_min_fitting_type_oct);
    REGISTER_TEST(int_too_large);
    REGISTER_TEST(int_suffix_error);
    REGISTER_TEST(float_suffix_error);
}
TEST_SUITE_END()

