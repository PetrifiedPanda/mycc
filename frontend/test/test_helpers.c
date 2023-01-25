#include "test_helpers.h"

#include "testing/asserts.h"

#include "util/file.h"

#include "frontend/preproc/preproc.h"

struct preproc_res tokenize(const char* file) {
    struct preproc_err err = create_preproc_err();
    struct preproc_res res = preproc(file, &err);
    ASSERT_NOT_NULL(res.toks);
    ASSERT_NOT_NULL(res.file_info.paths);
    ASSERT(err.type == PREPROC_ERR_NONE);
    const struct arch_type_info type_info = get_arch_type_info(ARCH_X86_64, false);
    ASSERT(convert_preproc_tokens(res.toks, &type_info, &err));
    ASSERT(err.type == PREPROC_ERR_NONE);
    return res;
}

struct preproc_res tokenize_string(const char* str, const char* file) {
    struct preproc_err err = create_preproc_err();
    struct preproc_res res = preproc_string(str, file, &err);
    ASSERT_NOT_NULL(res.toks);
    ASSERT(err.type == PREPROC_ERR_NONE);
    const struct arch_type_info type_info = get_arch_type_info(ARCH_X86_64, false);
    ASSERT(convert_preproc_tokens(res.toks, &type_info, &err));
    ASSERT(err.type == PREPROC_ERR_NONE);
    return res;
}

/*
 * read only one line, as we do not need the functionality of reading into an
 * already existing string
 */
static char* file_read_single_line(FILE* file,
                                   char* static_buf,
                                   size_t static_buf_len,
                                   size_t* res_len) {
    char* res = NULL;
    *res_len = 0;
    file_read_line(file, &res, res_len, static_buf, static_buf_len);
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

    size_t got_len, ex_len;

    size_t line_counter = 1;

    char* got_line = file_read_single_line(got, got_buf, BUF_LEN, &got_len);
    char* ex_line = file_read_single_line(ex, ex_buf, BUF_LEN, &ex_len);
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

        got_line = file_read_single_line(got, got_buf, BUF_LEN, &got_len);
        ex_line = file_read_single_line(ex, ex_buf, BUF_LEN, &ex_len);
    }

    if (got_line == NULL && ex_line != NULL) {
        fclose(got);
        fclose(ex);
        PRINT_ASSERT_ERR("Expected %s at line %zu but got to end of file",
                         ex_line,
                         line_counter);
    } else if (got_line != NULL && ex_line == NULL) {
        fclose(got);
        fclose(ex);
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
    remove(got_file);
}

struct str str_non_heap(size_t len, const char* str) {
    return (struct str){
        ._is_static_buf = false,
        ._cap = len + 1,
        ._len = len,
        ._data = (char*)str,
    };
}

