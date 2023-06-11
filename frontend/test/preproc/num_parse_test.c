#include "frontend/preproc/num_parse.h"

#include "testing/asserts.h"

#define TEST_UINT_LITERAL(constant, expected_val_type)                         \
    do {                                                                       \
        const uint64_t num = constant;                                         \
        const ArchTypeInfo info = get_arch_type_info(ARCH_X86_64, false);      \
        const ParseIntConstRes res = parse_int_const(STR_LIT(#constant),       \
                                                     &info);                   \
        ASSERT(res.err.kind == INT_CONST_ERR_NONE);                            \
        ASSERT(res.res.kind == expected_val_type);                             \
        ASSERT_UINTMAX_T(num, res.res.uint_val);                               \
    } while (0)

#define TEST_INT_LITERAL(constant, expected_val_type)                          \
    do {                                                                       \
        const int64_t num = constant;                                          \
        const ArchTypeInfo info = get_arch_type_info(ARCH_X86_64, false);      \
        const ParseIntConstRes res = parse_int_const(STR_LIT(#constant),       \
                                                     &info);                   \
        ASSERT(res.err.kind == INT_CONST_ERR_NONE);                            \
        ASSERT(res.res.kind == expected_val_type);                             \
        ASSERT_UINTMAX_T(num, res.res.sint_val);                               \
    } while (0)

TEST(integer) {
    TEST_UINT_LITERAL(0x100ULL, VALUE_ULL);
    TEST_UINT_LITERAL(031uLL, VALUE_ULL);
    TEST_UINT_LITERAL(20000ull, VALUE_ULL);
    TEST_UINT_LITERAL(1000Ull, VALUE_ULL);
    TEST_INT_LITERAL(0x1000l, VALUE_L);
    TEST_INT_LITERAL(200L, VALUE_L);
    TEST_UINT_LITERAL(070u, VALUE_UI);
    TEST_UINT_LITERAL(0xabU, VALUE_UI);
}

// TODO: weirdly different values
#define TEST_FLOAT_LITERAL(constant, expected_val_type)                        \
    do {                                                                       \
        const double num = constant;                                           \
        const ParseFloatConstRes res = parse_float_const(STR_LIT(#constant));  \
        ASSERT(res.err.kind == FLOAT_CONST_ERR_NONE);                          \
        ASSERT_DOUBLE(num, res.res.float_val, 0.000000001l);                   \
        ASSERT(res.res.kind == expected_val_type);                             \
    } while (0)

TEST(floating) {
    TEST_FLOAT_LITERAL(1.0f, VALUE_F);
    TEST_FLOAT_LITERAL(1.0, VALUE_D);
    TEST_FLOAT_LITERAL(0.05F, VALUE_F);
    TEST_FLOAT_LITERAL(.0l, VALUE_LD);
    TEST_FLOAT_LITERAL(2.02342L, VALUE_LD);
    TEST_FLOAT_LITERAL(1e-10f, VALUE_F);
    TEST_FLOAT_LITERAL(1e-10, VALUE_D);
    TEST_FLOAT_LITERAL(0xabcdef0p-10, VALUE_D);
    TEST_FLOAT_LITERAL(0xdeadbeefp-10L, VALUE_LD);
    TEST_FLOAT_LITERAL(0xdeadp+10f, VALUE_F);
}

TEST(int_min_fitting_type_decimal) {
    TEST_INT_LITERAL(10, VALUE_I);
    TEST_INT_LITERAL(2147483647, VALUE_I);
    TEST_INT_LITERAL(2147483648, VALUE_L);
    TEST_INT_LITERAL(9223372036854775807, VALUE_L);

    TEST_UINT_LITERAL(10u, VALUE_UI);
    TEST_UINT_LITERAL(4294967295u, VALUE_UI);
    TEST_UINT_LITERAL(4294967296u, VALUE_UL);
    TEST_UINT_LITERAL(18446744073709551615u, VALUE_UL);
}

TEST(int_min_fitting_type_hex) {
    TEST_INT_LITERAL(0xa, VALUE_I);
    TEST_INT_LITERAL(0x7FFFFFFF, VALUE_I);
    TEST_UINT_LITERAL(0x80000000, VALUE_UI);
    TEST_UINT_LITERAL(0xFFFFFFFF, VALUE_UI);
    TEST_INT_LITERAL(0x100000000, VALUE_L);
    TEST_INT_LITERAL(0x7FFFFFFFFFFFFFFF, VALUE_L);
    TEST_UINT_LITERAL(0x8000000000000000, VALUE_UL);
    TEST_UINT_LITERAL(0xFFFFFFFFFFFFFFFF, VALUE_UL);

    TEST_UINT_LITERAL(0xau, VALUE_UI);
    TEST_UINT_LITERAL(0xFFFFFFFFu, VALUE_UI);
    TEST_UINT_LITERAL(0x100000000u, VALUE_UL);
    TEST_UINT_LITERAL(0xFFFFFFFFFFFFFFFFu, VALUE_UL);
}

TEST(int_min_fitting_type_oct) {
    TEST_INT_LITERAL(012, VALUE_I);
    TEST_INT_LITERAL(017777777777, VALUE_I);
    TEST_UINT_LITERAL(020000000000, VALUE_UI);
    TEST_UINT_LITERAL(037777777777, VALUE_UI);
    TEST_INT_LITERAL(040000000000, VALUE_L);
    TEST_INT_LITERAL(0777777777777777777777, VALUE_L);
    TEST_UINT_LITERAL(01000000000000000000000, VALUE_UL);
    TEST_UINT_LITERAL(01777777777777777777777, VALUE_UL);

    TEST_UINT_LITERAL(012u, VALUE_UI);
    TEST_UINT_LITERAL(037777777777u, VALUE_UI);
    TEST_UINT_LITERAL(040000000000u, VALUE_UL);
    TEST_UINT_LITERAL(01777777777777777777777u, VALUE_UL);
}

static void test_parse_int_err(Str spell, IntConstErrKind err) {
    const ArchTypeInfo info = get_arch_type_info(ARCH_X86_64, false);
    const ParseIntConstRes res = parse_int_const(spell, &info);
    ASSERT(res.err.kind == err);
}
static void test_parse_float_err(Str spell, FloatConstErrKind err) {
    const ParseFloatConstRes res = parse_float_const(spell);
    ASSERT(res.err.kind == err);
}

static void test_parse_int_invalid_char(Str spell, char invalid_char) {
    const ArchTypeInfo info = get_arch_type_info(ARCH_X86_64, false);
    const ParseIntConstRes res = parse_int_const(spell, &info);
    ASSERT(res.err.kind == INT_CONST_ERR_INVALID_CHAR);
    ASSERT_CHAR(res.err.invalid_char, invalid_char);
}

static void test_parse_float_invalid_char(Str spell, char invalid_char) {
    const ParseFloatConstRes res = parse_float_const(spell);
    ASSERT(res.err.kind == FLOAT_CONST_ERR_INVALID_CHAR);
    ASSERT_CHAR(res.err.invalid_char, invalid_char);
}

TEST(int_too_large) {
    test_parse_int_err(STR_LIT("18446744073709551616u"), INT_CONST_ERR_TOO_LARGE);
    test_parse_int_err(STR_LIT("9223372036854775808"), INT_CONST_ERR_TOO_LARGE);

    test_parse_int_err(STR_LIT("0xffffffffffffffff0"), INT_CONST_ERR_TOO_LARGE);
    test_parse_int_err(STR_LIT("0xffffffffffffffff0U"), INT_CONST_ERR_TOO_LARGE);

    test_parse_int_err(STR_LIT("017777777777777777777770"), INT_CONST_ERR_TOO_LARGE);
    test_parse_int_err(STR_LIT("017777777777777777777770u"), INT_CONST_ERR_TOO_LARGE);
}

TEST(int_suffix_error) {
    test_parse_int_err(STR_LIT("10lul"), INT_CONST_ERR_U_BETWEEN_LS);
    test_parse_int_err(STR_LIT("10lUl"), INT_CONST_ERR_U_BETWEEN_LS);

    test_parse_int_err(STR_LIT("0x12lL"), INT_CONST_ERR_CASE_MIXING);
    test_parse_int_err(STR_LIT("0x12lLU"), INT_CONST_ERR_CASE_MIXING);
    test_parse_int_err(STR_LIT("0x12uLl"), INT_CONST_ERR_CASE_MIXING);

    test_parse_int_err(STR_LIT("0234234luu"), INT_CONST_ERR_DOUBLE_U);
    test_parse_int_err(STR_LIT("0234234UUl"), INT_CONST_ERR_DOUBLE_U);

    test_parse_int_err(STR_LIT("69lll"), INT_CONST_ERR_TRIPLE_LONG);
    test_parse_int_err(STR_LIT("69LLL"), INT_CONST_ERR_TRIPLE_LONG);

    test_parse_int_err(STR_LIT("69LLUsaflajsdflk"), INT_CONST_ERR_SUFFIX_TOO_LONG);
    test_parse_int_err(STR_LIT("69ullabcdefghi"), INT_CONST_ERR_SUFFIX_TOO_LONG);

    test_parse_int_invalid_char(STR_LIT("420ub"), 'b');
    test_parse_int_invalid_char(STR_LIT("420ld"), 'd');
}

TEST(float_suffix_error) {
    test_parse_float_err(STR_LIT("0xabcdefp-10ll"),
                         FLOAT_CONST_ERR_SUFFIX_TOO_LONG);
    test_parse_float_err(STR_LIT("0xabcdefp-10fl"),
                         FLOAT_CONST_ERR_SUFFIX_TOO_LONG);

    test_parse_float_invalid_char(STR_LIT("12.5g"), 'g');
    test_parse_float_invalid_char(STR_LIT("12.5r"), 'r');
}

// TODO: float too large

TEST_SUITE_BEGIN(num_parse){
    REGISTER_TEST(integer),
    REGISTER_TEST(floating),
    REGISTER_TEST(int_min_fitting_type_decimal),
    REGISTER_TEST(int_min_fitting_type_hex),
    REGISTER_TEST(int_min_fitting_type_oct),
    REGISTER_TEST(int_too_large),
    REGISTER_TEST(int_suffix_error),
    REGISTER_TEST(float_suffix_error),
} TEST_SUITE_END()

