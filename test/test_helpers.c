#include "test_helpers.h"

#include "util/file.h"

#include "preproc/preproc.h"

#include "test_asserts.h"

struct token* tokenize(const char* file) {
    struct preproc_err err = create_preproc_err();
    struct token* res = preproc(file, &err);
    ASSERT_NOT_NULL(res);
    ASSERT(err.type == PREPROC_ERR_NONE);
    convert_preproc_tokens(res);
    return res;
}

struct token* tokenize_string(const char* str, const char* file) {
    struct preproc_err err = create_preproc_err();
    struct token* res = preproc_string(str, file, &err);
    ASSERT_NOT_NULL(res);
    ASSERT(err.type == PREPROC_ERR_NONE);
    convert_preproc_tokens(res);
    return res;
}

void test_compare_files(const char* got_file, const char* ex_file) {
    FILE* got = fopen(got_file, "r");
    FILE* ex = fopen(ex_file, "r");

    enum {
        BUF_LEN = 500
    };
    char got_buf[BUF_LEN];
    char ex_buf[BUF_LEN];

    char* got_line;
    char* ex_line;

    size_t got_len, ex_len;

    size_t line_counter = 1;
    got_line = file_read_line(got, got_buf, BUF_LEN, &got_len);
    ex_line = file_read_line(ex, ex_buf, BUF_LEN, &ex_len);
    while (got_line != NULL && ex_line != NULL) {

        if (strcmp(got_line, ex_line) != 0) {
            fclose(got);
            fclose(ex);
            PRINT_ASSERT_ERR("Line %zu of file %s differs from expected file "
                             "%s: Expected %s but got %s",
                             line_counter,
                             got_file,
                             ex_file,
                             ex_line,
                             got_line);
        }

        if (got_line != got_buf) {
            free(got_line);
        }

        if (ex_line != ex_buf) {
            free(ex_line);
        }

        ++line_counter;

        got_line = file_read_line(got, got_buf, BUF_LEN, &got_len);
        ex_line = file_read_line(ex, ex_buf, BUF_LEN, &ex_len);
    }

    if (got_line == NULL && ex_line != NULL) {
        PRINT_ASSERT_ERR("Expected %s at line %zu but got to end of file",
                         ex_line,
                         line_counter);
    } else if (got_line != NULL && ex_line == NULL) {
        PRINT_ASSERT_ERR("Expected end of file at line %zu but got %s",
                         line_counter,
                         got_line);
    }

    if (got_line != got_buf) {
        free(got_line);
    }

    if (ex_line != ex_buf) {
        free(ex_line);
    }

    fclose(got);
    fclose(ex);
}

