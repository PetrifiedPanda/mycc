#include <stdio.h>
#include <string.h>

#include "testing/asserts.h"

#include "frontend/preproc/preproc.h"

#include "../test_helpers.h"

static struct token create(enum token_type type,
                           struct str spelling,
                           size_t line,
                           size_t index);
static struct token create_tok_int(struct int_value val,
                                   size_t line,
                                   size_t index);
static struct token create_tok_float(struct float_value val,
                                     size_t line,
                                     size_t index);

static void check_token_arr_file(const char* filename,
                                 const struct token* expected,
                                 size_t expected_len);
static void check_token_arr_str(const char* code,
                                const struct token* expected,
                                size_t expected_len);

TEST(simple) {
    const char*
        code = "typedef struct typedeftest /* This is a comment \n"
               "that goes over\n"
               "multiple lines\n"
               "*/\n"
               "{\n"
               "\tlong int* n;\n"
               "const long double *m;\n"
               "} Typedeftest; // Line comment\n"
               "const char* lstr = \n"
               "L\"Long string literal to check if long strings work\";\n"
               "int n = 0x123213 + 132 << 32 >> 0x123 - 0123 / 12;\n"
               "const char* str = \"Normal string literal\";\n"
               "int arr[1 ? 100 : 1000];\n";

    const struct token expected[] = {
        create(TYPEDEF, create_null_str(), 1, 1),
        create(STRUCT, create_null_str(), 1, 9),
        create(IDENTIFIER, STR_NON_HEAP("typedeftest"), 1, 16),
        create(LBRACE, create_null_str(), 5, 1),
        create(LONG, create_null_str(), 6, 2),
        create(INT, create_null_str(), 6, 7),
        create(ASTERISK, create_null_str(), 6, 10),
        create(IDENTIFIER, STR_NON_HEAP("n"), 6, 12),
        create(SEMICOLON, create_null_str(), 6, 13),
        create(CONST, create_null_str(), 7, 1),
        create(LONG, create_null_str(), 7, 7),
        create(DOUBLE, create_null_str(), 7, 12),
        create(ASTERISK, create_null_str(), 7, 19),
        create(IDENTIFIER, STR_NON_HEAP("m"), 7, 20),
        create(SEMICOLON, create_null_str(), 7, 21),
        create(RBRACE, create_null_str(), 8, 1),
        create(IDENTIFIER, STR_NON_HEAP("Typedeftest"), 8, 3),
        create(SEMICOLON, create_null_str(), 8, 14),
        create(CONST, create_null_str(), 9, 1),
        create(CHAR, create_null_str(), 9, 7),
        create(ASTERISK, create_null_str(), 9, 11),
        create(IDENTIFIER, STR_NON_HEAP("lstr"), 9, 13),
        create(ASSIGN, create_null_str(), 9, 18),
        create(STRING_LITERAL,
               STR_NON_HEAP(
                   "L\"Long string literal to check if long strings work\""),
               10,
               1),
        create(SEMICOLON, create_null_str(), 10, 53),
        create(INT, create_null_str(), 11, 1),
        create(IDENTIFIER, STR_NON_HEAP("n"), 11, 5),
        create(ASSIGN, create_null_str(), 11, 7),
        create_tok_int(create_int_value(INT_VALUE_I, 0x123213), 11, 9),
        create(ADD, create_null_str(), 11, 18),
        create_tok_int(create_int_value(INT_VALUE_I, 132), 11, 20),
        create(LEFT_OP, create_null_str(), 11, 24),
        create_tok_int(create_int_value(INT_VALUE_I, 32), 11, 27),
        create(RIGHT_OP, create_null_str(), 11, 30),
        create_tok_int(create_int_value(INT_VALUE_I, 0x123), 11, 33),
        create(SUB, create_null_str(), 11, 39),
        create_tok_int(create_int_value(INT_VALUE_I, 0123), 11, 41),
        create(DIV, create_null_str(), 11, 46),
        create_tok_int(create_int_value(INT_VALUE_I, 12), 11, 48),
        create(SEMICOLON, create_null_str(), 11, 50),
        create(CONST, create_null_str(), 12, 1),
        create(CHAR, create_null_str(), 12, 7),
        create(ASTERISK, create_null_str(), 12, 11),
        create(IDENTIFIER, STR_NON_HEAP("str"), 12, 13),
        create(ASSIGN, create_null_str(), 12, 17),
        create(STRING_LITERAL,
               STR_NON_HEAP("\"Normal string literal\""),
               12,
               19),
        create(SEMICOLON, create_null_str(), 12, 42),
        create(INT, create_null_str(), 13, 1),
        create(IDENTIFIER, STR_NON_HEAP("arr"), 13, 5),
        create(LINDEX, create_null_str(), 13, 8),
        create_tok_int(create_int_value(INT_VALUE_I, 1), 13, 9),
        create(QMARK, create_null_str(), 13, 11),
        create_tok_int(create_int_value(INT_VALUE_I, 100), 13, 13),
        create(COLON, create_null_str(), 13, 17),
        create_tok_int(create_int_value(INT_VALUE_I, 1000), 13, 19),
        create(RINDEX, create_null_str(), 13, 23),
        create(SEMICOLON, create_null_str(), 13, 24),
    };
    check_token_arr_str(code, expected, sizeof expected / sizeof *expected);
}

TEST(file) {
    const char* filename = "../frontend/test/files/no_preproc.c";

    const struct token expected[] = {
        create(TYPEDEF, create_null_str(), 3, 1),
        create(STRUCT, create_null_str(), 3, 9),
        create(LBRACE, create_null_str(), 3, 16),
        create(ATOMIC, create_null_str(), 4, 5),
        create(VOLATILE, create_null_str(), 4, 13),
        create(INT, create_null_str(), 4, 22),
        create(ASTERISK, create_null_str(), 4, 25),
        create(RESTRICT, create_null_str(), 4, 27),
        create(IDENTIFIER, STR_NON_HEAP("ptr"), 4, 36),
        create(SEMICOLON, create_null_str(), 4, 39),
        create(ALIGNAS, create_null_str(), 5, 5),
        create(LBRACKET, create_null_str(), 5, 13),
        create_tok_int(create_int_value(INT_VALUE_I, 16), 5, 14),
        create(RBRACKET, create_null_str(), 5, 16),
        create(CONST, create_null_str(), 5, 18),
        create(CHAR, create_null_str(), 5, 24),
        create(ASTERISK, create_null_str(), 5, 29),
        create(IDENTIFIER, STR_NON_HEAP("str"), 5, 31),
        create(SEMICOLON, create_null_str(), 5, 34),
        create(RBRACE, create_null_str(), 6, 1),
        create(IDENTIFIER, STR_NON_HEAP("MyStruct"), 6, 3),
        create(SEMICOLON, create_null_str(), 6, 11),
        create(UNION, create_null_str(), 8, 1),
        create(IDENTIFIER, STR_NON_HEAP("my_union"), 8, 7),
        create(LBRACE, create_null_str(), 8, 16),
        create(SHORT, create_null_str(), 9, 5),
        create(INT, create_null_str(), 9, 11),
        create(IDENTIFIER, STR_NON_HEAP("i"), 9, 15),
        create(COLON, create_null_str(), 9, 17),
        create_tok_int(create_int_value(INT_VALUE_I, 4), 9, 19),
        create(COMMA, create_null_str(), 9, 20),
        create(COLON, create_null_str(), 9, 22),
        create_tok_int(create_int_value(INT_VALUE_I, 4), 9, 24),
        create(SEMICOLON, create_null_str(), 9, 25),
        create(ALIGNAS, create_null_str(), 10, 5),
        create(LBRACKET, create_null_str(), 10, 13),
        create(CONST, create_null_str(), 10, 14),
        create(IDENTIFIER, STR_NON_HEAP("MyStruct"), 10, 20),
        create(RBRACKET, create_null_str(), 10, 28),
        create(FLOAT, create_null_str(), 10, 30),
        create(IDENTIFIER, STR_NON_HEAP("f"), 10, 36),
        create(SEMICOLON, create_null_str(), 10, 37),
        create(RBRACE, create_null_str(), 11, 1),
        create(SEMICOLON, create_null_str(), 11, 2),
        create(ENUM, create_null_str(), 14, 1),
        create(IDENTIFIER, STR_NON_HEAP("my_enum"), 14, 6),
        create(LBRACE, create_null_str(), 14, 14),
        create(IDENTIFIER, STR_NON_HEAP("VAL_1"), 15, 5),
        create(COMMA, create_null_str(), 15, 10),
        create(IDENTIFIER, STR_NON_HEAP("VAL_2"), 16, 5),
        create(COMMA, create_null_str(), 16, 10),
        create(IDENTIFIER, STR_NON_HEAP("VAL_3"), 17, 5),
        create(RBRACE, create_null_str(), 18, 1),
        create(SEMICOLON, create_null_str(), 18, 2),
        create(STATIC, create_null_str(), 20, 1),
        create(INLINE, create_null_str(), 20, 8),
        create(INT, create_null_str(), 20, 15),
        create(IDENTIFIER, STR_NON_HEAP("do_shit"), 20, 19),
        create(LBRACKET, create_null_str(), 20, 26),
        create(INT, create_null_str(), 20, 27),
        create(IDENTIFIER, STR_NON_HEAP("n"), 20, 31),
        create(COMMA, create_null_str(), 20, 32),
        create(INT, create_null_str(), 20, 34),
        create(IDENTIFIER, STR_NON_HEAP("m"), 20, 38),
        create(RBRACKET, create_null_str(), 20, 39),
        create(SEMICOLON, create_null_str(), 20, 40),
        create(STATIC, create_null_str(), 22, 1),
        create(VOID, create_null_str(), 22, 8),
        create(IDENTIFIER, STR_NON_HEAP("variadic"), 22, 13),
        create(LBRACKET, create_null_str(), 22, 21),
        create(INT, create_null_str(), 22, 22),
        create(IDENTIFIER, STR_NON_HEAP("m"), 22, 26),
        create(COMMA, create_null_str(), 22, 27),
        create(ELLIPSIS, create_null_str(), 22, 29),
        create(RBRACKET, create_null_str(), 22, 32),
        create(SEMICOLON, create_null_str(), 22, 33),
        create(EXTERN, create_null_str(), 24, 1),
        create(INT, create_null_str(), 24, 8),
        create(IDENTIFIER, STR_NON_HEAP("some_func"), 24, 12),
        create(LBRACKET, create_null_str(), 24, 21),
        create(RBRACKET, create_null_str(), 24, 22),
        create(SEMICOLON, create_null_str(), 24, 23),
        create(INT, create_null_str(), 26, 1),
        create(IDENTIFIER, STR_NON_HEAP("main"), 26, 5),
        create(LBRACKET, create_null_str(), 26, 9),
        create(RBRACKET, create_null_str(), 26, 10),
        create(LBRACE, create_null_str(), 26, 12),
        create(REGISTER, create_null_str(), 27, 5),
        create(ENUM, create_null_str(), 27, 14),
        create(IDENTIFIER, STR_NON_HEAP("my_enum"), 27, 19),
        create(IDENTIFIER, STR_NON_HEAP("type"), 27, 27),
        create(ASSIGN, create_null_str(), 27, 32),
        create(IDENTIFIER, STR_NON_HEAP("VAL_1"), 27, 34),
        create(SEMICOLON, create_null_str(), 27, 39),
        create(AUTO, create_null_str(), 29, 5),
        create(LONG, create_null_str(), 29, 10),
        create(SIGNED, create_null_str(), 29, 15),
        create(INT, create_null_str(), 29, 22),
        create(IDENTIFIER, STR_NON_HEAP("value"), 29, 26),
        create(SEMICOLON, create_null_str(), 29, 31),
        create(SWITCH, create_null_str(), 30, 5),
        create(LBRACKET, create_null_str(), 30, 12),
        create(IDENTIFIER, STR_NON_HEAP("type"), 30, 13),
        create(RBRACKET, create_null_str(), 30, 17),
        create(LBRACE, create_null_str(), 30, 19),
        create(CASE, create_null_str(), 31, 9),
        create(IDENTIFIER, STR_NON_HEAP("VAL_1"), 31, 14),
        create(COLON, create_null_str(), 31, 19),
        create(CASE, create_null_str(), 32, 9),
        create(IDENTIFIER, STR_NON_HEAP("VAL_2"), 32, 14),
        create(COLON, create_null_str(), 32, 19),
        create(IDENTIFIER, STR_NON_HEAP("value"), 33, 13),
        create(ASSIGN, create_null_str(), 33, 19),
        create_tok_int(create_int_value(INT_VALUE_L, 1000l), 33, 21),
        create(MOD, create_null_str(), 33, 27),
        create_tok_int(create_int_value(INT_VALUE_I, 5), 33, 29),
        create(SEMICOLON, create_null_str(), 33, 30),
        create(BREAK, create_null_str(), 34, 13),
        create(SEMICOLON, create_null_str(), 34, 18),
        create(DEFAULT, create_null_str(), 36, 9),
        create(COLON, create_null_str(), 36, 16),
        create(IDENTIFIER, STR_NON_HEAP("value"), 37, 13),
        create(ASSIGN, create_null_str(), 37, 19),
        create_tok_int(create_int_value(INT_VALUE_L, 30l), 37, 21),
        create(SEMICOLON, create_null_str(), 37, 24),
        create(BREAK, create_null_str(), 38, 13),
        create(SEMICOLON, create_null_str(), 38, 18),
        create(RBRACE, create_null_str(), 39, 5),
        create(IDENTIFIER, STR_NON_HEAP("MyStruct"), 46, 5),
        create(IDENTIFIER, STR_NON_HEAP("s"), 46, 14),
        create(ASSIGN, create_null_str(), 46, 16),
        create(LBRACE, create_null_str(), 46, 18),
        create_tok_int(create_int_value(INT_VALUE_I, 0), 46, 19),
        create(COMMA, create_null_str(), 46, 20),
        create(STRING_LITERAL,
               STR_NON_HEAP("L\"Hello there, this string literal needs to be "
                            "longer than 512 "
                            "characters oh no I don't know what to write here "
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaa\""),
               46,
               22),
        create(RBRACE, create_null_str(), 46, 1204),
        create(SEMICOLON, create_null_str(), 46, 1205),
        create(INT, create_null_str(), 47, 5),
        create(IDENTIFIER, STR_NON_HEAP("integer"), 47, 9),
        create(ASSIGN, create_null_str(), 47, 17),
        create_tok_int(create_int_value(INT_VALUE_I, 01000), 47, 19),
        create(LEFT_OP, create_null_str(), 47, 25),
        create_tok_int(create_int_value(INT_VALUE_I, 10), 47, 28),
        create(SEMICOLON, create_null_str(), 47, 30),
        create(IDENTIFIER, STR_NON_HEAP("s"), 48, 5),
        create(DOT, create_null_str(), 48, 6),
        create(IDENTIFIER, STR_NON_HEAP("ptr"), 48, 7),
        create(ASSIGN, create_null_str(), 48, 11),
        create(AND, create_null_str(), 48, 13),
        create(IDENTIFIER, STR_NON_HEAP("integer"), 48, 14),
        create(SEMICOLON, create_null_str(), 48, 21),
        create(IDENTIFIER, STR_NON_HEAP("MyStruct"), 50, 5),
        create(ASTERISK, create_null_str(), 50, 14),
        create(IDENTIFIER, STR_NON_HEAP("s_ptr"), 50, 15),
        create(ASSIGN, create_null_str(), 50, 21),
        create(AND, create_null_str(), 50, 23),
        create(IDENTIFIER, STR_NON_HEAP("s"), 50, 24),
        create(SEMICOLON, create_null_str(), 50, 25),
        create(IDENTIFIER, STR_NON_HEAP("s_ptr"), 51, 5),
        create(PTR_OP, create_null_str(), 51, 10),
        create(IDENTIFIER, STR_NON_HEAP("str"), 51, 12),
        create(ASSIGN, create_null_str(), 51, 16),
        create(STRING_LITERAL, STR_NON_HEAP("\"Goodbye\""), 51, 18),
        create(SEMICOLON, create_null_str(), 51, 27),
        create(ASTERISK, create_null_str(), 52, 5),
        create(IDENTIFIER, STR_NON_HEAP("s_ptr"), 52, 6),
        create(ASSIGN, create_null_str(), 52, 12),
        create(LBRACKET, create_null_str(), 52, 14),
        create(IDENTIFIER, STR_NON_HEAP("MyStruct"), 52, 15),
        create(RBRACKET, create_null_str(), 52, 23),
        create(LBRACE, create_null_str(), 52, 24),
        create(STRING_LITERAL,
               STR_NON_HEAP("L\"\\\"Lstrings seem to be int pointers\\\"\""),
               52,
               25),
        create(COMMA, create_null_str(), 52, 64),
        create(STRING_LITERAL, STR_NON_HEAP("\"doot\""), 52, 66),
        create(RBRACE, create_null_str(), 52, 72),
        create(SEMICOLON, create_null_str(), 52, 73),
        create(UNION, create_null_str(), 54, 5),
        create(IDENTIFIER, STR_NON_HEAP("my_union"), 54, 11),
        create(IDENTIFIER, STR_NON_HEAP("soviet_union"), 54, 20),
        create(SEMICOLON, create_null_str(), 54, 32),
        create(IDENTIFIER, STR_NON_HEAP("soviet_union"), 55, 5),
        create(DOT, create_null_str(), 55, 17),
        create(IDENTIFIER, STR_NON_HEAP("i"), 55, 18),
        create(ASSIGN, create_null_str(), 55, 20),
        create_tok_int(create_int_value(INT_VALUE_I, 0x1000), 55, 22),
        create(ADD, create_null_str(), 55, 29),
        create_tok_int(create_int_value(INT_VALUE_I, 033242), 55, 31),
        create(SEMICOLON, create_null_str(), 55, 37),
        create(INT, create_null_str(), 56, 5),
        create(
            IDENTIFIER,
            STR_NON_HEAP(
                "super_long_identifier_that_needs_to_be_over_512_characters_"
                "long_"
                "what_the_hell_am_i_supposed_to_write_here_a_b_c_d_e_f_g_h_i_j_"
                "k_l_"
                "m_n_o_p_q_r_s_t_u_v_w_x_y_z_"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaacccccccccccccccccccc"
                "cccc"
                "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
                "cccc"
                "ccccccccccccccccccccccccccccccccccccccccccccccccccccoooooooooo"
                "oooo"
                "ooooooooooooooooooooooooooooooosoooooooooooooooooooooooooooodf"
                "sooo"
                "ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo_"
                "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
                "ssss"
                "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
                "ssss"
                "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
                "ssss"
                "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
                "ssss"
                "ssssssssssssssssssssssssssssssssssssssssssssssssssssss"),
            56,
            9),
        create(SEMICOLON, create_null_str(), 56, 1141),
        create(GOTO, create_null_str(), 57, 5),
        create(IDENTIFIER, STR_NON_HEAP("my_cool_label"), 57, 10),
        create(SEMICOLON, create_null_str(), 57, 23),
        create(RETURN, create_null_str(), 59, 5),
        create(ALIGNOF, create_null_str(), 59, 12),
        create(LBRACKET, create_null_str(), 59, 20),
        create(LONG, create_null_str(), 59, 21),
        create(RBRACKET, create_null_str(), 59, 25),
        create(SEMICOLON, create_null_str(), 59, 26),
        create(IDENTIFIER, STR_NON_HEAP("my_cool_label"), 61, 1),
        create(COLON, create_null_str(), 61, 14),
        create(
            IDENTIFIER,
            STR_NON_HEAP(
                "super_long_identifier_that_needs_to_be_over_512_characters_"
                "long_"
                "what_the_hell_am_i_supposed_to_write_here_a_b_c_d_e_f_g_h_i_j_"
                "k_l_"
                "m_n_o_p_q_r_s_t_u_v_w_x_y_z_"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaacccccccccccccccccccc"
                "cccc"
                "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
                "cccc"
                "ccccccccccccccccccccccccccccccccccccccccccccccccccccoooooooooo"
                "oooo"
                "ooooooooooooooooooooooooooooooosoooooooooooooooooooooooooooodf"
                "sooo"
                "ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo_"
                "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
                "ssss"
                "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
                "ssss"
                "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
                "ssss"
                "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
                "ssss"
                "ssssssssssssssssssssssssssssssssssssssssssssssssssssss"),
            62,
            5),
        create(ASSIGN, create_null_str(), 62, 1138),
        create(LBRACKET, create_null_str(), 62, 1140),
        create(INT, create_null_str(), 62, 1141),
        create(RBRACKET, create_null_str(), 62, 1144),
        create(IDENTIFIER, STR_NON_HEAP("soviet_union"), 62, 1145),
        create(DOT, create_null_str(), 62, 1157),
        create(IDENTIFIER, STR_NON_HEAP("f"), 62, 1158),
        create(SEMICOLON, create_null_str(), 62, 1159),
        create(RBRACE, create_null_str(), 63, 1),
        create(STATIC, create_null_str(), 65, 1),
        create(INT, create_null_str(), 65, 8),
        create(IDENTIFIER, STR_NON_HEAP("do_shit"), 65, 12),
        create(LBRACKET, create_null_str(), 65, 19),
        create(IDENTIFIER, STR_NON_HEAP("n"), 65, 20),
        create(COMMA, create_null_str(), 65, 21),
        create(IDENTIFIER, STR_NON_HEAP("m"), 65, 23),
        create(RBRACKET, create_null_str(), 65, 24),
        create(INT, create_null_str(), 65, 26),
        create(IDENTIFIER, STR_NON_HEAP("n"), 65, 30),
        create(SEMICOLON, create_null_str(), 65, 31),
        create(INT, create_null_str(), 65, 33),
        create(IDENTIFIER, STR_NON_HEAP("m"), 65, 37),
        create(SEMICOLON, create_null_str(), 65, 38),
        create(LBRACE, create_null_str(), 65, 40),
        create(DOUBLE, create_null_str(), 66, 5),
        create(IDENTIFIER, STR_NON_HEAP("d"), 66, 12),
        create(ASSIGN, create_null_str(), 66, 14),
        create_tok_float(create_float_value(FLOAT_VALUE_D, 1e-10), 66, 16),
        create(SUB, create_null_str(), 66, 22),
        create_tok_float(create_float_value(FLOAT_VALUE_D, 0xabecp10), 66, 24),
        create(SEMICOLON, create_null_str(), 66, 33),
        create(INT, create_null_str(), 67, 5),
        create(IDENTIFIER, STR_NON_HEAP("type_size"), 67, 9),
        create(ASSIGN, create_null_str(), 67, 19),
        create(SIZEOF, create_null_str(), 67, 21),
        create(LBRACKET, create_null_str(), 67, 27),
        create(DOUBLE, create_null_str(), 67, 28),
        create(RBRACKET, create_null_str(), 67, 34),
        create(SEMICOLON, create_null_str(), 67, 35),
        create(INT, create_null_str(), 68, 5),
        create(IDENTIFIER, STR_NON_HEAP("size"), 68, 9),
        create(ASSIGN, create_null_str(), 68, 14),
        create(SIZEOF, create_null_str(), 68, 16),
        create(IDENTIFIER, STR_NON_HEAP("d"), 68, 23),
        create(SEMICOLON, create_null_str(), 68, 24),
        create(CHAR, create_null_str(), 70, 5),
        create(IDENTIFIER, STR_NON_HEAP("c"), 70, 10),
        create(ASSIGN, create_null_str(), 70, 12),
        create_tok_int(create_int_value(INT_VALUE_I, '\n'), 70, 14),
        create(SEMICOLON, create_null_str(), 70, 18),
        create(BOOL, create_null_str(), 72, 5),
        create(IDENTIFIER, STR_NON_HEAP("true_bool"), 72, 11),
        create(ASSIGN, create_null_str(), 72, 21),
        create(NOT, create_null_str(), 72, 23),
        create(LBRACKET, create_null_str(), 72, 24),
        create(IDENTIFIER, STR_NON_HEAP("c"), 72, 25),
        create(NE_OP, create_null_str(), 72, 27),
        create(IDENTIFIER, STR_NON_HEAP("c"), 72, 30),
        create(RBRACKET, create_null_str(), 72, 31),
        create(AND_OP, create_null_str(), 72, 33),
        create(LBRACKET, create_null_str(), 72, 36),
        create(IDENTIFIER, STR_NON_HEAP("c"), 72, 37),
        create(LE_OP, create_null_str(), 72, 39),
        create(IDENTIFIER, STR_NON_HEAP("size"), 72, 42),
        create(OR_OP, create_null_str(), 72, 47),
        create(IDENTIFIER, STR_NON_HEAP("c"), 72, 50),
        create(GE_OP, create_null_str(), 72, 52),
        create(IDENTIFIER, STR_NON_HEAP("size"), 72, 55),
        create(OR_OP, create_null_str(), 72, 60),
        create(IDENTIFIER, STR_NON_HEAP("c"), 72, 63),
        create(LT, create_null_str(), 72, 65),
        create(IDENTIFIER, STR_NON_HEAP("size"), 72, 67),
        create(OR_OP, create_null_str(), 72, 72),
        create(IDENTIFIER, STR_NON_HEAP("c"), 72, 75),
        create(GT, create_null_str(), 72, 77),
        create(IDENTIFIER, STR_NON_HEAP("size"), 72, 79),
        create(OR_OP, create_null_str(), 72, 84),
        create(IDENTIFIER, STR_NON_HEAP("c"), 72, 87),
        create(EQ_OP, create_null_str(), 72, 89),
        create(IDENTIFIER, STR_NON_HEAP("size"), 72, 92),
        create(OR_OP, create_null_str(), 72, 97),
        create(IDENTIFIER, STR_NON_HEAP("c"), 72, 100),
        create(NE_OP, create_null_str(), 72, 102),
        create(IDENTIFIER, STR_NON_HEAP("size"), 72, 105),
        create(RBRACKET, create_null_str(), 72, 109),
        create(SEMICOLON, create_null_str(), 72, 110),
        create(ENUM, create_null_str(), 74, 5),
        create(LBRACE, create_null_str(), 74, 10),
        create(IDENTIFIER, STR_NON_HEAP("LIMIT"), 74, 12),
        create(ASSIGN, create_null_str(), 74, 18),
        create_tok_int(create_int_value(INT_VALUE_I, 50000), 74, 20),
        create(RBRACE, create_null_str(), 74, 26),
        create(SEMICOLON, create_null_str(), 74, 27),
        create(CHAR, create_null_str(), 75, 5),
        create(IDENTIFIER, STR_NON_HEAP("arr"), 75, 10),
        create(LINDEX, create_null_str(), 75, 13),
        create(IDENTIFIER, STR_NON_HEAP("LIMIT"), 75, 14),
        create(RINDEX, create_null_str(), 75, 19),
        create(ASSIGN, create_null_str(), 75, 21),
        create(LBRACE, create_null_str(), 75, 23),
        create_tok_int(create_int_value(INT_VALUE_I, 0), 75, 24),
        create(RBRACE, create_null_str(), 75, 25),
        create(SEMICOLON, create_null_str(), 75, 26),
        create(FOR, create_null_str(), 76, 5),
        create(LBRACKET, create_null_str(), 76, 9),
        create(INT, create_null_str(), 76, 10),
        create(IDENTIFIER, STR_NON_HEAP("i"), 76, 14),
        create(ASSIGN, create_null_str(), 76, 16),
        create_tok_int(create_int_value(INT_VALUE_I, 0), 76, 18),
        create(COMMA, create_null_str(), 76, 19),
        create(IDENTIFIER, STR_NON_HEAP("j"), 76, 21),
        create(ASSIGN, create_null_str(), 76, 23),
        create(IDENTIFIER, STR_NON_HEAP("LIMIT"), 76, 25),
        create(SEMICOLON, create_null_str(), 76, 30),
        create(IDENTIFIER, STR_NON_HEAP("i"), 76, 32),
        create(LT, create_null_str(), 76, 34),
        create(IDENTIFIER, STR_NON_HEAP("LIMIT"), 76, 36),
        create(AND_OP, create_null_str(), 76, 42),
        create(IDENTIFIER, STR_NON_HEAP("j"), 76, 45),
        create(GT, create_null_str(), 76, 47),
        create_tok_int(create_int_value(INT_VALUE_I, 0), 76, 49),
        create(SEMICOLON, create_null_str(), 76, 50),
        create(INC_OP, create_null_str(), 76, 52),
        create(IDENTIFIER, STR_NON_HEAP("i"), 76, 54),
        create(COMMA, create_null_str(), 76, 55),
        create(IDENTIFIER, STR_NON_HEAP("j"), 76, 57),
        create(DEC_OP, create_null_str(), 76, 58),
        create(RBRACKET, create_null_str(), 76, 60),
        create(LBRACE, create_null_str(), 76, 62),
        create(IDENTIFIER, STR_NON_HEAP("arr"), 77, 9),
        create(LINDEX, create_null_str(), 77, 12),
        create(IDENTIFIER, STR_NON_HEAP("i"), 77, 13),
        create(RINDEX, create_null_str(), 77, 14),
        create(ASSIGN, create_null_str(), 77, 16),
        create(LBRACKET, create_null_str(), 77, 18),
        create(CHAR, create_null_str(), 77, 19),
        create(RBRACKET, create_null_str(), 77, 23),
        create(IDENTIFIER, STR_NON_HEAP("j"), 77, 24),
        create(OR, create_null_str(), 77, 26),
        create_tok_int(create_int_value(INT_VALUE_I, 5), 77, 28),
        create(SEMICOLON, create_null_str(), 77, 29),
        create(IDENTIFIER, STR_NON_HEAP("arr"), 78, 9),
        create(LINDEX, create_null_str(), 78, 12),
        create(IDENTIFIER, STR_NON_HEAP("j"), 78, 13),
        create(RINDEX, create_null_str(), 78, 14),
        create(ASSIGN, create_null_str(), 78, 16),
        create(LBRACKET, create_null_str(), 78, 18),
        create(CHAR, create_null_str(), 78, 19),
        create(RBRACKET, create_null_str(), 78, 23),
        create(IDENTIFIER, STR_NON_HEAP("i"), 78, 24),
        create(DIV, create_null_str(), 78, 26),
        create_tok_int(create_int_value(INT_VALUE_I, 4), 78, 28),
        create(XOR, create_null_str(), 78, 30),
        create(BNOT, create_null_str(), 78, 32),
        create(IDENTIFIER, STR_NON_HEAP("size"), 78, 33),
        create(SEMICOLON, create_null_str(), 78, 37),
        create(RBRACE, create_null_str(), 79, 5),
        create(INT, create_null_str(), 81, 5),
        create(IDENTIFIER, STR_NON_HEAP("i"), 81, 9),
        create(ASSIGN, create_null_str(), 81, 11),
        create_tok_int(create_int_value(INT_VALUE_I, 0), 81, 13),
        create(SEMICOLON, create_null_str(), 81, 14),
        create(INT, create_null_str(), 82, 5),
        create(IDENTIFIER, STR_NON_HEAP("victim"), 82, 9),
        create(ASSIGN, create_null_str(), 82, 16),
        create_tok_int(create_uint_value(INT_VALUE_UI, 0xdeadbeef), 82, 18),
        create(SEMICOLON, create_null_str(), 82, 28),
        create(DO, create_null_str(), 83, 5),
        create(LBRACE, create_null_str(), 83, 8),
        create(IF, create_null_str(), 84, 9),
        create(LBRACKET, create_null_str(), 84, 12),
        create(IDENTIFIER, STR_NON_HEAP("i"), 84, 13),
        create(EQ_OP, create_null_str(), 84, 15),
        create_tok_int(create_int_value(INT_VALUE_I, 40), 84, 18),
        create(RBRACKET, create_null_str(), 84, 20),
        create(LBRACE, create_null_str(), 84, 22),
        create(CONTINUE, create_null_str(), 85, 13),
        create(SEMICOLON, create_null_str(), 85, 21),
        create(RBRACE, create_null_str(), 86, 9),
        create(ELSE, create_null_str(), 86, 11),
        create(IF, create_null_str(), 86, 16),
        create(LBRACKET, create_null_str(), 86, 19),
        create(IDENTIFIER, STR_NON_HEAP("i"), 86, 20),
        create(EQ_OP, create_null_str(), 86, 22),
        create_tok_int(create_int_value(INT_VALUE_I, 1000), 86, 25),
        create(RBRACKET, create_null_str(), 86, 29),
        create(LBRACE, create_null_str(), 86, 31),
        create(BREAK, create_null_str(), 87, 13),
        create(SEMICOLON, create_null_str(), 87, 18),
        create(RBRACE, create_null_str(), 88, 9),
        create(ELSE, create_null_str(), 88, 11),
        create(LBRACE, create_null_str(), 88, 16),
        create(INC_OP, create_null_str(), 89, 13),
        create(IDENTIFIER, STR_NON_HEAP("i"), 89, 15),
        create(SEMICOLON, create_null_str(), 89, 16),
        create(RBRACE, create_null_str(), 90, 9),
        create(IDENTIFIER, STR_NON_HEAP("victim"), 91, 9),
        create(ADD_ASSIGN, create_null_str(), 91, 16),
        create(IDENTIFIER, STR_NON_HEAP("i"), 91, 19),
        create(SEMICOLON, create_null_str(), 91, 20),
        create(IDENTIFIER, STR_NON_HEAP("victim"), 92, 9),
        create(SUB_ASSIGN, create_null_str(), 92, 16),
        create(IDENTIFIER, STR_NON_HEAP("LIMIT"), 92, 19),
        create(SEMICOLON, create_null_str(), 92, 24),
        create(IDENTIFIER, STR_NON_HEAP("victim"), 93, 9),
        create(DIV_ASSIGN, create_null_str(), 93, 16),
        create(LBRACKET, create_null_str(), 93, 19),
        create(INT, create_null_str(), 93, 20),
        create(RBRACKET, create_null_str(), 93, 23),
        create(IDENTIFIER, STR_NON_HEAP("c"), 93, 24),
        create(SEMICOLON, create_null_str(), 93, 25),
        create(IDENTIFIER, STR_NON_HEAP("d"), 94, 9),
        create(MUL_ASSIGN, create_null_str(), 94, 11),
        create(IDENTIFIER, STR_NON_HEAP("victim"), 94, 14),
        create(SEMICOLON, create_null_str(), 94, 20),
        create(IDENTIFIER, STR_NON_HEAP("type_size"), 95, 9),
        create(MOD_ASSIGN, create_null_str(), 95, 19),
        create(IDENTIFIER, STR_NON_HEAP("size"), 95, 22),
        create(SEMICOLON, create_null_str(), 95, 26),
        create(IDENTIFIER, STR_NON_HEAP("true_bool"), 96, 9),
        create(LEFT_ASSIGN, create_null_str(), 96, 19),
        create_tok_int(create_int_value(INT_VALUE_I, 4), 96, 23),
        create(SEMICOLON, create_null_str(), 96, 24),
        create(IDENTIFIER, STR_NON_HEAP("true_bool"), 97, 9),
        create(RIGHT_ASSIGN, create_null_str(), 97, 19),
        create_tok_int(create_int_value(INT_VALUE_I, 5), 97, 23),
        create(SEMICOLON, create_null_str(), 97, 24),
        create(IDENTIFIER, STR_NON_HEAP("arr"), 98, 9),
        create(LINDEX, create_null_str(), 98, 12),
        create(IDENTIFIER, STR_NON_HEAP("i"), 98, 13),
        create(RINDEX, create_null_str(), 98, 14),
        create(AND_ASSIGN, create_null_str(), 98, 16),
        create_tok_int(create_int_value(INT_VALUE_I, 0x2341), 98, 19),
        create(SEMICOLON, create_null_str(), 98, 25),
        create(IDENTIFIER, STR_NON_HEAP("arr"), 99, 9),
        create(LINDEX, create_null_str(), 99, 12),
        create(IDENTIFIER, STR_NON_HEAP("i"), 99, 13),
        create(ADD, create_null_str(), 99, 15),
        create_tok_int(create_int_value(INT_VALUE_I, 1), 99, 17),
        create(RINDEX, create_null_str(), 99, 18),
        create(OR_ASSIGN, create_null_str(), 99, 20),
        create_tok_int(create_int_value(INT_VALUE_I, 1), 99, 23),
        create(SEMICOLON, create_null_str(), 99, 24),
        create(IDENTIFIER, STR_NON_HEAP("arr"), 100, 9),
        create(LINDEX, create_null_str(), 100, 12),
        create_tok_int(create_int_value(INT_VALUE_I, 0), 100, 13),
        create(RINDEX, create_null_str(), 100, 14),
        create(XOR_ASSIGN, create_null_str(), 100, 16),
        create_tok_int(create_int_value(INT_VALUE_I, 10423), 100, 19),
        create(SEMICOLON, create_null_str(), 100, 24),
        create(RBRACE, create_null_str(), 101, 5),
        create(WHILE, create_null_str(), 101, 7),
        create(LBRACKET, create_null_str(), 101, 13),
        create_tok_int(create_int_value(INT_VALUE_I, 1), 101, 14),
        create(RBRACKET, create_null_str(), 101, 15),
        create(SEMICOLON, create_null_str(), 101, 16),
        create(WHILE, create_null_str(), 103, 5),
        create(LBRACKET, create_null_str(), 103, 11),
        create_tok_int(create_int_value(INT_VALUE_I, 1), 103, 12),
        create(RBRACKET, create_null_str(), 103, 13),
        create(LBRACE, create_null_str(), 103, 15),
        create(BREAK, create_null_str(), 104, 9),
        create(SEMICOLON, create_null_str(), 104, 14),
        create(RBRACE, create_null_str(), 105, 5),
        create(RETURN, create_null_str(), 107, 5),
        create(LBRACKET, create_null_str(), 107, 12),
        create(IDENTIFIER, STR_NON_HEAP("arr"), 107, 13),
        create(LINDEX, create_null_str(), 107, 16),
        create_tok_int(create_int_value(INT_VALUE_I, 0), 107, 17),
        create(RINDEX, create_null_str(), 107, 18),
        create(EQ_OP, create_null_str(), 107, 20),
        create_tok_int(create_int_value(INT_VALUE_I, 37), 107, 23),
        create(QMARK, create_null_str(), 107, 26),
        create(IDENTIFIER, STR_NON_HEAP("size"), 107, 28),
        create(COLON, create_null_str(), 107, 33),
        create(LBRACKET, create_null_str(), 107, 35),
        create(IDENTIFIER, STR_NON_HEAP("arr"), 107, 36),
        create(LINDEX, create_null_str(), 107, 39),
        create_tok_int(create_int_value(INT_VALUE_I, 1), 107, 40),
        create(RINDEX, create_null_str(), 107, 41),
        create(EQ_OP, create_null_str(), 107, 43),
        create_tok_int(create_int_value(INT_VALUE_I, 37), 107, 46),
        create(QMARK, create_null_str(), 107, 49),
        create(IDENTIFIER, STR_NON_HEAP("type_size"), 107, 51),
        create(COLON, create_null_str(), 107, 61),
        create(SUB, create_null_str(), 107, 63),
        create_tok_int(create_int_value(INT_VALUE_I, 123123), 107, 64),
        create(RIGHT_OP, create_null_str(), 107, 71),
        create_tok_int(create_int_value(INT_VALUE_I, 4), 107, 74),
        create(RBRACKET, create_null_str(), 107, 75),
        create(RBRACKET, create_null_str(), 107, 76),
        create(SEMICOLON, create_null_str(), 107, 77),
        create(RBRACE, create_null_str(), 108, 1),
        create(NORETURN, create_null_str(), 110, 1),
        create(STATIC, create_null_str(), 110, 11),
        create(VOID, create_null_str(), 110, 18),
        create(IDENTIFIER, STR_NON_HEAP("variadic"), 110, 23),
        create(LBRACKET, create_null_str(), 110, 31),
        create(INT, create_null_str(), 110, 32),
        create(IDENTIFIER, STR_NON_HEAP("m"), 110, 36),
        create(COMMA, create_null_str(), 110, 37),
        create(ELLIPSIS, create_null_str(), 110, 39),
        create(RBRACKET, create_null_str(), 110, 42),
        create(LBRACE, create_null_str(), 110, 44),
        create(UNSIGNED, create_null_str(), 111, 5),
        create(CHAR, create_null_str(), 111, 14),
        create(IDENTIFIER, STR_NON_HEAP("c"), 111, 19),
        create(ASSIGN, create_null_str(), 111, 21),
        create_tok_int(create_int_value(INT_VALUE_I, '\n'), 111, 23),
        create(SEMICOLON, create_null_str(), 111, 27),
        create(CONST, create_null_str(), 113, 5),
        create(CHAR, create_null_str(), 113, 11),
        create(ASTERISK, create_null_str(), 113, 15),
        create(IDENTIFIER, STR_NON_HEAP("func_name"), 113, 17),
        create(ASSIGN, create_null_str(), 113, 27),
        create(FUNC_NAME, create_null_str(), 113, 29),
        create(SEMICOLON, create_null_str(), 113, 37),
        create(DOUBLE, create_null_str(), 114, 5),
        create(COMPLEX, create_null_str(), 114, 12),
        create(IDENTIFIER, STR_NON_HEAP("comp_d"), 114, 21),
        create(ASSIGN, create_null_str(), 114, 28),
        create_tok_int(create_int_value(INT_VALUE_I, 0), 114, 30),
        create(SEMICOLON, create_null_str(), 114, 31),
        create(DOUBLE, create_null_str(), 115, 5),
        create(IMAGINARY, create_null_str(), 115, 12),
        create(IDENTIFIER, STR_NON_HEAP("im_d"), 115, 23),
        create(ASSIGN, create_null_str(), 115, 28),
        create(GENERIC, create_null_str(), 115, 30),
        create(LBRACKET, create_null_str(), 115, 38),
        create_tok_float(create_float_value(FLOAT_VALUE_D, 1.0), 115, 39),
        create(COMMA, create_null_str(), 115, 42),
        create(FLOAT, create_null_str(), 115, 44),
        create(COLON, create_null_str(), 115, 49),
        create_tok_float(create_float_value(FLOAT_VALUE_F, 0x1p+2f), 115, 51),
        create(COMMA, create_null_str(), 115, 58),
        create(DOUBLE, create_null_str(), 115, 60),
        create(COLON, create_null_str(), 115, 66),
        create_tok_float(create_float_value(FLOAT_VALUE_LD, 0X12.0P-10L),
                         115,
                         68),
        create(RBRACKET, create_null_str(), 115, 79),
        create(SEMICOLON, create_null_str(), 115, 80),
        create(STATIC_ASSERT, create_null_str(), 116, 5),
        create(LBRACKET, create_null_str(), 116, 19),
        create_tok_int(create_int_value(INT_VALUE_I, 1), 116, 20),
        create(COMMA, create_null_str(), 116, 21),
        create(STRING_LITERAL, STR_NON_HEAP("\"Something is wrong\""), 116, 23),
        create(RBRACKET, create_null_str(), 116, 43),
        create(SEMICOLON, create_null_str(), 116, 44),
        create(RETURN, create_null_str(), 117, 5),
        create(SEMICOLON, create_null_str(), 117, 11),
        create(CHAR, create_null_str(), 118, 5),
        create(IDENTIFIER, STR_NON_HEAP("d"), 118, 10),
        create(ASSIGN, create_null_str(), 118, 12),
        create_tok_int(create_int_value(INT_VALUE_I, '\\'), 118, 14),
        create(COMMA, create_null_str(), 118, 18),
        create(IDENTIFIER, STR_NON_HEAP("e"), 118, 20),
        create(ASSIGN, create_null_str(), 118, 22),
        create_tok_int(create_int_value(INT_VALUE_I, '\''), 118, 24),
        create(COMMA, create_null_str(), 118, 28),
        create(IDENTIFIER, STR_NON_HEAP("f"), 118, 30),
        create(ASSIGN, create_null_str(), 118, 32),
        create_tok_int(create_int_value(INT_VALUE_I, '"'), 118, 34),
        create(COMMA, create_null_str(), 118, 37),
        create(IDENTIFIER, STR_NON_HEAP("g"), 118, 39),
        create(ASSIGN, create_null_str(), 118, 41),
        create_tok_int(create_int_value(INT_VALUE_I, '\0'), 118, 43),
        create(SEMICOLON, create_null_str(), 118, 47),
        create(INT, create_null_str(), 119, 5),
        create(IDENTIFIER, STR_NON_HEAP("h"), 119, 9),
        create(ASSIGN, create_null_str(), 119, 11),
        create_tok_int(create_int_value(INT_VALUE_I, 's'), 119, 13),
        create(SEMICOLON, create_null_str(), 119, 16),
        create(RBRACE, create_null_str(), 120, 1),
        create(THREAD_LOCAL, create_null_str(), 122, 1),
        create(INT, create_null_str(), 122, 15),
        create(IDENTIFIER, STR_NON_HEAP("g_thread"), 122, 19),
        create(SEMICOLON, create_null_str(), 122, 27),
    };
    check_token_arr_file(filename,
                         expected,
                         sizeof expected / sizeof *expected);
}

TEST(hex_literal_or_var) {
    {
        const char* code = "vare-10";

        const struct token expected[] = {
            create(IDENTIFIER, STR_NON_HEAP("vare"), 1, 1),
            create(SUB, create_null_str(), 1, 5),
            create_tok_int(create_int_value(INT_VALUE_I, 10), 1, 6),
        };
        check_token_arr_str(code, expected, sizeof expected / sizeof *expected);
    }
    {
        const char* code = "var2e-10";

        const struct token expected[] = {
            create(IDENTIFIER, STR_NON_HEAP("var2e"), 1, 1),
            create(SUB, create_null_str(), 1, 6),
            create_tok_int(create_int_value(INT_VALUE_I, 10), 1, 7),
        };
        check_token_arr_str(code, expected, sizeof expected / sizeof *expected);
    }
    {
        const char* code = "var2p-10";

        const struct token expected[] = {
            create(IDENTIFIER, STR_NON_HEAP("var2p"), 1, 1),
            create(SUB, create_null_str(), 1, 6),
            create_tok_int(create_int_value(INT_VALUE_I, 10), 1, 7),
        };
        check_token_arr_str(code, expected, sizeof expected / sizeof *expected);
    }
}

TEST(dot_float_literal_or_op) {
    {
        const char* code = "int n = .001";

        const struct token expected[] = {
            create(INT, create_null_str(), 1, 1),
            create(IDENTIFIER, STR_NON_HEAP("n"), 1, 5),
            create(ASSIGN, create_null_str(), 1, 7),
            create_tok_float(create_float_value(FLOAT_VALUE_D, .001), 1, 9),
        };
        check_token_arr_str(code, expected, sizeof expected / sizeof *expected);
    }
}

TEST_SUITE_BEGIN(tokenizer, 4) {
    REGISTER_TEST(simple);
    REGISTER_TEST(file);
    REGISTER_TEST(hex_literal_or_var);
    REGISTER_TEST(dot_float_literal_or_op);
}
TEST_SUITE_END()

static struct token create(enum token_type type,
                           struct str spelling,
                           size_t line,
                           size_t index) {
    return (struct token){
        .type = type,
        .spelling = spelling,
        .loc =
            {
                .file_idx = 0,
                .file_loc = {line, index},
            },
    };
}

static struct token create_tok_int(struct int_value val,
                                   size_t line,
                                   size_t index) {
    return (struct token){
        .type = I_CONSTANT,
        .int_val = val,
        .loc =
            {
                .file_idx = 0,
                .file_loc = {line, index},
            },
    };
}

static struct token create_tok_float(struct float_value val,
                                     size_t line,
                                     size_t index) {
    return (struct token){
        .type = F_CONSTANT,
        .float_val = val,
        .loc =
            {
                .file_idx = 0,
                .file_loc = {line, index},
            },
    };
}

static void check_size(const struct token* tokens, size_t expected) {
    size_t size = 0;
    const struct token* it = tokens;

    while (it->type != INVALID) {
        ++it;
        ++size;
    }
    ASSERT_SIZE_T(size, expected);
}

static void check_file(const struct token* tokens, size_t file_idx) {
    for (const struct token* it = tokens; it->type != INVALID; ++it) {
        ASSERT_SIZE_T(it->loc.file_idx, file_idx);
    }
}

static void check_token(const struct token* t, const struct token* expected) {
    ASSERT_TOKEN_TYPE(t->type, expected->type);

    if (t->type == I_CONSTANT) {
        ASSERT_INT_VALUE_TYPE(t->int_val.type, expected->int_val.type);
        if (int_value_is_signed(t->int_val.type)) {
            ASSERT_INTMAX_T(t->int_val.int_val, expected->int_val.int_val);
        } else {
            ASSERT_UINTMAX_T(t->int_val.uint_val, expected->int_val.uint_val);
        }
    } else if (t->type == F_CONSTANT) {
        ASSERT_FLOAT_VALUE_TYPE(t->float_val.type, expected->float_val.type);
        ASSERT_DOUBLE(t->float_val.val, expected->float_val.val, 0.0001);
    } else {
        ASSERT_STR(str_get_data(&t->spelling),
                   str_get_data(&expected->spelling));
    }

    ASSERT_SIZE_T(t->loc.file_loc.line, expected->loc.file_loc.line);
    ASSERT_SIZE_T(t->loc.file_loc.index, expected->loc.file_loc.index);
}

static void compare_tokens(const struct token* got,
                           const struct token* expected,
                           size_t len) {
    for (size_t i = 0; i < len; ++i) {
        check_token(&got[i], &expected[i]);
    }
}

static void check_token_arr_helper(const char* file_or_code,
                                   const struct token* expected,
                                   size_t expected_len,
                                   struct preproc_res (*func)(const char*)) {
    struct preproc_res preproc_res = func(file_or_code);
    ASSERT_NOT_NULL(preproc_res.toks);
    check_size(preproc_res.toks, expected_len);
    check_file(preproc_res.toks, 0);

    compare_tokens(preproc_res.toks, expected, expected_len);

    free_preproc_res(&preproc_res);
}

static void check_token_arr_file(const char* filename,
                                 const struct token* expected,
                                 size_t expected_len) {
    check_token_arr_helper(filename, expected, expected_len, tokenize);
}

static struct preproc_res tokenize_string_wrapper(const char* code) {
    return tokenize_string(code, "code.c");
}

static void check_token_arr_str(const char* code,
                                const struct token* expected,
                                size_t expected_len) {
    check_token_arr_helper(code,
                           expected,
                           expected_len,
                           tokenize_string_wrapper);
}
