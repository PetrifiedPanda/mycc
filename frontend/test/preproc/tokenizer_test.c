#include <stdio.h>
#include <string.h>

#include "testing/asserts.h"

#include "frontend/preproc/preproc.h"

#include "../test_helpers.h"

static struct token create(enum token_type type,
                           const char* spelling,
                           size_t line,
                           size_t index);
static struct token create_value(struct value val, size_t line, size_t index);

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
        create(TYPEDEF, NULL, 1, 1),
        create(STRUCT, NULL, 1, 9),
        create(IDENTIFIER, "typedeftest", 1, 16),
        create(LBRACE, NULL, 5, 1),
        create(LONG, NULL, 6, 2),
        create(INT, NULL, 6, 7),
        create(ASTERISK, NULL, 6, 10),
        create(IDENTIFIER, "n", 6, 12),
        create(SEMICOLON, NULL, 6, 13),
        create(CONST, NULL, 7, 1),
        create(LONG, NULL, 7, 7),
        create(DOUBLE, NULL, 7, 12),
        create(ASTERISK, NULL, 7, 19),
        create(IDENTIFIER, "m", 7, 20),
        create(SEMICOLON, NULL, 7, 21),
        create(RBRACE, NULL, 8, 1),
        create(IDENTIFIER, "Typedeftest", 8, 3),
        create(SEMICOLON, NULL, 8, 14),
        create(CONST, NULL, 9, 1),
        create(CHAR, NULL, 9, 7),
        create(ASTERISK, NULL, 9, 11),
        create(IDENTIFIER, "lstr", 9, 13),
        create(ASSIGN, NULL, 9, 18),
        create(STRING_LITERAL,
               "L\"Long string literal to check if long strings work\"",
               10,
               1),
        create(SEMICOLON, NULL, 10, 53),
        create(INT, NULL, 11, 1),
        create(IDENTIFIER, "n", 11, 5),
        create(ASSIGN, NULL, 11, 7),
        create_value(create_int_value(VALUE_INT, 0x123213), 11, 9),
        create(ADD, NULL, 11, 18),
        create_value(create_int_value(VALUE_INT, 132), 11, 20),
        create(LEFT_OP, NULL, 11, 24),
        create_value(create_int_value(VALUE_INT, 32), 11, 27),
        create(RIGHT_OP, NULL, 11, 30),
        create_value(create_int_value(VALUE_INT, 0x123), 11, 33),
        create(SUB, NULL, 11, 39),
        create_value(create_int_value(VALUE_INT, 0123), 11, 41),
        create(DIV, NULL, 11, 46),
        create_value(create_int_value(VALUE_INT, 12), 11, 48),
        create(SEMICOLON, NULL, 11, 50),
        create(CONST, NULL, 12, 1),
        create(CHAR, NULL, 12, 7),
        create(ASTERISK, NULL, 12, 11),
        create(IDENTIFIER, "str", 12, 13),
        create(ASSIGN, NULL, 12, 17),
        create(STRING_LITERAL, "\"Normal string literal\"", 12, 19),
        create(SEMICOLON, NULL, 12, 42),
        create(INT, NULL, 13, 1),
        create(IDENTIFIER, "arr", 13, 5),
        create(LINDEX, NULL, 13, 8),
        create_value(create_int_value(VALUE_INT, 1), 13, 9),
        create(QMARK, NULL, 13, 11),
        create_value(create_int_value(VALUE_INT, 100), 13, 13),
        create(COLON, NULL, 13, 17),
        create_value(create_int_value(VALUE_INT, 1000), 13, 19),
        create(RINDEX, NULL, 13, 23),
        create(SEMICOLON, NULL, 13, 24),
    };
    check_token_arr_str(code, expected, sizeof expected / sizeof *expected);
}

TEST(file) {
    const char* filename = "../frontend/test/files/no_preproc.c";

    const struct token expected[] = {
        create(TYPEDEF, NULL, 3, 1),
        create(STRUCT, NULL, 3, 9),
        create(LBRACE, NULL, 3, 16),
        create(ATOMIC, NULL, 4, 5),
        create(VOLATILE, NULL, 4, 13),
        create(INT, NULL, 4, 22),
        create(ASTERISK, NULL, 4, 25),
        create(RESTRICT, NULL, 4, 27),
        create(IDENTIFIER, "ptr", 4, 36),
        create(SEMICOLON, NULL, 4, 39),
        create(ALIGNAS, NULL, 5, 5),
        create(LBRACKET, NULL, 5, 13),
        create_value(create_int_value(VALUE_INT, 16), 5, 14),
        create(RBRACKET, NULL, 5, 16),
        create(CONST, NULL, 5, 18),
        create(CHAR, NULL, 5, 24),
        create(ASTERISK, NULL, 5, 29),
        create(IDENTIFIER, "str", 5, 31),
        create(SEMICOLON, NULL, 5, 34),
        create(RBRACE, NULL, 6, 1),
        create(IDENTIFIER, "MyStruct", 6, 3),
        create(SEMICOLON, NULL, 6, 11),
        create(UNION, NULL, 8, 1),
        create(IDENTIFIER, "my_union", 8, 7),
        create(LBRACE, NULL, 8, 16),
        create(SHORT, NULL, 9, 5),
        create(INT, NULL, 9, 11),
        create(IDENTIFIER, "i", 9, 15),
        create(COLON, NULL, 9, 17),
        create_value(create_int_value(VALUE_INT, 4), 9, 19),
        create(COMMA, NULL, 9, 20),
        create(COLON, NULL, 9, 22),
        create_value(create_int_value(VALUE_INT, 4), 9, 24),
        create(SEMICOLON, NULL, 9, 25),
        create(ALIGNAS, NULL, 10, 5),
        create(LBRACKET, NULL, 10, 13),
        create(CONST, NULL, 10, 14),
        create(IDENTIFIER, "MyStruct", 10, 20),
        create(RBRACKET, NULL, 10, 28),
        create(FLOAT, NULL, 10, 30),
        create(IDENTIFIER, "f", 10, 36),
        create(SEMICOLON, NULL, 10, 37),
        create(RBRACE, NULL, 11, 1),
        create(SEMICOLON, NULL, 11, 2),
        create(ENUM, NULL, 14, 1),
        create(IDENTIFIER, "my_enum", 14, 6),
        create(LBRACE, NULL, 14, 14),
        create(IDENTIFIER, "VAL_1", 15, 5),
        create(COMMA, NULL, 15, 10),
        create(IDENTIFIER, "VAL_2", 16, 5),
        create(COMMA, NULL, 16, 10),
        create(IDENTIFIER, "VAL_3", 17, 5),
        create(RBRACE, NULL, 18, 1),
        create(SEMICOLON, NULL, 18, 2),
        create(STATIC, NULL, 20, 1),
        create(INLINE, NULL, 20, 8),
        create(INT, NULL, 20, 15),
        create(IDENTIFIER, "do_shit", 20, 19),
        create(LBRACKET, NULL, 20, 26),
        create(INT, NULL, 20, 27),
        create(IDENTIFIER, "n", 20, 31),
        create(COMMA, NULL, 20, 32),
        create(INT, NULL, 20, 34),
        create(IDENTIFIER, "m", 20, 38),
        create(RBRACKET, NULL, 20, 39),
        create(SEMICOLON, NULL, 20, 40),
        create(STATIC, NULL, 22, 1),
        create(VOID, NULL, 22, 8),
        create(IDENTIFIER, "variadic", 22, 13),
        create(LBRACKET, NULL, 22, 21),
        create(INT, NULL, 22, 22),
        create(IDENTIFIER, "m", 22, 26),
        create(COMMA, NULL, 22, 27),
        create(ELLIPSIS, NULL, 22, 29),
        create(RBRACKET, NULL, 22, 32),
        create(SEMICOLON, NULL, 22, 33),
        create(EXTERN, NULL, 24, 1),
        create(INT, NULL, 24, 8),
        create(IDENTIFIER, "some_func", 24, 12),
        create(LBRACKET, NULL, 24, 21),
        create(RBRACKET, NULL, 24, 22),
        create(SEMICOLON, NULL, 24, 23),
        create(INT, NULL, 26, 1),
        create(IDENTIFIER, "main", 26, 5),
        create(LBRACKET, NULL, 26, 9),
        create(RBRACKET, NULL, 26, 10),
        create(LBRACE, NULL, 26, 12),
        create(REGISTER, NULL, 27, 5),
        create(ENUM, NULL, 27, 14),
        create(IDENTIFIER, "my_enum", 27, 19),
        create(IDENTIFIER, "type", 27, 27),
        create(ASSIGN, NULL, 27, 32),
        create(IDENTIFIER, "VAL_1", 27, 34),
        create(SEMICOLON, NULL, 27, 39),
        create(AUTO, NULL, 29, 5),
        create(LONG, NULL, 29, 10),
        create(SIGNED, NULL, 29, 15),
        create(INT, NULL, 29, 22),
        create(IDENTIFIER, "value", 29, 26),
        create(SEMICOLON, NULL, 29, 31),
        create(SWITCH, NULL, 30, 5),
        create(LBRACKET, NULL, 30, 12),
        create(IDENTIFIER, "type", 30, 13),
        create(RBRACKET, NULL, 30, 17),
        create(LBRACE, NULL, 30, 19),
        create(CASE, NULL, 31, 9),
        create(IDENTIFIER, "VAL_1", 31, 14),
        create(COLON, NULL, 31, 19),
        create(CASE, NULL, 32, 9),
        create(IDENTIFIER, "VAL_2", 32, 14),
        create(COLON, NULL, 32, 19),
        create(IDENTIFIER, "value", 33, 13),
        create(ASSIGN, NULL, 33, 19),
        create_value(create_int_value(VALUE_LINT, 1000l), 33, 21),
        create(MOD, NULL, 33, 27),
        create_value(create_int_value(VALUE_INT, 5), 33, 29),
        create(SEMICOLON, NULL, 33, 30),
        create(BREAK, NULL, 34, 13),
        create(SEMICOLON, NULL, 34, 18),
        create(DEFAULT, NULL, 36, 9),
        create(COLON, NULL, 36, 16),
        create(IDENTIFIER, "value", 37, 13),
        create(ASSIGN, NULL, 37, 19),
        create_value(create_int_value(VALUE_LINT, 30l), 37, 21),
        create(SEMICOLON, NULL, 37, 24),
        create(BREAK, NULL, 38, 13),
        create(SEMICOLON, NULL, 38, 18),
        create(RBRACE, NULL, 39, 5),
        create(IDENTIFIER, "MyStruct", 46, 5),
        create(IDENTIFIER, "s", 46, 14),
        create(ASSIGN, NULL, 46, 16),
        create(LBRACE, NULL, 46, 18),
        create_value(create_int_value(VALUE_INT, 0), 46, 19),
        create(COMMA, NULL, 46, 20),
        create(
            STRING_LITERAL,
            "L\"Hello there, this string literal needs to be longer than 512 "
            "characters oh no I don't know what to write here "
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaa\"",
            46,
            22),
        create(RBRACE, NULL, 46, 1204),
        create(SEMICOLON, NULL, 46, 1205),
        create(INT, NULL, 47, 5),
        create(IDENTIFIER, "integer", 47, 9),
        create(ASSIGN, NULL, 47, 17),
        create_value(create_int_value(VALUE_INT, 01000), 47, 19),
        create(LEFT_OP, NULL, 47, 25),
        create_value(create_int_value(VALUE_INT, 10), 47, 28),
        create(SEMICOLON, NULL, 47, 30),
        create(IDENTIFIER, "s", 48, 5),
        create(DOT, NULL, 48, 6),
        create(IDENTIFIER, "ptr", 48, 7),
        create(ASSIGN, NULL, 48, 11),
        create(AND, NULL, 48, 13),
        create(IDENTIFIER, "integer", 48, 14),
        create(SEMICOLON, NULL, 48, 21),
        create(IDENTIFIER, "MyStruct", 50, 5),
        create(ASTERISK, NULL, 50, 14),
        create(IDENTIFIER, "s_ptr", 50, 15),
        create(ASSIGN, NULL, 50, 21),
        create(AND, NULL, 50, 23),
        create(IDENTIFIER, "s", 50, 24),
        create(SEMICOLON, NULL, 50, 25),
        create(IDENTIFIER, "s_ptr", 51, 5),
        create(PTR_OP, NULL, 51, 10),
        create(IDENTIFIER, "str", 51, 12),
        create(ASSIGN, NULL, 51, 16),
        create(STRING_LITERAL, "\"Goodbye\"", 51, 18),
        create(SEMICOLON, NULL, 51, 27),
        create(ASTERISK, NULL, 52, 5),
        create(IDENTIFIER, "s_ptr", 52, 6),
        create(ASSIGN, NULL, 52, 12),
        create(LBRACKET, NULL, 52, 14),
        create(IDENTIFIER, "MyStruct", 52, 15),
        create(RBRACKET, NULL, 52, 23),
        create(LBRACE, NULL, 52, 24),
        create(STRING_LITERAL,
               "L\"\\\"Lstrings seem to be int pointers\\\"\"",
               52,
               25),
        create(COMMA, NULL, 52, 64),
        create(STRING_LITERAL, "\"doot\"", 52, 66),
        create(RBRACE, NULL, 52, 72),
        create(SEMICOLON, NULL, 52, 73),
        create(UNION, NULL, 54, 5),
        create(IDENTIFIER, "my_union", 54, 11),
        create(IDENTIFIER, "soviet_union", 54, 20),
        create(SEMICOLON, NULL, 54, 32),
        create(IDENTIFIER, "soviet_union", 55, 5),
        create(DOT, NULL, 55, 17),
        create(IDENTIFIER, "i", 55, 18),
        create(ASSIGN, NULL, 55, 20),
        create_value(create_int_value(VALUE_INT, 0x1000), 55, 22),
        create(ADD, NULL, 55, 29),
        create_value(create_int_value(VALUE_INT, 033242), 55, 31),
        create(SEMICOLON, NULL, 55, 37),
        create(INT, NULL, 56, 5),
        create(
            IDENTIFIER,
            "super_long_identifier_that_needs_to_be_over_512_characters_long_"
            "what_the_hell_am_i_supposed_to_write_here_a_b_c_d_e_f_g_h_i_j_k_l_"
            "m_n_o_p_q_r_s_t_u_v_w_x_y_z_"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaacccccccccccccccccccccccc"
            "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
            "ccccccccccccccccccccccccccccccccccccccccccccccccccccoooooooooooooo"
            "ooooooooooooooooooooooooooooooosoooooooooooooooooooooooooooodfsooo"
            "ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo_"
            "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
            "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
            "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
            "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
            "ssssssssssssssssssssssssssssssssssssssssssssssssssssss",
            56,
            9),
        create(SEMICOLON, NULL, 56, 1141),
        create(GOTO, NULL, 57, 5),
        create(IDENTIFIER, "my_cool_label", 57, 10),
        create(SEMICOLON, NULL, 57, 23),
        create(RETURN, NULL, 59, 5),
        create(ALIGNOF, NULL, 59, 12),
        create(LBRACKET, NULL, 59, 20),
        create(LONG, NULL, 59, 21),
        create(RBRACKET, NULL, 59, 25),
        create(SEMICOLON, NULL, 59, 26),
        create(IDENTIFIER, "my_cool_label", 61, 1),
        create(COLON, NULL, 61, 14),
        create(
            IDENTIFIER,
            "super_long_identifier_that_needs_to_be_over_512_characters_long_"
            "what_the_hell_am_i_supposed_to_write_here_a_b_c_d_e_f_g_h_i_j_k_l_"
            "m_n_o_p_q_r_s_t_u_v_w_x_y_z_"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaacccccccccccccccccccccccc"
            "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
            "ccccccccccccccccccccccccccccccccccccccccccccccccccccoooooooooooooo"
            "ooooooooooooooooooooooooooooooosoooooooooooooooooooooooooooodfsooo"
            "ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo_"
            "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
            "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
            "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
            "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
            "ssssssssssssssssssssssssssssssssssssssssssssssssssssss",
            62,
            5),
        create(ASSIGN, NULL, 62, 1138),
        create(LBRACKET, NULL, 62, 1140),
        create(INT, NULL, 62, 1141),
        create(RBRACKET, NULL, 62, 1144),
        create(IDENTIFIER, "soviet_union", 62, 1145),
        create(DOT, NULL, 62, 1157),
        create(IDENTIFIER, "f", 62, 1158),
        create(SEMICOLON, NULL, 62, 1159),
        create(RBRACE, NULL, 63, 1),
        create(STATIC, NULL, 65, 1),
        create(INT, NULL, 65, 8),
        create(IDENTIFIER, "do_shit", 65, 12),
        create(LBRACKET, NULL, 65, 19),
        create(IDENTIFIER, "n", 65, 20),
        create(COMMA, NULL, 65, 21),
        create(IDENTIFIER, "m", 65, 23),
        create(RBRACKET, NULL, 65, 24),
        create(INT, NULL, 65, 26),
        create(IDENTIFIER, "n", 65, 30),
        create(SEMICOLON, NULL, 65, 31),
        create(INT, NULL, 65, 33),
        create(IDENTIFIER, "m", 65, 37),
        create(SEMICOLON, NULL, 65, 38),
        create(LBRACE, NULL, 65, 40),
        create(DOUBLE, NULL, 66, 5),
        create(IDENTIFIER, "d", 66, 12),
        create(ASSIGN, NULL, 66, 14),
        create_value(create_float_value(VALUE_DOUBLE, 1e-10), 66, 16),
        create(SUB, NULL, 66, 22),
        create_value(create_float_value(VALUE_DOUBLE, 0xabecp10), 66, 24),
        create(SEMICOLON, NULL, 66, 33),
        create(INT, NULL, 67, 5),
        create(IDENTIFIER, "type_size", 67, 9),
        create(ASSIGN, NULL, 67, 19),
        create(SIZEOF, NULL, 67, 21),
        create(LBRACKET, NULL, 67, 27),
        create(DOUBLE, NULL, 67, 28),
        create(RBRACKET, NULL, 67, 34),
        create(SEMICOLON, NULL, 67, 35),
        create(INT, NULL, 68, 5),
        create(IDENTIFIER, "size", 68, 9),
        create(ASSIGN, NULL, 68, 14),
        create(SIZEOF, NULL, 68, 16),
        create(IDENTIFIER, "d", 68, 23),
        create(SEMICOLON, NULL, 68, 24),
        create(CHAR, NULL, 70, 5),
        create(IDENTIFIER, "c", 70, 10),
        create(ASSIGN, NULL, 70, 12),
        create_value(create_int_value(VALUE_INT, '\n'), 70, 14),
        create(SEMICOLON, NULL, 70, 18),
        create(BOOL, NULL, 72, 5),
        create(IDENTIFIER, "true_bool", 72, 11),
        create(ASSIGN, NULL, 72, 21),
        create(NOT, NULL, 72, 23),
        create(LBRACKET, NULL, 72, 24),
        create(IDENTIFIER, "c", 72, 25),
        create(NE_OP, NULL, 72, 27),
        create(IDENTIFIER, "c", 72, 30),
        create(RBRACKET, NULL, 72, 31),
        create(AND_OP, NULL, 72, 33),
        create(LBRACKET, NULL, 72, 36),
        create(IDENTIFIER, "c", 72, 37),
        create(LE_OP, NULL, 72, 39),
        create(IDENTIFIER, "size", 72, 42),
        create(OR_OP, NULL, 72, 47),
        create(IDENTIFIER, "c", 72, 50),
        create(GE_OP, NULL, 72, 52),
        create(IDENTIFIER, "size", 72, 55),
        create(OR_OP, NULL, 72, 60),
        create(IDENTIFIER, "c", 72, 63),
        create(LT, NULL, 72, 65),
        create(IDENTIFIER, "size", 72, 67),
        create(OR_OP, NULL, 72, 72),
        create(IDENTIFIER, "c", 72, 75),
        create(GT, NULL, 72, 77),
        create(IDENTIFIER, "size", 72, 79),
        create(OR_OP, NULL, 72, 84),
        create(IDENTIFIER, "c", 72, 87),
        create(EQ_OP, NULL, 72, 89),
        create(IDENTIFIER, "size", 72, 92),
        create(OR_OP, NULL, 72, 97),
        create(IDENTIFIER, "c", 72, 100),
        create(NE_OP, NULL, 72, 102),
        create(IDENTIFIER, "size", 72, 105),
        create(RBRACKET, NULL, 72, 109),
        create(SEMICOLON, NULL, 72, 110),
        create(ENUM, NULL, 74, 5),
        create(LBRACE, NULL, 74, 10),
        create(IDENTIFIER, "LIMIT", 74, 12),
        create(ASSIGN, NULL, 74, 18),
        create_value(create_int_value(VALUE_INT, 50000), 74, 20),
        create(RBRACE, NULL, 74, 26),
        create(SEMICOLON, NULL, 74, 27),
        create(CHAR, NULL, 75, 5),
        create(IDENTIFIER, "arr", 75, 10),
        create(LINDEX, NULL, 75, 13),
        create(IDENTIFIER, "LIMIT", 75, 14),
        create(RINDEX, NULL, 75, 19),
        create(ASSIGN, NULL, 75, 21),
        create(LBRACE, NULL, 75, 23),
        create_value(create_int_value(VALUE_INT, 0), 75, 24),
        create(RBRACE, NULL, 75, 25),
        create(SEMICOLON, NULL, 75, 26),
        create(FOR, NULL, 76, 5),
        create(LBRACKET, NULL, 76, 9),
        create(INT, NULL, 76, 10),
        create(IDENTIFIER, "i", 76, 14),
        create(ASSIGN, NULL, 76, 16),
        create_value(create_int_value(VALUE_INT, 0), 76, 18),
        create(COMMA, NULL, 76, 19),
        create(IDENTIFIER, "j", 76, 21),
        create(ASSIGN, NULL, 76, 23),
        create(IDENTIFIER, "LIMIT", 76, 25),
        create(SEMICOLON, NULL, 76, 30),
        create(IDENTIFIER, "i", 76, 32),
        create(LT, NULL, 76, 34),
        create(IDENTIFIER, "LIMIT", 76, 36),
        create(AND_OP, NULL, 76, 42),
        create(IDENTIFIER, "j", 76, 45),
        create(GT, NULL, 76, 47),
        create_value(create_int_value(VALUE_INT, 0), 76, 49),
        create(SEMICOLON, NULL, 76, 50),
        create(INC_OP, NULL, 76, 52),
        create(IDENTIFIER, "i", 76, 54),
        create(COMMA, NULL, 76, 55),
        create(IDENTIFIER, "j", 76, 57),
        create(DEC_OP, NULL, 76, 58),
        create(RBRACKET, NULL, 76, 60),
        create(LBRACE, NULL, 76, 62),
        create(IDENTIFIER, "arr", 77, 9),
        create(LINDEX, NULL, 77, 12),
        create(IDENTIFIER, "i", 77, 13),
        create(RINDEX, NULL, 77, 14),
        create(ASSIGN, NULL, 77, 16),
        create(LBRACKET, NULL, 77, 18),
        create(CHAR, NULL, 77, 19),
        create(RBRACKET, NULL, 77, 23),
        create(IDENTIFIER, "j", 77, 24),
        create(OR, NULL, 77, 26),
        create_value(create_int_value(VALUE_INT, 5), 77, 28),
        create(SEMICOLON, NULL, 77, 29),
        create(IDENTIFIER, "arr", 78, 9),
        create(LINDEX, NULL, 78, 12),
        create(IDENTIFIER, "j", 78, 13),
        create(RINDEX, NULL, 78, 14),
        create(ASSIGN, NULL, 78, 16),
        create(LBRACKET, NULL, 78, 18),
        create(CHAR, NULL, 78, 19),
        create(RBRACKET, NULL, 78, 23),
        create(IDENTIFIER, "i", 78, 24),
        create(DIV, NULL, 78, 26),
        create_value(create_int_value(VALUE_INT, 4), 78, 28),
        create(XOR, NULL, 78, 30),
        create(BNOT, NULL, 78, 32),
        create(IDENTIFIER, "size", 78, 33),
        create(SEMICOLON, NULL, 78, 37),
        create(RBRACE, NULL, 79, 5),
        create(INT, NULL, 81, 5),
        create(IDENTIFIER, "i", 81, 9),
        create(ASSIGN, NULL, 81, 11),
        create_value(create_int_value(VALUE_INT, 0), 81, 13),
        create(SEMICOLON, NULL, 81, 14),
        create(INT, NULL, 82, 5),
        create(IDENTIFIER, "victim", 82, 9),
        create(ASSIGN, NULL, 82, 16),
        create_value(create_uint_value(VALUE_UINT, 0xdeadbeef), 82, 18),
        create(SEMICOLON, NULL, 82, 28),
        create(DO, NULL, 83, 5),
        create(LBRACE, NULL, 83, 8),
        create(IF, NULL, 84, 9),
        create(LBRACKET, NULL, 84, 12),
        create(IDENTIFIER, "i", 84, 13),
        create(EQ_OP, NULL, 84, 15),
        create_value(create_int_value(VALUE_INT, 40), 84, 18),
        create(RBRACKET, NULL, 84, 20),
        create(LBRACE, NULL, 84, 22),
        create(CONTINUE, NULL, 85, 13),
        create(SEMICOLON, NULL, 85, 21),
        create(RBRACE, NULL, 86, 9),
        create(ELSE, NULL, 86, 11),
        create(IF, NULL, 86, 16),
        create(LBRACKET, NULL, 86, 19),
        create(IDENTIFIER, "i", 86, 20),
        create(EQ_OP, NULL, 86, 22),
        create_value(create_int_value(VALUE_INT, 1000), 86, 25),
        create(RBRACKET, NULL, 86, 29),
        create(LBRACE, NULL, 86, 31),
        create(BREAK, NULL, 87, 13),
        create(SEMICOLON, NULL, 87, 18),
        create(RBRACE, NULL, 88, 9),
        create(ELSE, NULL, 88, 11),
        create(LBRACE, NULL, 88, 16),
        create(INC_OP, NULL, 89, 13),
        create(IDENTIFIER, "i", 89, 15),
        create(SEMICOLON, NULL, 89, 16),
        create(RBRACE, NULL, 90, 9),
        create(IDENTIFIER, "victim", 91, 9),
        create(ADD_ASSIGN, NULL, 91, 16),
        create(IDENTIFIER, "i", 91, 19),
        create(SEMICOLON, NULL, 91, 20),
        create(IDENTIFIER, "victim", 92, 9),
        create(SUB_ASSIGN, NULL, 92, 16),
        create(IDENTIFIER, "LIMIT", 92, 19),
        create(SEMICOLON, NULL, 92, 24),
        create(IDENTIFIER, "victim", 93, 9),
        create(DIV_ASSIGN, NULL, 93, 16),
        create(LBRACKET, NULL, 93, 19),
        create(INT, NULL, 93, 20),
        create(RBRACKET, NULL, 93, 23),
        create(IDENTIFIER, "c", 93, 24),
        create(SEMICOLON, NULL, 93, 25),
        create(IDENTIFIER, "d", 94, 9),
        create(MUL_ASSIGN, NULL, 94, 11),
        create(IDENTIFIER, "victim", 94, 14),
        create(SEMICOLON, NULL, 94, 20),
        create(IDENTIFIER, "type_size", 95, 9),
        create(MOD_ASSIGN, NULL, 95, 19),
        create(IDENTIFIER, "size", 95, 22),
        create(SEMICOLON, NULL, 95, 26),
        create(IDENTIFIER, "true_bool", 96, 9),
        create(LEFT_ASSIGN, NULL, 96, 19),
        create_value(create_int_value(VALUE_INT, 4), 96, 23),
        create(SEMICOLON, NULL, 96, 24),
        create(IDENTIFIER, "true_bool", 97, 9),
        create(RIGHT_ASSIGN, NULL, 97, 19),
        create_value(create_int_value(VALUE_INT, 5), 97, 23),
        create(SEMICOLON, NULL, 97, 24),
        create(IDENTIFIER, "arr", 98, 9),
        create(LINDEX, NULL, 98, 12),
        create(IDENTIFIER, "i", 98, 13),
        create(RINDEX, NULL, 98, 14),
        create(AND_ASSIGN, NULL, 98, 16),
        create_value(create_int_value(VALUE_INT, 0x2341), 98, 19),
        create(SEMICOLON, NULL, 98, 25),
        create(IDENTIFIER, "arr", 99, 9),
        create(LINDEX, NULL, 99, 12),
        create(IDENTIFIER, "i", 99, 13),
        create(ADD, NULL, 99, 15),
        create_value(create_int_value(VALUE_INT, 1), 99, 17),
        create(RINDEX, NULL, 99, 18),
        create(OR_ASSIGN, NULL, 99, 20),
        create_value(create_int_value(VALUE_INT, 1), 99, 23),
        create(SEMICOLON, NULL, 99, 24),
        create(IDENTIFIER, "arr", 100, 9),
        create(LINDEX, NULL, 100, 12),
        create_value(create_int_value(VALUE_INT, 0), 100, 13),
        create(RINDEX, NULL, 100, 14),
        create(XOR_ASSIGN, NULL, 100, 16),
        create_value(create_int_value(VALUE_INT, 10423), 100, 19),
        create(SEMICOLON, NULL, 100, 24),
        create(RBRACE, NULL, 101, 5),
        create(WHILE, NULL, 101, 7),
        create(LBRACKET, NULL, 101, 13),
        create_value(create_int_value(VALUE_INT, 1), 101, 14),
        create(RBRACKET, NULL, 101, 15),
        create(SEMICOLON, NULL, 101, 16),
        create(WHILE, NULL, 103, 5),
        create(LBRACKET, NULL, 103, 11),
        create_value(create_int_value(VALUE_INT, 1), 103, 12),
        create(RBRACKET, NULL, 103, 13),
        create(LBRACE, NULL, 103, 15),
        create(BREAK, NULL, 104, 9),
        create(SEMICOLON, NULL, 104, 14),
        create(RBRACE, NULL, 105, 5),
        create(RETURN, NULL, 107, 5),
        create(LBRACKET, NULL, 107, 12),
        create(IDENTIFIER, "arr", 107, 13),
        create(LINDEX, NULL, 107, 16),
        create_value(create_int_value(VALUE_INT, 0), 107, 17),
        create(RINDEX, NULL, 107, 18),
        create(EQ_OP, NULL, 107, 20),
        create_value(create_int_value(VALUE_INT, 37), 107, 23),
        create(QMARK, NULL, 107, 26),
        create(IDENTIFIER, "size", 107, 28),
        create(COLON, NULL, 107, 33),
        create(LBRACKET, NULL, 107, 35),
        create(IDENTIFIER, "arr", 107, 36),
        create(LINDEX, NULL, 107, 39),
        create_value(create_int_value(VALUE_INT, 1), 107, 40),
        create(RINDEX, NULL, 107, 41),
        create(EQ_OP, NULL, 107, 43),
        create_value(create_int_value(VALUE_INT, 37), 107, 46),
        create(QMARK, NULL, 107, 49),
        create(IDENTIFIER, "type_size", 107, 51),
        create(COLON, NULL, 107, 61),
        create(SUB, NULL, 107, 63),
        create_value(create_int_value(VALUE_INT, 123123), 107, 64),
        create(RIGHT_OP, NULL, 107, 71),
        create_value(create_int_value(VALUE_INT, 4), 107, 74),
        create(RBRACKET, NULL, 107, 75),
        create(RBRACKET, NULL, 107, 76),
        create(SEMICOLON, NULL, 107, 77),
        create(RBRACE, NULL, 108, 1),
        create(NORETURN, NULL, 110, 1),
        create(STATIC, NULL, 110, 11),
        create(VOID, NULL, 110, 18),
        create(IDENTIFIER, "variadic", 110, 23),
        create(LBRACKET, NULL, 110, 31),
        create(INT, NULL, 110, 32),
        create(IDENTIFIER, "m", 110, 36),
        create(COMMA, NULL, 110, 37),
        create(ELLIPSIS, NULL, 110, 39),
        create(RBRACKET, NULL, 110, 42),
        create(LBRACE, NULL, 110, 44),
        create(UNSIGNED, NULL, 111, 5),
        create(CHAR, NULL, 111, 14),
        create(IDENTIFIER, "c", 111, 19),
        create(ASSIGN, NULL, 111, 21),
        create_value(create_int_value(VALUE_INT, '\n'), 111, 23),
        create(SEMICOLON, NULL, 111, 27),
        create(CONST, NULL, 113, 5),
        create(CHAR, NULL, 113, 11),
        create(ASTERISK, NULL, 113, 15),
        create(IDENTIFIER, "func_name", 113, 17),
        create(ASSIGN, NULL, 113, 27),
        create(FUNC_NAME, NULL, 113, 29),
        create(SEMICOLON, NULL, 113, 37),
        create(DOUBLE, NULL, 114, 5),
        create(COMPLEX, NULL, 114, 12),
        create(IDENTIFIER, "comp_d", 114, 21),
        create(ASSIGN, NULL, 114, 28),
        create_value(create_int_value(VALUE_INT, 0), 114, 30),
        create(SEMICOLON, 0, 114, 31),
        create(DOUBLE, NULL, 115, 5),
        create(IMAGINARY, NULL, 115, 12),
        create(IDENTIFIER, "im_d", 115, 23),
        create(ASSIGN, NULL, 115, 28),
        create(GENERIC, NULL, 115, 30),
        create(LBRACKET, NULL, 115, 38),
        create_value(create_float_value(VALUE_DOUBLE, 1.0), 115, 39),
        create(COMMA, NULL, 115, 42),
        create(FLOAT, NULL, 115, 44),
        create(COLON, NULL, 115, 49),
        create_value(create_float_value(VALUE_FLOAT, 0x1p+2f), 115, 51),
        create(COMMA, NULL, 115, 58),
        create(DOUBLE, NULL, 115, 60),
        create(COLON, NULL, 115, 66),
        create_value(create_float_value(VALUE_LDOUBLE, 0X12.0P-10L), 115, 68),
        create(RBRACKET, NULL, 115, 79),
        create(SEMICOLON, NULL, 115, 80),
        create(STATIC_ASSERT, NULL, 116, 5),
        create(LBRACKET, NULL, 116, 19),
        create_value(create_int_value(VALUE_INT, 1), 116, 20),
        create(COMMA, NULL, 116, 21),
        create(STRING_LITERAL, "\"Something is wrong\"", 116, 23),
        create(RBRACKET, NULL, 116, 43),
        create(SEMICOLON, NULL, 116, 44),
        create(RETURN, NULL, 117, 5),
        create(SEMICOLON, NULL, 117, 11),
        create(CHAR, NULL, 118, 5),
        create(IDENTIFIER, "d", 118, 10),
        create(ASSIGN, NULL, 118, 12),
        create_value(create_int_value(VALUE_INT, '\\'), 118, 14),
        create(COMMA, NULL, 118, 18),
        create(IDENTIFIER, "e", 118, 20),
        create(ASSIGN, NULL, 118, 22),
        create_value(create_int_value(VALUE_INT, '\''), 118, 24),
        create(COMMA, NULL, 118, 28),
        create(IDENTIFIER, "f", 118, 30),
        create(ASSIGN, NULL, 118, 32),
        create_value(create_int_value(VALUE_INT, '\"'), 118, 34),
        create(COMMA, NULL, 118, 37),
        create(IDENTIFIER, "g", 118, 39),
        create(ASSIGN, NULL, 118, 41),
        create_value(create_int_value(VALUE_INT, '\0'), 118, 43),
        create(SEMICOLON, NULL, 118, 47),
        create(INT, NULL, 119, 5),
        create(IDENTIFIER, "h", 119, 9),
        create(ASSIGN, NULL, 119, 11),
        create_value(create_int_value(VALUE_INT, 's'), 119, 13),
        create(SEMICOLON, NULL, 119, 16),
        create(RBRACE, NULL, 120, 1),
        create(THREAD_LOCAL, NULL, 122, 1),
        create(INT, NULL, 122, 15),
        create(IDENTIFIER, "g_thread", 122, 19),
        create(SEMICOLON, NULL, 122, 27),
    };
    check_token_arr_file(filename,
                         expected,
                         sizeof expected / sizeof *expected);
}

TEST(hex_literal_or_var) {
    {
        const char* code = "vare-10";

        const struct token expected[] = {
            create(IDENTIFIER, "vare", 1, 1),
            create(SUB, NULL, 1, 5),
            create_value(create_int_value(VALUE_INT, 10), 1, 6),
        };
        check_token_arr_str(code, expected, sizeof expected / sizeof *expected);
    }
    {
        const char* code = "var2e-10";

        const struct token expected[] = {
            create(IDENTIFIER, "var2e", 1, 1),
            create(SUB, NULL, 1, 6),
            create_value(create_int_value(VALUE_INT, 10), 1, 7),
        };
        check_token_arr_str(code, expected, sizeof expected / sizeof *expected);
    }
    {
        const char* code = "var2p-10";

        const struct token expected[] = {
            create(IDENTIFIER, "var2p", 1, 1),
            create(SUB, NULL, 1, 6),
            create_value(create_int_value(VALUE_INT, 10), 1, 7),
        };
        check_token_arr_str(code, expected, sizeof expected / sizeof *expected);
    }
}

TEST(dot_float_literal_or_op) {
    {
        const char* code = "int n = .001";

        const struct token expected[] = {
            create(INT, NULL, 1, 1),
            create(IDENTIFIER, "n", 1, 5),
            create(ASSIGN, NULL, 1, 7),
            create_value(create_float_value(VALUE_DOUBLE, .001), 1, 9),
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
                           const char* spelling,
                           size_t line,
                           size_t index) {
    return (struct token){
        .type = type,
        .spelling = (char*)spelling,
        .loc =
            {
                .file_idx = 0,
                .file_loc = {line, index},
            },
    };
}

static struct token create_value(struct value val, size_t line, size_t index) {
    enum token_type type = I_CONSTANT;
    if (val.type == VALUE_FLOAT || val.type == VALUE_DOUBLE
        || val.type == VALUE_LDOUBLE) {
        type = F_CONSTANT;
    }
    return (struct token){
        .type = type,
        .val = val,
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
        ASSERT_VALUE_TYPE(t->val.type, expected->val.type);
        if (value_is_int(t->val.type)) {
            ASSERT_INTMAX_T(t->val.int_val, expected->val.int_val);
        } else {
            ASSERT_UINTMAX_T(t->val.uint_val, expected->val.uint_val);
        }
    } else if (t->type == F_CONSTANT) {
        ASSERT(t->val.type == expected->val.type);
        ASSERT_LONG_DOUBLE(t->val.float_val, expected->val.float_val, 0.0001);
    } else {
        ASSERT_STR(t->spelling, expected->spelling);
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
