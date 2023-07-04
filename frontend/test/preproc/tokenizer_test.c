#include <string.h>

#include "testing/asserts.h"

#include "frontend/preproc/preproc.h"

#include "../test_helpers.h"

static Token create_file(TokenKind kind,
                         StrBuf spelling,
                         size_t line,
                         size_t idx,
                         size_t file);

static Token create(TokenKind kind, StrBuf spelling, size_t line, size_t index);
static Token create_tok_val(Value val, size_t line, size_t index);

static Token create_tok_str_lit(StrLitKind kind,
                                StrBuf cont,
                                size_t line,
                                size_t index);

static void check_token_arr_file(CStr filename,
                                 const Token* expected,
                                 size_t expected_len);
static void check_token_arr_str(CStr code,
                                const Token* expected,
                                size_t expected_len);

TEST(simple) {
    CStr code = CSTR_LIT(
        "typedef struct typedeftest /* This is a comment \n"
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
        "int arr[1 ? 100 : 1000];\n");

    const Token expected[] = {
        create(TOKEN_TYPEDEF, StrBuf_null(), 1, 1),
        create(TOKEN_STRUCT, StrBuf_null(), 1, 9),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("typedeftest"), 1, 16),
        create(TOKEN_LBRACE, StrBuf_null(), 5, 1),
        create(TOKEN_LONG, StrBuf_null(), 6, 2),
        create(TOKEN_INT, StrBuf_null(), 6, 7),
        create(TOKEN_ASTERISK, StrBuf_null(), 6, 10),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("n"), 6, 12),
        create(TOKEN_SEMICOLON, StrBuf_null(), 6, 13),
        create(TOKEN_CONST, StrBuf_null(), 7, 1),
        create(TOKEN_LONG, StrBuf_null(), 7, 7),
        create(TOKEN_DOUBLE, StrBuf_null(), 7, 12),
        create(TOKEN_ASTERISK, StrBuf_null(), 7, 19),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("m"), 7, 20),
        create(TOKEN_SEMICOLON, StrBuf_null(), 7, 21),
        create(TOKEN_RBRACE, StrBuf_null(), 8, 1),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("Typedeftest"), 8, 3),
        create(TOKEN_SEMICOLON, StrBuf_null(), 8, 14),
        create(TOKEN_CONST, StrBuf_null(), 9, 1),
        create(TOKEN_CHAR, StrBuf_null(), 9, 7),
        create(TOKEN_ASTERISK, StrBuf_null(), 9, 11),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("lstr"), 9, 13),
        create(TOKEN_ASSIGN, StrBuf_null(), 9, 18),
        create_tok_str_lit(
            STR_LIT_L,
            STR_BUF_NON_HEAP(
                "Long string literal to check if long strings work"),
            10,
            1),
        create(TOKEN_SEMICOLON, StrBuf_null(), 10, 53),
        create(TOKEN_INT, StrBuf_null(), 11, 1),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("n"), 11, 5),
        create(TOKEN_ASSIGN, StrBuf_null(), 11, 7),
        create_tok_val(Value_create_sint(VALUE_I, 0x123213), 11, 9),
        create(TOKEN_ADD, StrBuf_null(), 11, 18),
        create_tok_val(Value_create_sint(VALUE_I, 132), 11, 20),
        create(TOKEN_LSHIFT, StrBuf_null(), 11, 24),
        create_tok_val(Value_create_sint(VALUE_I, 32), 11, 27),
        create(TOKEN_RSHIFT, StrBuf_null(), 11, 30),
        create_tok_val(Value_create_sint(VALUE_I, 0x123), 11, 33),
        create(TOKEN_SUB, StrBuf_null(), 11, 39),
        create_tok_val(Value_create_sint(VALUE_I, 0123), 11, 41),
        create(TOKEN_DIV, StrBuf_null(), 11, 46),
        create_tok_val(Value_create_sint(VALUE_I, 12), 11, 48),
        create(TOKEN_SEMICOLON, StrBuf_null(), 11, 50),
        create(TOKEN_CONST, StrBuf_null(), 12, 1),
        create(TOKEN_CHAR, StrBuf_null(), 12, 7),
        create(TOKEN_ASTERISK, StrBuf_null(), 12, 11),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("str"), 12, 13),
        create(TOKEN_ASSIGN, StrBuf_null(), 12, 17),
        create_tok_str_lit(STR_LIT_DEFAULT,
                           STR_BUF_NON_HEAP("Normal string literal"),
                           12,
                           19),
        create(TOKEN_SEMICOLON, StrBuf_null(), 12, 42),
        create(TOKEN_INT, StrBuf_null(), 13, 1),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("arr"), 13, 5),
        create(TOKEN_LINDEX, StrBuf_null(), 13, 8),
        create_tok_val(Value_create_sint(VALUE_I, 1), 13, 9),
        create(TOKEN_QMARK, StrBuf_null(), 13, 11),
        create_tok_val(Value_create_sint(VALUE_I, 100), 13, 13),
        create(TOKEN_COLON, StrBuf_null(), 13, 17),
        create_tok_val(Value_create_sint(VALUE_I, 1000), 13, 19),
        create(TOKEN_RINDEX, StrBuf_null(), 13, 23),
        create(TOKEN_SEMICOLON, StrBuf_null(), 13, 24),
    };
    check_token_arr_str(code, expected, ARR_LEN(expected));
}

TEST(file) {
    CStr filename = CSTR_LIT("../frontend/test/files/no_preproc.c");

    const Token expected[] = {
        create(TOKEN_TYPEDEF, StrBuf_null(), 3, 1),
        create(TOKEN_STRUCT, StrBuf_null(), 3, 9),
        create(TOKEN_LBRACE, StrBuf_null(), 3, 16),
        create(TOKEN_ATOMIC, StrBuf_null(), 4, 5),
        create(TOKEN_VOLATILE, StrBuf_null(), 4, 13),
        create(TOKEN_INT, StrBuf_null(), 4, 22),
        create(TOKEN_ASTERISK, StrBuf_null(), 4, 25),
        create(TOKEN_RESTRICT, StrBuf_null(), 4, 27),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("ptr"), 4, 36),
        create(TOKEN_SEMICOLON, StrBuf_null(), 4, 39),
        create(TOKEN_ALIGNAS, StrBuf_null(), 5, 5),
        create(TOKEN_LBRACKET, StrBuf_null(), 5, 13),
        create_tok_val(Value_create_sint(VALUE_I, 16), 5, 14),
        create(TOKEN_RBRACKET, StrBuf_null(), 5, 16),
        create(TOKEN_CONST, StrBuf_null(), 5, 18),
        create(TOKEN_CHAR, StrBuf_null(), 5, 24),
        create(TOKEN_ASTERISK, StrBuf_null(), 5, 29),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("str"), 5, 31),
        create(TOKEN_SEMICOLON, StrBuf_null(), 5, 34),
        create(TOKEN_RBRACE, StrBuf_null(), 6, 1),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("MyStruct"), 6, 3),
        create(TOKEN_SEMICOLON, StrBuf_null(), 6, 11),
        create(TOKEN_UNION, StrBuf_null(), 8, 1),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("my_union"), 8, 7),
        create(TOKEN_LBRACE, StrBuf_null(), 8, 16),
        create(TOKEN_SHORT, StrBuf_null(), 9, 5),
        create(TOKEN_INT, StrBuf_null(), 9, 11),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i"), 9, 15),
        create(TOKEN_COLON, StrBuf_null(), 9, 17),
        create_tok_val(Value_create_sint(VALUE_I, 4), 9, 19),
        create(TOKEN_COMMA, StrBuf_null(), 9, 20),
        create(TOKEN_COLON, StrBuf_null(), 9, 22),
        create_tok_val(Value_create_sint(VALUE_I, 4), 9, 24),
        create(TOKEN_SEMICOLON, StrBuf_null(), 9, 25),
        create(TOKEN_ALIGNAS, StrBuf_null(), 10, 5),
        create(TOKEN_LBRACKET, StrBuf_null(), 10, 13),
        create(TOKEN_CONST, StrBuf_null(), 10, 14),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("MyStruct"), 10, 20),
        create(TOKEN_RBRACKET, StrBuf_null(), 10, 28),
        create(TOKEN_FLOAT, StrBuf_null(), 10, 30),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("f"), 10, 36),
        create(TOKEN_SEMICOLON, StrBuf_null(), 10, 37),
        create(TOKEN_RBRACE, StrBuf_null(), 11, 1),
        create(TOKEN_SEMICOLON, StrBuf_null(), 11, 2),
        create(TOKEN_ENUM, StrBuf_null(), 14, 1),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("my_enum"), 14, 6),
        create(TOKEN_LBRACE, StrBuf_null(), 14, 14),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("VAL_1"), 15, 5),
        create(TOKEN_COMMA, StrBuf_null(), 15, 10),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("VAL_2"), 16, 5),
        create(TOKEN_COMMA, StrBuf_null(), 16, 10),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("VAL_3"), 17, 5),
        create(TOKEN_RBRACE, StrBuf_null(), 18, 1),
        create(TOKEN_SEMICOLON, StrBuf_null(), 18, 2),
        create(TOKEN_STATIC, StrBuf_null(), 20, 1),
        create(TOKEN_INLINE, StrBuf_null(), 20, 8),
        create(TOKEN_INT, StrBuf_null(), 20, 15),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("do_shit"), 20, 19),
        create(TOKEN_LBRACKET, StrBuf_null(), 20, 26),
        create(TOKEN_INT, StrBuf_null(), 20, 27),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("n"), 20, 31),
        create(TOKEN_COMMA, StrBuf_null(), 20, 32),
        create(TOKEN_INT, StrBuf_null(), 20, 34),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("m"), 20, 38),
        create(TOKEN_RBRACKET, StrBuf_null(), 20, 39),
        create(TOKEN_SEMICOLON, StrBuf_null(), 20, 40),
        create(TOKEN_STATIC, StrBuf_null(), 22, 1),
        create(TOKEN_VOID, StrBuf_null(), 22, 8),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("variadic"), 22, 13),
        create(TOKEN_LBRACKET, StrBuf_null(), 22, 21),
        create(TOKEN_INT, StrBuf_null(), 22, 22),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("m"), 22, 26),
        create(TOKEN_COMMA, StrBuf_null(), 22, 27),
        create(TOKEN_ELLIPSIS, StrBuf_null(), 22, 29),
        create(TOKEN_RBRACKET, StrBuf_null(), 22, 32),
        create(TOKEN_SEMICOLON, StrBuf_null(), 22, 33),
        create(TOKEN_EXTERN, StrBuf_null(), 24, 1),
        create(TOKEN_INT, StrBuf_null(), 24, 8),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("some_func"), 24, 12),
        create(TOKEN_LBRACKET, StrBuf_null(), 24, 21),
        create(TOKEN_RBRACKET, StrBuf_null(), 24, 22),
        create(TOKEN_SEMICOLON, StrBuf_null(), 24, 23),
        create(TOKEN_INT, StrBuf_null(), 26, 1),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("main"), 26, 5),
        create(TOKEN_LBRACKET, StrBuf_null(), 26, 9),
        create(TOKEN_RBRACKET, StrBuf_null(), 26, 10),
        create(TOKEN_LBRACE, StrBuf_null(), 26, 12),
        create(TOKEN_REGISTER, StrBuf_null(), 27, 5),
        create(TOKEN_ENUM, StrBuf_null(), 27, 14),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("my_enum"), 27, 19),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("type"), 27, 27),
        create(TOKEN_ASSIGN, StrBuf_null(), 27, 32),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("VAL_1"), 27, 34),
        create(TOKEN_SEMICOLON, StrBuf_null(), 27, 39),
        create(TOKEN_AUTO, StrBuf_null(), 29, 5),
        create(TOKEN_LONG, StrBuf_null(), 29, 10),
        create(TOKEN_SIGNED, StrBuf_null(), 29, 15),
        create(TOKEN_INT, StrBuf_null(), 29, 22),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("value"), 29, 26),
        create(TOKEN_SEMICOLON, StrBuf_null(), 29, 31),
        create(TOKEN_SWITCH, StrBuf_null(), 30, 5),
        create(TOKEN_LBRACKET, StrBuf_null(), 30, 12),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("type"), 30, 13),
        create(TOKEN_RBRACKET, StrBuf_null(), 30, 17),
        create(TOKEN_LBRACE, StrBuf_null(), 30, 19),
        create(TOKEN_CASE, StrBuf_null(), 31, 9),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("VAL_1"), 31, 14),
        create(TOKEN_COLON, StrBuf_null(), 31, 19),
        create(TOKEN_CASE, StrBuf_null(), 32, 9),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("VAL_2"), 32, 14),
        create(TOKEN_COLON, StrBuf_null(), 32, 19),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("value"), 33, 13),
        create(TOKEN_ASSIGN, StrBuf_null(), 33, 19),
        create_tok_val(Value_create_sint(VALUE_L, 1000l), 33, 21),
        create(TOKEN_MOD, StrBuf_null(), 33, 27),
        create_tok_val(Value_create_sint(VALUE_I, 5), 33, 29),
        create(TOKEN_SEMICOLON, StrBuf_null(), 33, 30),
        create(TOKEN_BREAK, StrBuf_null(), 34, 13),
        create(TOKEN_SEMICOLON, StrBuf_null(), 34, 18),
        create(TOKEN_DEFAULT, StrBuf_null(), 36, 9),
        create(TOKEN_COLON, StrBuf_null(), 36, 16),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("value"), 37, 13),
        create(TOKEN_ASSIGN, StrBuf_null(), 37, 19),
        create_tok_val(Value_create_sint(VALUE_L, 30l), 37, 21),
        create(TOKEN_SEMICOLON, StrBuf_null(), 37, 24),
        create(TOKEN_BREAK, StrBuf_null(), 38, 13),
        create(TOKEN_SEMICOLON, StrBuf_null(), 38, 18),
        create(TOKEN_RBRACE, StrBuf_null(), 39, 5),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("MyStruct"), 46, 5),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("s"), 46, 14),
        create(TOKEN_ASSIGN, StrBuf_null(), 46, 16),
        create(TOKEN_LBRACE, StrBuf_null(), 46, 18),
        create_tok_val(Value_create_sint(VALUE_I, 0), 46, 19),
        create(TOKEN_COMMA, StrBuf_null(), 46, 20),
        create_tok_str_lit(
            STR_LIT_L,
            STR_BUF_NON_HEAP(
                "Hello there, this string literal needs to be "
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
                "aaaaaaaaaaaaa"),
            46,
            22),
        create(TOKEN_RBRACE, StrBuf_null(), 46, 1204),
        create(TOKEN_SEMICOLON, StrBuf_null(), 46, 1205),
        create(TOKEN_INT, StrBuf_null(), 47, 5),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("integer"), 47, 9),
        create(TOKEN_ASSIGN, StrBuf_null(), 47, 17),
        create_tok_val(Value_create_sint(VALUE_I, 01000), 47, 19),
        create(TOKEN_LSHIFT, StrBuf_null(), 47, 25),
        create_tok_val(Value_create_sint(VALUE_I, 10), 47, 28),
        create(TOKEN_SEMICOLON, StrBuf_null(), 47, 30),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("s"), 48, 5),
        create(TOKEN_DOT, StrBuf_null(), 48, 6),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("ptr"), 48, 7),
        create(TOKEN_ASSIGN, StrBuf_null(), 48, 11),
        create(TOKEN_AND, StrBuf_null(), 48, 13),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("integer"), 48, 14),
        create(TOKEN_SEMICOLON, StrBuf_null(), 48, 21),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("MyStruct"), 50, 5),
        create(TOKEN_ASTERISK, StrBuf_null(), 50, 14),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("s_ptr"), 50, 15),
        create(TOKEN_ASSIGN, StrBuf_null(), 50, 21),
        create(TOKEN_AND, StrBuf_null(), 50, 23),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("s"), 50, 24),
        create(TOKEN_SEMICOLON, StrBuf_null(), 50, 25),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("s_ptr"), 51, 5),
        create(TOKEN_PTR_OP, StrBuf_null(), 51, 10),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("str"), 51, 12),
        create(TOKEN_ASSIGN, StrBuf_null(), 51, 16),
        create_tok_str_lit(STR_LIT_DEFAULT,
                           STR_BUF_NON_HEAP("Goodbye"),
                           51,
                           18),
        create(TOKEN_SEMICOLON, StrBuf_null(), 51, 27),
        create(TOKEN_ASTERISK, StrBuf_null(), 52, 5),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("s_ptr"), 52, 6),
        create(TOKEN_ASSIGN, StrBuf_null(), 52, 12),
        create(TOKEN_LBRACKET, StrBuf_null(), 52, 14),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("MyStruct"), 52, 15),
        create(TOKEN_RBRACKET, StrBuf_null(), 52, 23),
        create(TOKEN_LBRACE, StrBuf_null(), 52, 24),
        create_tok_str_lit(
            STR_LIT_L,
            STR_BUF_NON_HEAP("\\\"Lstrings seem to be int pointers\\\""),
            52,
            25),
        create(TOKEN_COMMA, StrBuf_null(), 52, 64),
        create_tok_str_lit(STR_LIT_DEFAULT, STR_BUF_NON_HEAP("doot"), 52, 66),
        create(TOKEN_RBRACE, StrBuf_null(), 52, 72),
        create(TOKEN_SEMICOLON, StrBuf_null(), 52, 73),
        create(TOKEN_UNION, StrBuf_null(), 54, 5),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("my_union"), 54, 11),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("soviet_union"), 54, 20),
        create(TOKEN_SEMICOLON, StrBuf_null(), 54, 32),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("soviet_union"), 55, 5),
        create(TOKEN_DOT, StrBuf_null(), 55, 17),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i"), 55, 18),
        create(TOKEN_ASSIGN, StrBuf_null(), 55, 20),
        create_tok_val(Value_create_sint(VALUE_I, 0x1000), 55, 22),
        create(TOKEN_ADD, StrBuf_null(), 55, 29),
        create_tok_val(Value_create_sint(VALUE_I, 033242), 55, 31),
        create(TOKEN_SEMICOLON, StrBuf_null(), 55, 37),
        create(TOKEN_INT, StrBuf_null(), 56, 5),
        create(
            TOKEN_IDENTIFIER,
            STR_BUF_NON_HEAP(
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
        create(TOKEN_SEMICOLON, StrBuf_null(), 56, 1141),
        create(TOKEN_GOTO, StrBuf_null(), 57, 5),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("my_cool_label"), 57, 10),
        create(TOKEN_SEMICOLON, StrBuf_null(), 57, 23),
        create(TOKEN_RETURN, StrBuf_null(), 59, 5),
        create(TOKEN_ALIGNOF, StrBuf_null(), 59, 12),
        create(TOKEN_LBRACKET, StrBuf_null(), 59, 20),
        create(TOKEN_LONG, StrBuf_null(), 59, 21),
        create(TOKEN_RBRACKET, StrBuf_null(), 59, 25),
        create(TOKEN_SEMICOLON, StrBuf_null(), 59, 26),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("my_cool_label"), 61, 1),
        create(TOKEN_COLON, StrBuf_null(), 61, 14),
        create(
            TOKEN_IDENTIFIER,
            STR_BUF_NON_HEAP(
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
        create(TOKEN_ASSIGN, StrBuf_null(), 62, 1138),
        create(TOKEN_LBRACKET, StrBuf_null(), 62, 1140),
        create(TOKEN_INT, StrBuf_null(), 62, 1141),
        create(TOKEN_RBRACKET, StrBuf_null(), 62, 1144),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("soviet_union"), 62, 1145),
        create(TOKEN_DOT, StrBuf_null(), 62, 1157),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("f"), 62, 1158),
        create(TOKEN_SEMICOLON, StrBuf_null(), 62, 1159),
        create(TOKEN_RBRACE, StrBuf_null(), 63, 1),
        create(TOKEN_STATIC, StrBuf_null(), 65, 1),
        create(TOKEN_INT, StrBuf_null(), 65, 8),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("do_shit"), 65, 12),
        create(TOKEN_LBRACKET, StrBuf_null(), 65, 19),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("n"), 65, 20),
        create(TOKEN_COMMA, StrBuf_null(), 65, 21),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("m"), 65, 23),
        create(TOKEN_RBRACKET, StrBuf_null(), 65, 24),
        create(TOKEN_INT, StrBuf_null(), 65, 26),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("n"), 65, 30),
        create(TOKEN_SEMICOLON, StrBuf_null(), 65, 31),
        create(TOKEN_INT, StrBuf_null(), 65, 33),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("m"), 65, 37),
        create(TOKEN_SEMICOLON, StrBuf_null(), 65, 38),
        create(TOKEN_LBRACE, StrBuf_null(), 65, 40),
        create(TOKEN_DOUBLE, StrBuf_null(), 66, 5),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("d"), 66, 12),
        create(TOKEN_ASSIGN, StrBuf_null(), 66, 14),
        create_tok_val(Value_create_float(VALUE_D, 1e-10), 66, 16),
        create(TOKEN_SUB, StrBuf_null(), 66, 22),
        create_tok_val(Value_create_float(VALUE_D, 0xabecp10), 66, 24),
        create(TOKEN_SEMICOLON, StrBuf_null(), 66, 33),
        create(TOKEN_INT, StrBuf_null(), 67, 5),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("type_size"), 67, 9),
        create(TOKEN_ASSIGN, StrBuf_null(), 67, 19),
        create(TOKEN_SIZEOF, StrBuf_null(), 67, 21),
        create(TOKEN_LBRACKET, StrBuf_null(), 67, 27),
        create(TOKEN_DOUBLE, StrBuf_null(), 67, 28),
        create(TOKEN_RBRACKET, StrBuf_null(), 67, 34),
        create(TOKEN_SEMICOLON, StrBuf_null(), 67, 35),
        create(TOKEN_INT, StrBuf_null(), 68, 5),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("size"), 68, 9),
        create(TOKEN_ASSIGN, StrBuf_null(), 68, 14),
        create(TOKEN_SIZEOF, StrBuf_null(), 68, 16),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("d"), 68, 23),
        create(TOKEN_SEMICOLON, StrBuf_null(), 68, 24),
        create(TOKEN_CHAR, StrBuf_null(), 70, 5),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("c"), 70, 10),
        create(TOKEN_ASSIGN, StrBuf_null(), 70, 12),
        create_tok_val(Value_create_sint(VALUE_I, '\n'), 70, 14),
        create(TOKEN_SEMICOLON, StrBuf_null(), 70, 18),
        create(TOKEN_BOOL, StrBuf_null(), 72, 5),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("true_bool"), 72, 11),
        create(TOKEN_ASSIGN, StrBuf_null(), 72, 21),
        create(TOKEN_NOT, StrBuf_null(), 72, 23),
        create(TOKEN_LBRACKET, StrBuf_null(), 72, 24),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("c"), 72, 25),
        create(TOKEN_NE, StrBuf_null(), 72, 27),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("c"), 72, 30),
        create(TOKEN_RBRACKET, StrBuf_null(), 72, 31),
        create(TOKEN_LAND, StrBuf_null(), 72, 33),
        create(TOKEN_LBRACKET, StrBuf_null(), 72, 36),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("c"), 72, 37),
        create(TOKEN_LE, StrBuf_null(), 72, 39),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("size"), 72, 42),
        create(TOKEN_LOR, StrBuf_null(), 72, 47),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("c"), 72, 50),
        create(TOKEN_GE, StrBuf_null(), 72, 52),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("size"), 72, 55),
        create(TOKEN_LOR, StrBuf_null(), 72, 60),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("c"), 72, 63),
        create(TOKEN_LT, StrBuf_null(), 72, 65),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("size"), 72, 67),
        create(TOKEN_LOR, StrBuf_null(), 72, 72),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("c"), 72, 75),
        create(TOKEN_GT, StrBuf_null(), 72, 77),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("size"), 72, 79),
        create(TOKEN_LOR, StrBuf_null(), 72, 84),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("c"), 72, 87),
        create(TOKEN_EQ, StrBuf_null(), 72, 89),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("size"), 72, 92),
        create(TOKEN_LOR, StrBuf_null(), 72, 97),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("c"), 72, 100),
        create(TOKEN_NE, StrBuf_null(), 72, 102),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("size"), 72, 105),
        create(TOKEN_RBRACKET, StrBuf_null(), 72, 109),
        create(TOKEN_SEMICOLON, StrBuf_null(), 72, 110),
        create(TOKEN_ENUM, StrBuf_null(), 74, 5),
        create(TOKEN_LBRACE, StrBuf_null(), 74, 10),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("LIMIT"), 74, 12),
        create(TOKEN_ASSIGN, StrBuf_null(), 74, 18),
        create_tok_val(Value_create_sint(VALUE_I, 50000), 74, 20),
        create(TOKEN_RBRACE, StrBuf_null(), 74, 26),
        create(TOKEN_SEMICOLON, StrBuf_null(), 74, 27),
        create(TOKEN_CHAR, StrBuf_null(), 75, 5),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("arr"), 75, 10),
        create(TOKEN_LINDEX, StrBuf_null(), 75, 13),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("LIMIT"), 75, 14),
        create(TOKEN_RINDEX, StrBuf_null(), 75, 19),
        create(TOKEN_ASSIGN, StrBuf_null(), 75, 21),
        create(TOKEN_LBRACE, StrBuf_null(), 75, 23),
        create_tok_val(Value_create_sint(VALUE_I, 0), 75, 24),
        create(TOKEN_RBRACE, StrBuf_null(), 75, 25),
        create(TOKEN_SEMICOLON, StrBuf_null(), 75, 26),
        create(TOKEN_FOR, StrBuf_null(), 76, 5),
        create(TOKEN_LBRACKET, StrBuf_null(), 76, 9),
        create(TOKEN_INT, StrBuf_null(), 76, 10),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i"), 76, 14),
        create(TOKEN_ASSIGN, StrBuf_null(), 76, 16),
        create_tok_val(Value_create_sint(VALUE_I, 0), 76, 18),
        create(TOKEN_COMMA, StrBuf_null(), 76, 19),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("j"), 76, 21),
        create(TOKEN_ASSIGN, StrBuf_null(), 76, 23),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("LIMIT"), 76, 25),
        create(TOKEN_SEMICOLON, StrBuf_null(), 76, 30),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i"), 76, 32),
        create(TOKEN_LT, StrBuf_null(), 76, 34),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("LIMIT"), 76, 36),
        create(TOKEN_LAND, StrBuf_null(), 76, 42),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("j"), 76, 45),
        create(TOKEN_GT, StrBuf_null(), 76, 47),
        create_tok_val(Value_create_sint(VALUE_I, 0), 76, 49),
        create(TOKEN_SEMICOLON, StrBuf_null(), 76, 50),
        create(TOKEN_INC, StrBuf_null(), 76, 52),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i"), 76, 54),
        create(TOKEN_COMMA, StrBuf_null(), 76, 55),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("j"), 76, 57),
        create(TOKEN_DEC, StrBuf_null(), 76, 58),
        create(TOKEN_RBRACKET, StrBuf_null(), 76, 60),
        create(TOKEN_LBRACE, StrBuf_null(), 76, 62),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("arr"), 77, 9),
        create(TOKEN_LINDEX, StrBuf_null(), 77, 12),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i"), 77, 13),
        create(TOKEN_RINDEX, StrBuf_null(), 77, 14),
        create(TOKEN_ASSIGN, StrBuf_null(), 77, 16),
        create(TOKEN_LBRACKET, StrBuf_null(), 77, 18),
        create(TOKEN_CHAR, StrBuf_null(), 77, 19),
        create(TOKEN_RBRACKET, StrBuf_null(), 77, 23),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("j"), 77, 24),
        create(TOKEN_OR, StrBuf_null(), 77, 26),
        create_tok_val(Value_create_sint(VALUE_I, 5), 77, 28),
        create(TOKEN_SEMICOLON, StrBuf_null(), 77, 29),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("arr"), 78, 9),
        create(TOKEN_LINDEX, StrBuf_null(), 78, 12),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("j"), 78, 13),
        create(TOKEN_RINDEX, StrBuf_null(), 78, 14),
        create(TOKEN_ASSIGN, StrBuf_null(), 78, 16),
        create(TOKEN_LBRACKET, StrBuf_null(), 78, 18),
        create(TOKEN_CHAR, StrBuf_null(), 78, 19),
        create(TOKEN_RBRACKET, StrBuf_null(), 78, 23),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i"), 78, 24),
        create(TOKEN_DIV, StrBuf_null(), 78, 26),
        create_tok_val(Value_create_sint(VALUE_I, 4), 78, 28),
        create(TOKEN_XOR, StrBuf_null(), 78, 30),
        create(TOKEN_BNOT, StrBuf_null(), 78, 32),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("size"), 78, 33),
        create(TOKEN_SEMICOLON, StrBuf_null(), 78, 37),
        create(TOKEN_RBRACE, StrBuf_null(), 79, 5),
        create(TOKEN_INT, StrBuf_null(), 81, 5),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i"), 81, 9),
        create(TOKEN_ASSIGN, StrBuf_null(), 81, 11),
        create_tok_val(Value_create_sint(VALUE_I, 0), 81, 13),
        create(TOKEN_SEMICOLON, StrBuf_null(), 81, 14),
        create(TOKEN_INT, StrBuf_null(), 82, 5),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("victim"), 82, 9),
        create(TOKEN_ASSIGN, StrBuf_null(), 82, 16),
        create_tok_val(Value_create_uint(VALUE_UI, 0xdeadbeef), 82, 18),
        create(TOKEN_SEMICOLON, StrBuf_null(), 82, 28),
        create(TOKEN_DO, StrBuf_null(), 83, 5),
        create(TOKEN_LBRACE, StrBuf_null(), 83, 8),
        create(TOKEN_IF, StrBuf_null(), 84, 9),
        create(TOKEN_LBRACKET, StrBuf_null(), 84, 12),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i"), 84, 13),
        create(TOKEN_EQ, StrBuf_null(), 84, 15),
        create_tok_val(Value_create_sint(VALUE_I, 40), 84, 18),
        create(TOKEN_RBRACKET, StrBuf_null(), 84, 20),
        create(TOKEN_LBRACE, StrBuf_null(), 84, 22),
        create(TOKEN_CONTINUE, StrBuf_null(), 85, 13),
        create(TOKEN_SEMICOLON, StrBuf_null(), 85, 21),
        create(TOKEN_RBRACE, StrBuf_null(), 86, 9),
        create(TOKEN_ELSE, StrBuf_null(), 86, 11),
        create(TOKEN_IF, StrBuf_null(), 86, 16),
        create(TOKEN_LBRACKET, StrBuf_null(), 86, 19),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i"), 86, 20),
        create(TOKEN_EQ, StrBuf_null(), 86, 22),
        create_tok_val(Value_create_sint(VALUE_I, 1000), 86, 25),
        create(TOKEN_RBRACKET, StrBuf_null(), 86, 29),
        create(TOKEN_LBRACE, StrBuf_null(), 86, 31),
        create(TOKEN_BREAK, StrBuf_null(), 87, 13),
        create(TOKEN_SEMICOLON, StrBuf_null(), 87, 18),
        create(TOKEN_RBRACE, StrBuf_null(), 88, 9),
        create(TOKEN_ELSE, StrBuf_null(), 88, 11),
        create(TOKEN_LBRACE, StrBuf_null(), 88, 16),
        create(TOKEN_INC, StrBuf_null(), 89, 13),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i"), 89, 15),
        create(TOKEN_SEMICOLON, StrBuf_null(), 89, 16),
        create(TOKEN_RBRACE, StrBuf_null(), 90, 9),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("victim"), 91, 9),
        create(TOKEN_ADD_ASSIGN, StrBuf_null(), 91, 16),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i"), 91, 19),
        create(TOKEN_SEMICOLON, StrBuf_null(), 91, 20),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("victim"), 92, 9),
        create(TOKEN_SUB_ASSIGN, StrBuf_null(), 92, 16),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("LIMIT"), 92, 19),
        create(TOKEN_SEMICOLON, StrBuf_null(), 92, 24),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("victim"), 93, 9),
        create(TOKEN_DIV_ASSIGN, StrBuf_null(), 93, 16),
        create(TOKEN_LBRACKET, StrBuf_null(), 93, 19),
        create(TOKEN_INT, StrBuf_null(), 93, 20),
        create(TOKEN_RBRACKET, StrBuf_null(), 93, 23),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("c"), 93, 24),
        create(TOKEN_SEMICOLON, StrBuf_null(), 93, 25),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("d"), 94, 9),
        create(TOKEN_MUL_ASSIGN, StrBuf_null(), 94, 11),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("victim"), 94, 14),
        create(TOKEN_SEMICOLON, StrBuf_null(), 94, 20),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("type_size"), 95, 9),
        create(TOKEN_MOD_ASSIGN, StrBuf_null(), 95, 19),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("size"), 95, 22),
        create(TOKEN_SEMICOLON, StrBuf_null(), 95, 26),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("true_bool"), 96, 9),
        create(TOKEN_LSHIFT_ASSIGN, StrBuf_null(), 96, 19),
        create_tok_val(Value_create_sint(VALUE_I, 4), 96, 23),
        create(TOKEN_SEMICOLON, StrBuf_null(), 96, 24),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("true_bool"), 97, 9),
        create(TOKEN_RSHIFT_ASSIGN, StrBuf_null(), 97, 19),
        create_tok_val(Value_create_sint(VALUE_I, 5), 97, 23),
        create(TOKEN_SEMICOLON, StrBuf_null(), 97, 24),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("arr"), 98, 9),
        create(TOKEN_LINDEX, StrBuf_null(), 98, 12),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i"), 98, 13),
        create(TOKEN_RINDEX, StrBuf_null(), 98, 14),
        create(TOKEN_AND_ASSIGN, StrBuf_null(), 98, 16),
        create_tok_val(Value_create_sint(VALUE_I, 0x2341), 98, 19),
        create(TOKEN_SEMICOLON, StrBuf_null(), 98, 25),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("arr"), 99, 9),
        create(TOKEN_LINDEX, StrBuf_null(), 99, 12),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i"), 99, 13),
        create(TOKEN_ADD, StrBuf_null(), 99, 15),
        create_tok_val(Value_create_sint(VALUE_I, 1), 99, 17),
        create(TOKEN_RINDEX, StrBuf_null(), 99, 18),
        create(TOKEN_OR_ASSIGN, StrBuf_null(), 99, 20),
        create_tok_val(Value_create_sint(VALUE_I, 1), 99, 23),
        create(TOKEN_SEMICOLON, StrBuf_null(), 99, 24),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("arr"), 100, 9),
        create(TOKEN_LINDEX, StrBuf_null(), 100, 12),
        create_tok_val(Value_create_sint(VALUE_I, 0), 100, 13),
        create(TOKEN_RINDEX, StrBuf_null(), 100, 14),
        create(TOKEN_XOR_ASSIGN, StrBuf_null(), 100, 16),
        create_tok_val(Value_create_sint(VALUE_I, 10423), 100, 19),
        create(TOKEN_SEMICOLON, StrBuf_null(), 100, 24),
        create(TOKEN_RBRACE, StrBuf_null(), 101, 5),
        create(TOKEN_WHILE, StrBuf_null(), 101, 7),
        create(TOKEN_LBRACKET, StrBuf_null(), 101, 13),
        create_tok_val(Value_create_sint(VALUE_I, 1), 101, 14),
        create(TOKEN_RBRACKET, StrBuf_null(), 101, 15),
        create(TOKEN_SEMICOLON, StrBuf_null(), 101, 16),
        create(TOKEN_WHILE, StrBuf_null(), 103, 5),
        create(TOKEN_LBRACKET, StrBuf_null(), 103, 11),
        create_tok_val(Value_create_sint(VALUE_I, 1), 103, 12),
        create(TOKEN_RBRACKET, StrBuf_null(), 103, 13),
        create(TOKEN_LBRACE, StrBuf_null(), 103, 15),
        create(TOKEN_BREAK, StrBuf_null(), 104, 9),
        create(TOKEN_SEMICOLON, StrBuf_null(), 104, 14),
        create(TOKEN_RBRACE, StrBuf_null(), 105, 5),
        create(TOKEN_RETURN, StrBuf_null(), 107, 5),
        create(TOKEN_LBRACKET, StrBuf_null(), 107, 12),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("arr"), 107, 13),
        create(TOKEN_LINDEX, StrBuf_null(), 107, 16),
        create_tok_val(Value_create_sint(VALUE_I, 0), 107, 17),
        create(TOKEN_RINDEX, StrBuf_null(), 107, 18),
        create(TOKEN_EQ, StrBuf_null(), 107, 20),
        create_tok_val(Value_create_sint(VALUE_I, 37), 107, 23),
        create(TOKEN_QMARK, StrBuf_null(), 107, 26),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("size"), 107, 28),
        create(TOKEN_COLON, StrBuf_null(), 107, 33),
        create(TOKEN_LBRACKET, StrBuf_null(), 107, 35),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("arr"), 107, 36),
        create(TOKEN_LINDEX, StrBuf_null(), 107, 39),
        create_tok_val(Value_create_sint(VALUE_I, 1), 107, 40),
        create(TOKEN_RINDEX, StrBuf_null(), 107, 41),
        create(TOKEN_EQ, StrBuf_null(), 107, 43),
        create_tok_val(Value_create_sint(VALUE_I, 37), 107, 46),
        create(TOKEN_QMARK, StrBuf_null(), 107, 49),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("type_size"), 107, 51),
        create(TOKEN_COLON, StrBuf_null(), 107, 61),
        create(TOKEN_SUB, StrBuf_null(), 107, 63),
        create_tok_val(Value_create_sint(VALUE_I, 123123), 107, 64),
        create(TOKEN_RSHIFT, StrBuf_null(), 107, 71),
        create_tok_val(Value_create_sint(VALUE_I, 4), 107, 74),
        create(TOKEN_RBRACKET, StrBuf_null(), 107, 75),
        create(TOKEN_RBRACKET, StrBuf_null(), 107, 76),
        create(TOKEN_SEMICOLON, StrBuf_null(), 107, 77),
        create(TOKEN_RBRACE, StrBuf_null(), 108, 1),
        create(TOKEN_NORETURN, StrBuf_null(), 110, 1),
        create(TOKEN_STATIC, StrBuf_null(), 110, 11),
        create(TOKEN_VOID, StrBuf_null(), 110, 18),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("variadic"), 110, 23),
        create(TOKEN_LBRACKET, StrBuf_null(), 110, 31),
        create(TOKEN_INT, StrBuf_null(), 110, 32),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("m"), 110, 36),
        create(TOKEN_COMMA, StrBuf_null(), 110, 37),
        create(TOKEN_ELLIPSIS, StrBuf_null(), 110, 39),
        create(TOKEN_RBRACKET, StrBuf_null(), 110, 42),
        create(TOKEN_LBRACE, StrBuf_null(), 110, 44),
        create(TOKEN_UNSIGNED, StrBuf_null(), 111, 5),
        create(TOKEN_CHAR, StrBuf_null(), 111, 14),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("c"), 111, 19),
        create(TOKEN_ASSIGN, StrBuf_null(), 111, 21),
        create_tok_val(Value_create_sint(VALUE_I, '\n'), 111, 23),
        create(TOKEN_SEMICOLON, StrBuf_null(), 111, 27),
        create(TOKEN_CONST, StrBuf_null(), 113, 5),
        create(TOKEN_CHAR, StrBuf_null(), 113, 11),
        create(TOKEN_ASTERISK, StrBuf_null(), 113, 15),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("func_name"), 113, 17),
        create(TOKEN_ASSIGN, StrBuf_null(), 113, 27),
        create(TOKEN_FUNC_NAME, StrBuf_null(), 113, 29),
        create(TOKEN_SEMICOLON, StrBuf_null(), 113, 37),
        create(TOKEN_DOUBLE, StrBuf_null(), 114, 5),
        create(TOKEN_COMPLEX, StrBuf_null(), 114, 12),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("comp_d"), 114, 21),
        create(TOKEN_ASSIGN, StrBuf_null(), 114, 28),
        create_tok_val(Value_create_sint(VALUE_I, 0), 114, 30),
        create(TOKEN_SEMICOLON, StrBuf_null(), 114, 31),
        create(TOKEN_DOUBLE, StrBuf_null(), 115, 5),
        create(TOKEN_IMAGINARY, StrBuf_null(), 115, 12),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("im_d"), 115, 23),
        create(TOKEN_ASSIGN, StrBuf_null(), 115, 28),
        create(TOKEN_GENERIC, StrBuf_null(), 115, 30),
        create(TOKEN_LBRACKET, StrBuf_null(), 115, 38),
        create_tok_val(Value_create_float(VALUE_D, 1.0), 115, 39),
        create(TOKEN_COMMA, StrBuf_null(), 115, 42),
        create(TOKEN_FLOAT, StrBuf_null(), 115, 44),
        create(TOKEN_COLON, StrBuf_null(), 115, 49),
        create_tok_val(Value_create_float(VALUE_F, 0x1p+2f), 115, 51),
        create(TOKEN_COMMA, StrBuf_null(), 115, 58),
        create(TOKEN_DOUBLE, StrBuf_null(), 115, 60),
        create(TOKEN_COLON, StrBuf_null(), 115, 66),
        create_tok_val(Value_create_float(VALUE_LD, 0X12.0P-10L), 115, 68),
        create(TOKEN_RBRACKET, StrBuf_null(), 115, 79),
        create(TOKEN_SEMICOLON, StrBuf_null(), 115, 80),
        create(TOKEN_STATIC_ASSERT, StrBuf_null(), 116, 5),
        create(TOKEN_LBRACKET, StrBuf_null(), 116, 19),
        create_tok_val(Value_create_sint(VALUE_I, 1), 116, 20),
        create(TOKEN_COMMA, StrBuf_null(), 116, 21),
        create_tok_str_lit(STR_LIT_DEFAULT,
                           STR_BUF_NON_HEAP("Something is wrong"),
                           116,
                           23),
        create(TOKEN_RBRACKET, StrBuf_null(), 116, 43),
        create(TOKEN_SEMICOLON, StrBuf_null(), 116, 44),
        create(TOKEN_RETURN, StrBuf_null(), 117, 5),
        create(TOKEN_SEMICOLON, StrBuf_null(), 117, 11),
        create(TOKEN_CHAR, StrBuf_null(), 118, 5),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("d"), 118, 10),
        create(TOKEN_ASSIGN, StrBuf_null(), 118, 12),
        create_tok_val(Value_create_sint(VALUE_I, '\\'), 118, 14),
        create(TOKEN_COMMA, StrBuf_null(), 118, 18),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("e"), 118, 20),
        create(TOKEN_ASSIGN, StrBuf_null(), 118, 22),
        create_tok_val(Value_create_sint(VALUE_I, '\''), 118, 24),
        create(TOKEN_COMMA, StrBuf_null(), 118, 28),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("f"), 118, 30),
        create(TOKEN_ASSIGN, StrBuf_null(), 118, 32),
        create_tok_val(Value_create_sint(VALUE_I, '"'), 118, 34),
        create(TOKEN_COMMA, StrBuf_null(), 118, 37),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("g"), 118, 39),
        create(TOKEN_ASSIGN, StrBuf_null(), 118, 41),
        create_tok_val(Value_create_sint(VALUE_I, '\0'), 118, 43),
        create(TOKEN_SEMICOLON, StrBuf_null(), 118, 47),
        create(TOKEN_INT, StrBuf_null(), 119, 5),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("h"), 119, 9),
        create(TOKEN_ASSIGN, StrBuf_null(), 119, 11),
        create_tok_val(Value_create_sint(VALUE_I, 's'), 119, 13),
        create(TOKEN_SEMICOLON, StrBuf_null(), 119, 16),
        create(TOKEN_RBRACE, StrBuf_null(), 120, 1),
        create(TOKEN_THREAD_LOCAL, StrBuf_null(), 122, 1),
        create(TOKEN_INT, StrBuf_null(), 122, 15),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("g_thread"), 122, 19),
        create(TOKEN_SEMICOLON, StrBuf_null(), 122, 27),
    };
    check_token_arr_file(filename, expected, ARR_LEN(expected));
}

TEST(include) {
    CStr filename = CSTR_LIT("../frontend/test/files/include_test/start.c");
    const Token expected[] = {
        create_file(TOKEN_EXTERN, StrBuf_null(), 2, 1, 0),
        create_file(TOKEN_INT, StrBuf_null(), 2, 8, 0),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("n"), 2, 12, 0),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 2, 13, 0),
        // i1
        create_file(TOKEN_ENUM, StrBuf_null(), 4, 1, 1),
        create_file(TOKEN_IDENTIFIER,
                    STR_BUF_NON_HEAP("i1_test_enum"),
                    4,
                    6,
                    1),
        create_file(TOKEN_LBRACE, StrBuf_null(), 4, 19, 1),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("I1_ENUM_1"), 5, 5, 1),
        create_file(TOKEN_COMMA, StrBuf_null(), 5, 14, 1),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("I1_ENUM_2"), 6, 5, 1),
        create_file(TOKEN_COMMA, StrBuf_null(), 6, 14, 1),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("I1_ENUM_3"), 7, 5, 1),
        create_file(TOKEN_COMMA, StrBuf_null(), 7, 14, 1),
        create_file(TOKEN_RBRACE, StrBuf_null(), 8, 1, 1),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 8, 2, 1),
        // i2
        // i3
        // i4
        // i5
        // i6
        create_file(TOKEN_CHAR, StrBuf_null(), 3, 1, 6),
        create_file(TOKEN_IDENTIFIER,
                    STR_BUF_NON_HEAP("i6_test_func"),
                    3,
                    6,
                    6),
        create_file(TOKEN_LBRACKET, StrBuf_null(), 3, 18, 6),
        create_file(TOKEN_LONG, StrBuf_null(), 3, 19, 6),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("b"), 3, 24, 6),
        create_file(TOKEN_RBRACKET, StrBuf_null(), 3, 25, 6),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 3, 26, 6),
        // i7
        // i8
        // i9
        // i10
        create_file(TOKEN_EXTERN, StrBuf_null(), 2, 1, 10),
        create_file(TOKEN_CHAR, StrBuf_null(), 2, 8, 10),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i11_chr"), 2, 13, 10),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 2, 20, 10),
        // i11
        create_file(TOKEN_EXTERN, StrBuf_null(), 2, 1, 11),
        create_file(TOKEN_VOID, StrBuf_null(), 2, 8, 11),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i11_func"), 2, 13, 11),
        create_file(TOKEN_LBRACKET, StrBuf_null(), 2, 21, 11),
        create_file(TOKEN_INT, StrBuf_null(), 2, 22, 11),
        create_file(TOKEN_RBRACKET, StrBuf_null(), 2, 25, 11),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 2, 26, 11),
        // i13
        create_file(TOKEN_EXTERN, StrBuf_null(), 2, 1, 13),
        create_file(TOKEN_CHAR, StrBuf_null(), 2, 8, 13),
        create_file(TOKEN_IDENTIFIER,
                    STR_BUF_NON_HEAP("i13_srsdfkjsdfkl"),
                    2,
                    13,
                    13),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 2, 29, 13),
        // i14
        create_file(TOKEN_INT, StrBuf_null(), 2, 1, 14),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i_14_n"), 2, 5, 14),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 2, 11, 14),
        // i15
        create_file(TOKEN_LONG, StrBuf_null(), 3, 1, 15),
        create_file(TOKEN_ASTERISK, StrBuf_null(), 3, 5, 15),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i15_ptr"), 3, 7, 15),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 3, 14, 15),
        // i17
        create_file(TOKEN_TYPEDEF, StrBuf_null(), 1, 1, 17),
        create_file(TOKEN_INT, StrBuf_null(), 1, 9, 17),
        create_file(TOKEN_IDENTIFIER,
                    STR_BUF_NON_HEAP("I17Typedef"),
                    1,
                    13,
                    17),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 1, 23, 17),
        // i19
        create_file(TOKEN_CHAR, StrBuf_null(), 2, 1, 19),
        create_file(TOKEN_ASTERISK, StrBuf_null(), 2, 6, 19),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i19_str"), 2, 7, 19),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 2, 14, 19),
        // i20
        create_file(TOKEN_VOID, StrBuf_null(), 2, 1, 20),
        create_file(TOKEN_ASTERISK, StrBuf_null(), 2, 5, 20),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i20_malloc"), 2, 7, 20),
        create_file(TOKEN_LBRACKET, StrBuf_null(), 2, 17, 20),
        create_file(TOKEN_INT, StrBuf_null(), 2, 18, 20),
        create_file(TOKEN_RBRACKET, StrBuf_null(), 2, 21, 20),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 2, 22, 20),
        // i21
        create_file(TOKEN_INT, StrBuf_null(), 2, 1, 21),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i21_nonono"), 2, 5, 21),
        create_file(TOKEN_LBRACKET, StrBuf_null(), 2, 15, 21),
        create_file(TOKEN_CHAR, StrBuf_null(), 2, 16, 21),
        create_file(TOKEN_RBRACKET, StrBuf_null(), 2, 20, 21),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 2, 21, 21),
        // i23
        create_file(TOKEN_CHAR, StrBuf_null(), 2, 1, 23),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i23_char"), 2, 6, 23),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 2, 14, 23),
        // i24
        create_file(TOKEN_VOID, StrBuf_null(), 3, 1, 24),
        create_file(TOKEN_LBRACKET, StrBuf_null(), 3, 6, 24),
        create_file(TOKEN_ASTERISK, StrBuf_null(), 3, 7, 24),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i24_void"), 3, 8, 24),
        create_file(TOKEN_RBRACKET, StrBuf_null(), 3, 16, 24),
        create_file(TOKEN_LBRACKET, StrBuf_null(), 3, 17, 24),
        create_file(TOKEN_VOID, StrBuf_null(), 3, 18, 24),
        create_file(TOKEN_RBRACKET, StrBuf_null(), 3, 22, 24),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 3, 23, 24),
        // i23
        create_file(TOKEN_INT, StrBuf_null(), 6, 1, 23),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i23_int"), 6, 5, 23),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 6, 12, 23),
        // i22
        create_file(TOKEN_VOID, StrBuf_null(), 3, 1, 22),
        create_file(TOKEN_ASTERISK, StrBuf_null(), 3, 5, 22),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i22_no"), 3, 7, 22),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 3, 13, 22),
        // i21
        create_file(TOKEN_VOID, StrBuf_null(), 6, 1, 21),
        create_file(TOKEN_IDENTIFIER,
                    STR_BUF_NON_HEAP("i21_when_will_this_end"),
                    6,
                    6,
                    21),
        create_file(TOKEN_LBRACKET, StrBuf_null(), 6, 28, 21),
        create_file(TOKEN_VOID, StrBuf_null(), 6, 29, 21),
        create_file(TOKEN_RBRACKET, StrBuf_null(), 6, 33, 21),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 6, 34, 21),
        // i18
        create_file(TOKEN_UNION, StrBuf_null(), 3, 1, 18),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i18_union"), 3, 7, 18),
        create_file(TOKEN_LBRACE, StrBuf_null(), 3, 17, 18),
        create_file(TOKEN_INT, StrBuf_null(), 4, 5, 18),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i"), 4, 9, 18),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 4, 10, 18),
        create_file(TOKEN_FLOAT, StrBuf_null(), 5, 5, 18),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("f"), 5, 11, 18),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 5, 12, 18),
        create_file(TOKEN_RBRACE, StrBuf_null(), 6, 1, 18),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 6, 2, 18),
        // i16
        create_file(TOKEN_SIGNED, StrBuf_null(), 3, 1, 16),
        create_file(TOKEN_IDENTIFIER,
                    STR_BUF_NON_HEAP("i16_signed_int"),
                    3,
                    8,
                    16),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 3, 22, 16),
        // i15
        create_file(TOKEN_STRUCT, StrBuf_null(), 7, 1, 15),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i15_struct"), 7, 8, 15),
        create_file(TOKEN_LBRACE, StrBuf_null(), 7, 19, 15),
        create_file(TOKEN_INT, StrBuf_null(), 8, 5, 15),
        create_file(TOKEN_ASTERISK, StrBuf_null(), 8, 8, 15),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("data"), 8, 10, 15),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 8, 14, 15),
        create_file(TOKEN_INT, StrBuf_null(), 9, 5, 15),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("len"), 9, 9, 15),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 9, 12, 15),
        create_file(TOKEN_RBRACE, StrBuf_null(), 10, 1, 15),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 10, 2, 15),
        // i12
        create_file(TOKEN_INT, StrBuf_null(), 4, 1, 12),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i12_int"), 4, 5, 12),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 4, 12, 12),
        create_file(TOKEN_STRUCT, StrBuf_null(), 7, 1, 12),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i12_struct"), 7, 8, 12),
        create_file(TOKEN_LBRACE, StrBuf_null(), 7, 19, 12),
        create_file(TOKEN_VOID, StrBuf_null(), 8, 5, 12),
        create_file(TOKEN_ASTERISK, StrBuf_null(), 8, 9, 12),
        create_file(TOKEN_ASTERISK, StrBuf_null(), 8, 10, 12),
        create_file(TOKEN_ASTERISK, StrBuf_null(), 8, 11, 12),
        create_file(TOKEN_ASTERISK, StrBuf_null(), 8, 12, 12),
        create_file(TOKEN_ASTERISK, StrBuf_null(), 8, 13, 12),
        create_file(TOKEN_ASTERISK, StrBuf_null(), 8, 14, 12),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("ptr"), 8, 16, 12),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 8, 19, 12),
        create_file(TOKEN_RBRACE, StrBuf_null(), 9, 1, 12),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 9, 2, 12),
        // i9
        create_file(TOKEN_INT, StrBuf_null(), 5, 1, 9),
        create_file(TOKEN_IDENTIFIER,
                    STR_BUF_NON_HEAP("intel_i9_processor"),
                    5,
                    5,
                    9),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 5, 23, 9),
        // i8
        create_file(TOKEN_VOID, StrBuf_null(), 4, 1, 8),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i8_func"), 4, 6, 8),
        create_file(TOKEN_LBRACKET, StrBuf_null(), 4, 13, 8),
        create_file(TOKEN_VOID, StrBuf_null(), 4, 14, 8),
        create_file(TOKEN_RBRACKET, StrBuf_null(), 4, 18, 8),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 4, 19, 8),
        // i7
        create_file(TOKEN_ENUM, StrBuf_null(), 4, 1, 7),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i7_enum"), 4, 6, 7),
        create_file(TOKEN_LBRACE, StrBuf_null(), 4, 14, 7),
        create_file(TOKEN_IDENTIFIER,
                    STR_BUF_NON_HEAP("I7_ENUM_ONLY_VALUE"),
                    4,
                    16,
                    7),
        create_file(TOKEN_RBRACE, StrBuf_null(), 4, 35, 7),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 4, 36, 7),
        // i6
        create_file(TOKEN_CHAR, StrBuf_null(), 7, 1, 6),
        create_file(TOKEN_IDENTIFIER,
                    STR_BUF_NON_HEAP("i6_second_test"),
                    7,
                    6,
                    6),
        create_file(TOKEN_LBRACKET, StrBuf_null(), 7, 20, 6),
        create_file(TOKEN_SHORT, StrBuf_null(), 7, 21, 6),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("s"), 7, 27, 6),
        create_file(TOKEN_RBRACKET, StrBuf_null(), 7, 28, 6),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 7, 29, 6),
        // i5
        create_file(TOKEN_TYPEDEF, StrBuf_null(), 4, 1, 5),
        create_file(TOKEN_STRUCT, StrBuf_null(), 4, 9, 5),
        create_file(TOKEN_LBRACE, StrBuf_null(), 4, 16, 5),
        create_file(TOKEN_CHAR, StrBuf_null(), 5, 5, 5),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("c"), 5, 10, 5),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 5, 11, 5),
        create_file(TOKEN_VOID, StrBuf_null(), 6, 5, 5),
        create_file(TOKEN_ASTERISK, StrBuf_null(), 6, 9, 5),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("ptr"), 6, 11, 5),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 6, 14, 5),
        create_file(TOKEN_RBRACE, StrBuf_null(), 7, 1, 5),
        create_file(TOKEN_IDENTIFIER,
                    STR_BUF_NON_HEAP("I5TypedefStruct"),
                    7,
                    3,
                    5),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 7, 18, 5),
        // i4
        create_file(TOKEN_VOID, StrBuf_null(), 4, 1, 4),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i4_test"), 4, 6, 4),
        create_file(TOKEN_LBRACKET, StrBuf_null(), 4, 13, 4),
        create_file(TOKEN_VOID, StrBuf_null(), 4, 14, 4),
        create_file(TOKEN_RBRACKET, StrBuf_null(), 4, 18, 4),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 4, 19, 4),
        // i3
        create_file(TOKEN_STRUCT, StrBuf_null(), 5, 1, 3),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("i3_struct"), 5, 8, 3),
        create_file(TOKEN_LBRACE, StrBuf_null(), 5, 18, 3),
        create_file(TOKEN_INT, StrBuf_null(), 6, 5, 3),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("n"), 6, 9, 3),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 6, 10, 3),
        create_file(TOKEN_INT, StrBuf_null(), 7, 5, 3),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("m"), 7, 9, 3),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 7, 10, 3),
        create_file(TOKEN_RBRACE, StrBuf_null(), 8, 1, 3),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 8, 2, 3),
        // i2
        create_file(TOKEN_VOID, StrBuf_null(), 4, 1, 2),
        create_file(TOKEN_IDENTIFIER,
                    STR_BUF_NON_HEAP("i2_func_decl"),
                    4,
                    6,
                    2),
        create_file(TOKEN_LBRACKET, StrBuf_null(), 4, 18, 2),
        create_file(TOKEN_INT, StrBuf_null(), 4, 19, 2),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("m"), 4, 23, 2),
        create_file(TOKEN_COMMA, StrBuf_null(), 4, 24, 2),
        create_file(TOKEN_INT, StrBuf_null(), 4, 26, 2),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("n"), 4, 30, 2),
        create_file(TOKEN_RBRACKET, StrBuf_null(), 4, 31, 2),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 4, 32, 2),
        // i1
        create_file(TOKEN_VOID, StrBuf_null(), 12, 1, 1),
        create_file(TOKEN_IDENTIFIER,
                    STR_BUF_NON_HEAP("i1_test_func"),
                    12,
                    6,
                    1),
        create_file(TOKEN_LBRACKET, StrBuf_null(), 12, 18, 1),
        create_file(TOKEN_VOID, StrBuf_null(), 12, 19, 1),
        create_file(TOKEN_RBRACKET, StrBuf_null(), 12, 23, 1),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 12, 24, 1),
        // start.c
        create_file(TOKEN_CHAR, StrBuf_null(), 6, 1, 0),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("start_char"), 6, 6, 0),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 6, 16, 0),
        // long_include_that_will_not_fit_in_static_buffer.h
        create_file(TOKEN_INT, StrBuf_null(), 2, 1, 25),
        create_file(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("long_test"), 2, 5, 25),
        create_file(TOKEN_SEMICOLON, StrBuf_null(), 2, 14, 25),
    };
    check_token_arr_file(filename, expected, ARR_LEN(expected));
}

TEST(preproc_if) {
    CStr filename = CSTR_LIT("../frontend/test/files/preproc_if.c");
    const Token expected[] = {
        create(TOKEN_CHAR, StrBuf_null(), 9, 1),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("b"), 9, 6),
        create(TOKEN_ASSIGN, StrBuf_null(), 9, 8),
        create_tok_val(Value_create_sint(VALUE_I, 100), 9, 10),
        create(TOKEN_SEMICOLON, StrBuf_null(), 9, 13),
        create(TOKEN_CONST, StrBuf_null(), 19, 1),
        create(TOKEN_CHAR, StrBuf_null(), 19, 7),
        create(TOKEN_ASTERISK, StrBuf_null(), 19, 11),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("not_d_var"), 19, 13),
        create(TOKEN_ASSIGN, StrBuf_null(), 19, 23),
        create_tok_str_lit(STR_LIT_DEFAULT, STR_BUF_NON_HEAP("not d"), 19, 25),
        create(TOKEN_SEMICOLON, StrBuf_null(), 19, 32),
        create(TOKEN_INT, StrBuf_null(), 29, 1),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("the_world_is_in_order"), 29, 5),
        create(TOKEN_ASSIGN, StrBuf_null(), 29, 27),
        create_tok_val(Value_create_sint(VALUE_I, 60), 29, 29),
        create(TOKEN_SEMICOLON, StrBuf_null(), 29, 31),
        create(TOKEN_INT, StrBuf_null(), 49, 1),
        create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("inner_elif"), 49, 5),
        create(TOKEN_ASSIGN, StrBuf_null(), 49, 16),
        create_tok_val(Value_create_sint(VALUE_I, 23), 49, 18),
        create(TOKEN_SEMICOLON, StrBuf_null(), 49, 20),
    };
    check_token_arr_file(filename, expected, ARR_LEN(expected));
}

TEST(hex_literal_or_var) {
    {
        CStr code = CSTR_LIT("vare-10");

        const Token expected[] = {
            create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("vare"), 1, 1),
            create(TOKEN_SUB, StrBuf_null(), 1, 5),
            create_tok_val(Value_create_sint(VALUE_I, 10), 1, 6),
        };
        check_token_arr_str(code, expected, ARR_LEN(expected));
    }
    {
        CStr code = CSTR_LIT("var2e-10");

        const Token expected[] = {
            create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("var2e"), 1, 1),
            create(TOKEN_SUB, StrBuf_null(), 1, 6),
            create_tok_val(Value_create_sint(VALUE_I, 10), 1, 7),
        };
        check_token_arr_str(code, expected, ARR_LEN(expected));
    }
    {
        CStr code = CSTR_LIT("var2p-10");

        const Token expected[] = {
            create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("var2p"), 1, 1),
            create(TOKEN_SUB, StrBuf_null(), 1, 6),
            create_tok_val(Value_create_sint(VALUE_I, 10), 1, 7),
        };
        check_token_arr_str(code, expected, ARR_LEN(expected));
    }
}

TEST(dot_float_literal_or_op) {
    {
        CStr code = CSTR_LIT("int n = .001");

        const Token expected[] = {
            create(TOKEN_INT, StrBuf_null(), 1, 1),
            create(TOKEN_IDENTIFIER, STR_BUF_NON_HEAP("n"), 1, 5),
            create(TOKEN_ASSIGN, StrBuf_null(), 1, 7),
            create_tok_val(Value_create_float(VALUE_D, .001), 1, 9),
        };
        check_token_arr_str(code, expected, ARR_LEN(expected));
    }
}

TEST_SUITE_BEGIN(tokenizer){
    REGISTER_TEST(simple),
    REGISTER_TEST(file),
    REGISTER_TEST(include),
    REGISTER_TEST(preproc_if),
    REGISTER_TEST(hex_literal_or_var),
    REGISTER_TEST(dot_float_literal_or_op),
} TEST_SUITE_END()

    static Token create_file(TokenKind kind,
                             StrBuf spelling,
                             size_t line,
                             size_t idx,
                             size_t file) {
    return (Token){
        .kind = kind,
        .spelling = spelling,
        .loc =
            {
                .file_idx = file,
                .file_loc = {line, idx},
            },
    };
}

static Token create(TokenKind kind,
                    StrBuf spelling,
                    size_t line,
                    size_t index) {
    return create_file(kind, spelling, line, index, 0);
}

static Token create_tok_val(Value val, size_t line, size_t index) {
    return (Token){
        .kind = ValueKind_is_float(val.kind) ? TOKEN_F_CONSTANT
                                             : TOKEN_I_CONSTANT,
        .val = val,
        .loc =
            {
                .file_idx = 0,
                .file_loc = {line, index},
            },
    };
}

static Token create_tok_str_lit(StrLitKind kind,
                                StrBuf cont,
                                size_t line,
                                size_t index) {
    return (Token){
        .kind = TOKEN_STRING_LITERAL,
        .str_lit = (StrLit){kind, cont},
        .loc =
            {
                .file_idx = 0,
                .file_loc = {line, index},
            },
    };
}

static void check_size(const Token* tokens, size_t expected) {
    size_t size = 0;
    const Token* it = tokens;

    while (it->kind != TOKEN_INVALID) {
        ++it;
        ++size;
    }
    ASSERT_SIZE_T(size, expected);
}

static void check_token(const Token* t, const Token* expected) {
    ASSERT_TOKEN_KIND(t->kind, expected->kind);

    if (t->kind == TOKEN_I_CONSTANT) {
        ASSERT_VALUE_KIND(t->val.kind, expected->val.kind);
        if (ValueKind_is_sint(t->val.kind)) {
            ASSERT_I64(t->val.sint_val, expected->val.sint_val);
        } else {
            ASSERT_U64(t->val.uint_val, expected->val.uint_val);
        }
    } else if (t->kind == TOKEN_F_CONSTANT) {
        ASSERT_VALUE_KIND(t->val.kind, expected->val.kind);
        ASSERT_DOUBLE(t->val.float_val, expected->val.float_val, 0.0001);
    } else if (t->kind == TOKEN_STRING_LITERAL) {
        ASSERT_STR_LIT_KIND(t->str_lit.kind, expected->str_lit.kind);
        ASSERT_STR(StrBuf_as_str(&t->str_lit.contents),
                   StrBuf_as_str(&expected->str_lit.contents));
    } else {
        ASSERT_STR(StrBuf_as_str(&t->spelling),
                   StrBuf_as_str(&expected->spelling));
    }

    ASSERT_SIZE_T(t->loc.file_loc.line, expected->loc.file_loc.line);
    ASSERT_SIZE_T(t->loc.file_loc.index, expected->loc.file_loc.index);
    ASSERT_SIZE_T(t->loc.file_idx, expected->loc.file_idx);
}

static void compare_tokens(const Token* got,
                           const Token* expected,
                           size_t len) {
    for (size_t i = 0; i < len; ++i) {
        check_token(&got[i], &expected[i]);
    }
}

static void check_token_arr_helper(CStr file_or_code,
                                   const Token* expected,
                                   size_t expected_len,
                                   PreprocRes (*func)(CStr)) {
    PreprocRes preproc_res = func(file_or_code);
    ASSERT_NOT_NULL(preproc_res.toks);
    check_size(preproc_res.toks, expected_len);

    compare_tokens(preproc_res.toks, expected, expected_len);

    PreprocRes_free(&preproc_res);
}

static void check_token_arr_file(CStr filename,
                                 const Token* expected,
                                 size_t expected_len) {
    check_token_arr_helper(filename, expected, expected_len, tokenize);
}

static PreprocRes tokenize_string_wrapper(CStr code) {
    return tokenize_string(CStr_as_str(code), STR_LIT("code.c"));
}

static void check_token_arr_str(CStr code,
                                const Token* expected,
                                size_t expected_len) {
    check_token_arr_helper(code,
                           expected,
                           expected_len,
                           tokenize_string_wrapper);
}
