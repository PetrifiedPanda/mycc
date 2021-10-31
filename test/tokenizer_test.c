#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "tokenizer.h"
#include "error.h"

static void simple_test();

void tokenizer_test() {
    simple_test();
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

static Token create(TokenType type, const char* spelling, size_t line, size_t index) {
    return (Token){type, spelling == NULL ? (char*)get_spelling(type) : (char*)spelling, NULL, (SourceLocation){line, index}};
}

static void check_token(Token t, Token expected) {
    assert(t.type == expected.type);
    assert(strcmp(t.spelling, expected.spelling) == 0);
    assert(t.source_loc.line == expected.source_loc.line);
    assert(t.source_loc.index == expected.source_loc.index);
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
    
    enum { EXPECTED_SIZE = 57 };
    check_size(tokens, EXPECTED_SIZE);

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

    for (size_t i = 0; i < EXPECTED_SIZE; ++i) {
        check_token(tokens[i], expected[i]);
    }

    free_tokenizer_result(tokens);
}
