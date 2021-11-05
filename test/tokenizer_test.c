#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "tokenizer.h"
#include "error.h"

static void simple_test();
static void file_test();

void tokenizer_test() {
    simple_test();
    file_test();
    printf("Tokenizer test successful\n");;
}

static void check_size(const Token* tokens, size_t expected) {
    size_t size = 0;
    const Token* it = tokens;
    
    while (it->type != INVALID) {
        ++it;
        ++size;
    }
    assert(size == expected);
}

static void check_file(const Token* tokens, const char* file) {
    for (const Token* it = tokens; it->type != INVALID; ++it) {
        assert(strcmp(it->file, file) == 0);
    }
}

static Token create(TokenType type, const char* spelling, size_t line, size_t index) {
    return (Token){type, (char*)spelling, NULL, (SourceLocation){line, index}};
}

static void check_token(const Token* t, const Token* expected) {
    assert(t->type == expected->type);
    if (t->spelling == NULL || expected->spelling == NULL) {
        assert(t->spelling == expected->spelling);
    } else {
        assert(strcmp(t->spelling, expected->spelling) == 0);
    }
    assert(t->source_loc.line == expected->source_loc.line);
    assert(t->source_loc.index == expected->source_loc.index);
}

static void compare_tokens(const Token* got, const Token* expected, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        check_token(&got[i], &expected[i]);
    }
}

static void simple_test() {
    const char* code = 
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
        "int arr[1 ? 100 : 1000];\n";
    
    const char* filename = "not_a_file.c";
    Token* tokens = tokenize(code, filename);
    assert(get_last_error() == ERR_NONE);
    assert(tokens);
    
    enum { EXPECTED_SIZE = 57 };
    check_size(tokens, EXPECTED_SIZE);
    check_file(tokens, filename);

    Token expected[EXPECTED_SIZE] = {
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
        create(STRING_LITERAL, "L\"Long string literal to check if long strings work\"", 10, 1),
        create(SEMICOLON, NULL, 10, 53),
        create(INT, NULL, 11, 1),
        create(IDENTIFIER, "n", 11, 5),
        create(ASSIGN, NULL, 11, 7),
        create(CONSTANT, "0x123213", 11, 9),
        create(ADD, NULL, 11, 18),
        create(CONSTANT, "132", 11, 20),
        create(LEFT_OP, NULL, 11, 24),
        create(CONSTANT, "32", 11, 27),
        create(RIGHT_OP, NULL, 11, 30),
        create(CONSTANT, "0x123", 11, 33),
        create(SUB, NULL, 11, 39),
        create(CONSTANT, "0123", 11, 41),
        create(DIV, NULL, 11, 46),
        create(CONSTANT, "12", 11, 48),
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
        create(CONSTANT, "1", 13, 9),
        create(QMARK, NULL, 13, 11),
        create(CONSTANT, "100", 13, 13),
        create(COLON, NULL, 13, 17),
        create(CONSTANT, "1000", 13, 19),
        create(RINDEX, NULL, 13, 23),
        create(SEMICOLON, NULL, 13, 24)
    };
    
    compare_tokens(tokens, expected, EXPECTED_SIZE);

    free_tokenizer_result(tokens);
}

static char* read_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    assert(f);
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* res = malloc(sizeof(char) * (fsize + 1));
    assert(res);

    fread(res, 1, fsize, f);
    res[fsize] = '\0';

    fclose(f);
    return res;
}

static void file_test() {
    const char* filename = "../test/files/no_preproc.c";
    char* code = read_file(filename); 
    assert(code);
    
    Token* tokens = tokenize(code, filename);
    assert(get_last_error() == ERR_NONE);
    assert(tokens);
    
    enum { EXPECTED_SIZE = 473 };
    check_size(tokens, EXPECTED_SIZE); // No idea if this is correct
    check_file(tokens, filename);
    
    Token expected[] = {
        create(TYPEDEF, NULL, 3, 1),
        create(STRUCT, NULL, 3, 9),
        create(LBRACE, NULL, 3, 16),
        create(VOLATILE, NULL, 4, 5),
        create(INT, NULL, 4, 14),
        create(ASTERISK, NULL, 4, 17),
        create(IDENTIFIER, "ptr", 4, 19),
        create(SEMICOLON, NULL, 4, 22),
        create(CONST, NULL, 5, 5),
        create(CHAR, NULL, 5, 11),
        create(ASTERISK, NULL, 5, 16),
        create(IDENTIFIER, "str", 5, 18),
        create(SEMICOLON, NULL, 5, 21),
        create(RBRACE, NULL, 6, 1),
        create(IDENTIFIER, "MyStruct", 6, 3),
        create(SEMICOLON, NULL, 6, 11),
        create(UNION, NULL, 8, 1),
        create(IDENTIFIER, "my_union", 8, 7),
        create(LBRACE, NULL, 8, 16),
        create(SHORT, NULL, 9, 5),
        create(INT, NULL, 9, 11),
        create(IDENTIFIER, "i", 9, 15),
        create(SEMICOLON, NULL, 9, 16),
        create(FLOAT, NULL, 10, 5),
        create(IDENTIFIER, "f", 10, 11),
        create(SEMICOLON, NULL, 10, 12),
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
        create(INT, NULL, 20, 8),
        create(IDENTIFIER, "do_shit", 20, 12),
        create(LBRACKET, NULL, 20, 19),
        create(RBRACKET, NULL, 20, 20),
        create(SEMICOLON, NULL, 20, 21),
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
        create(CONSTANT, "1000l", 33, 21),
        create(MOD, NULL, 33, 27),
        create(CONSTANT, "5", 33, 29),
        create(SEMICOLON, NULL, 33, 30),
        create(BREAK, NULL, 34, 13),
        create(SEMICOLON, NULL, 34, 18),
        create(DEFAULT, NULL, 36, 9),
        create(COLON, NULL, 36, 16),
        create(IDENTIFIER, "value", 37, 13),
        create(ASSIGN, NULL, 37, 19),
        create(CONSTANT, "30l", 37, 21),
        create(SEMICOLON, NULL, 37, 24),
        create(BREAK, NULL, 38, 13),
        create(SEMICOLON, NULL, 38, 18),
        create(RBRACE, NULL, 39, 5),
        create(IDENTIFIER, "MyStruct", 46, 5),
        create(IDENTIFIER, "s", 46, 14),
        create(ASSIGN, NULL, 46, 16),
        create(LBRACE, NULL, 46, 18),
        create(CONSTANT, "0", 46, 19),
        create(COMMA, NULL, 46, 20),
        create(STRING_LITERAL, "L\"Hello there\"", 46, 22),
        create(RBRACE, NULL, 46, 36),
        create(SEMICOLON, NULL, 46, 37),
        create(INT, NULL, 47, 5),
        create(IDENTIFIER, "integer", 47, 9),
        create(ASSIGN, NULL, 47, 17),
        create(CONSTANT, "01000", 47, 19),
        create(SUB, NULL, 47, 25),
        create(CONSTANT, "10", 47, 27),
        create(SEMICOLON, NULL, 47, 29),
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
        create(STRING_LITERAL, "L\"Lstrings seem to be int pointers\"", 52, 25),
        create(COMMA, NULL, 52, 60),
        create(STRING_LITERAL, "\"doot\"", 52, 62),
        create(RBRACE, NULL, 52, 68),
        create(SEMICOLON, NULL, 52, 69),
        create(UNION, NULL, 54, 5),
        create(IDENTIFIER, "my_union", 54, 11),
        create(IDENTIFIER, "soviet_union", 54, 20),
        create(SEMICOLON, NULL, 54, 32),
        create(IDENTIFIER, "soviet_union", 55, 5),
        create(DOT, NULL, 55, 17),
        create(IDENTIFIER, "i", 55, 18),
        create(ASSIGN, NULL, 55, 20),
        create(CONSTANT, "0x1000", 55, 22),
        create(ADD, NULL, 55, 29),
        create(CONSTANT, "033242", 55, 31),
        create(SEMICOLON, NULL, 55, 37),
        create(INT, NULL, 56, 5),
        create(IDENTIFIER, "n", 56, 9),
        create(SEMICOLON, NULL, 56, 10),
        create(GOTO, NULL, 57, 5),
        create(IDENTIFIER, "my_cool_label", 57, 10),
        create(SEMICOLON, NULL, 57, 23),
        create(RETURN, NULL, 59, 5),
        create(CONSTANT, "69", 59, 12),
        create(SEMICOLON, NULL, 59, 14),
        create(IDENTIFIER, "my_cool_label", 61, 1),
        create(COLON, NULL, 61, 14),
        create(IDENTIFIER, "n", 62, 5),
        create(ASSIGN, NULL, 62, 7),
        create(LBRACKET, NULL, 62, 9),
        create(INT, NULL, 62, 10),
        create(RBRACKET, NULL, 62, 13),
        create(IDENTIFIER, "soviet_union", 62, 14),
        create(DOT, NULL, 62, 26),
        create(IDENTIFIER, "f", 62, 27),
        create(SEMICOLON, NULL, 62, 28),
        create(RBRACE, NULL, 63, 1)
    };
    enum { TMP_SIZE = sizeof(expected) / sizeof(Token) };

    printf("Warning: Only %d of %d elements are being compared\n", TMP_SIZE, EXPECTED_SIZE);

    compare_tokens(tokens, expected, TMP_SIZE);

    free(code);
    free_tokenizer_result(tokens);
}
