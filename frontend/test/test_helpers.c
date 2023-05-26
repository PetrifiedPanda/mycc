#include "test_helpers.h"

#include "util/file.h"
#include "util/mem.h"

#include "testing/asserts.h"

#include "frontend/preproc/preproc.h"

PreprocRes tokenize(const char* file) {
    PreprocErr err = PreprocErr_create();
    PreprocRes res = preproc(file, &err);
    ASSERT_NOT_NULL(res.toks);
    ASSERT_NOT_NULL(res.file_info.paths);
    ASSERT(err.kind == PREPROC_ERR_NONE);
    const ArchTypeInfo type_info = get_arch_type_info(ARCH_X86_64,
                                                               false);
    ASSERT(convert_preproc_tokens(res.toks, &type_info, &err));
    ASSERT(err.kind == PREPROC_ERR_NONE);
    return res;
}

PreprocRes tokenize_string(const char* str, const char* file) {
    PreprocErr err = PreprocErr_create();
    PreprocRes res = preproc_string(str, file, &err);
    ASSERT_NOT_NULL(res.toks);
    ASSERT(err.kind == PREPROC_ERR_NONE);
    const ArchTypeInfo type_info = get_arch_type_info(ARCH_X86_64,
                                                               false);
    ASSERT(convert_preproc_tokens(res.toks, &type_info, &err));
    ASSERT(err.kind == PREPROC_ERR_NONE);
    return res;
}

void test_compare_files(const char* got_file, const char* ex_file) {
    FILE* got = fopen(got_file, "r");
    FILE* ex = fopen(ex_file, "r");

    enum {
        BUF_LEN = 500
    };
    Str got_str = Str_create_empty();
    Str ex_str = Str_create_empty();
    char got_buf[BUF_LEN] = {0};
    char ex_buf[BUF_LEN] = {0};

    size_t got_len = 0, ex_len = 0;

    size_t line_counter = 1;

    const char* got_line = file_read_line(got,
                                          &got_str,
                                          &got_len,
                                          got_buf,
                                          BUF_LEN);
    const char* ex_line = file_read_line(ex, &ex_str, &ex_len, ex_buf, BUF_LEN);
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

        ++line_counter;

        Str_clear(&got_str);
        Str_clear(&ex_str);
        got_buf[0] = '\0';
        ex_buf[0] = '\0';
        got_len = ex_len = 0;

        got_line = file_read_line(got, &got_str, &got_len, got_buf, BUF_LEN);
        ex_line = file_read_line(ex, &ex_str, &ex_len, ex_buf, BUF_LEN);
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

    Str_free(&got_str);
    Str_free(&ex_str);

    fclose(got);
    fclose(ex);
    remove(got_file);
}

Str str_non_heap(size_t len, const char* str) {
    return (Str){
        ._is_static_buf = false,
        ._cap = len + 1,
        ._len = len,
        ._data = (char*)str,
    };
}

