#include "test_helpers.h"

#include "testing/asserts.h"

#include "frontend/preproc/preproc.h"

TestPreprocRes tokenize(CStr file) {
    PreprocErr err = PreprocErr_create();
    const ArchTypeInfo info = get_arch_type_info(ARCH_X86_64, false);
    PreprocRes res = preproc(file, 0, NULL, &info, &err);
    ASSERT(res.toks.len != 0);
    ASSERT_NOT_NULL(res.file_info.paths);
    ASSERT(err.kind == PREPROC_ERR_NONE);
    TokenArr toks = convert_preproc_tokens(&res.toks, &info, &err);
    ASSERT(toks.len != 0);
    ASSERT(err.kind == PREPROC_ERR_NONE);
    return (TestPreprocRes){
        .toks = toks,
        .file_info = res.file_info,
    };
}

TestPreprocRes tokenize_string(Str str, Str file) {
    PreprocErr err = PreprocErr_create();
    const ArchTypeInfo info = get_arch_type_info(ARCH_X86_64, false);
    PreprocRes res = preproc_string(str, file,
                                    &(PreprocInitialStrings){0},
                                    0, NULL, &info, &err);
    ASSERT(res.toks.len != 0);
    ASSERT(err.kind == PREPROC_ERR_NONE);
    TokenArr toks = convert_preproc_tokens(&res.toks, &info, &err);
    ASSERT(toks.len != 0);
    ASSERT(err.kind == PREPROC_ERR_NONE);
    return (TestPreprocRes){
        .toks = toks,
        .file_info = res.file_info,
    };
}
void TestPreprocRes_free(const TestPreprocRes* res) {
    TokenArr_free(&res->toks);
    FileInfo_free(&res->file_info);
}

void test_compare_files(CStr got_file, CStr ex_file) {
    File got = File_open(got_file, FILE_READ);
    File ex = File_open(ex_file, FILE_READ);

    StrBuf got_str = StrBuf_create_empty();
    StrBuf ex_str = StrBuf_create_empty();

    uint32_t line_counter = 1;

    Str got_line = File_read_line(got, &got_str);
    Str ex_line = File_read_line(ex, &ex_str);
    while (Str_valid(got_line) && Str_valid(ex_line)) {
        if (!Str_eq(got_line, ex_line)) {
            File_close(got);
            File_close(ex);
            PRINT_ASSERT_ERR(
                "Line {u32} of file {u32} differs from expected file "
                "{Str}: Expected {Str} but got {Str}",
                line_counter,
                got_file,
                ex_file,
                ex_line,
                got_line);
        }

        ++line_counter;

        StrBuf_clear(&got_str);
        StrBuf_clear(&ex_str);

        got_line = File_read_line(got, &got_str);
        ex_line = File_read_line(ex, &ex_str);
    }

    if (!Str_valid(got_line) && Str_valid(ex_line)) {
        File_close(got);
        File_close(ex);
        PRINT_ASSERT_ERR("Expected {Str} at line %zu but got to end of file",
                         ex_line,
                         line_counter);
    } else if (Str_valid(got_line) && !Str_valid(ex_line)) {
        File_close(got);
        File_close(ex);
        PRINT_ASSERT_ERR("Expected end of file at line %zu but got {Str}",
                         line_counter,
                         got_line);
    }

    StrBuf_free(&got_str);
    StrBuf_free(&ex_str);

    File_close(got);
    File_close(ex);
    remove(got_file.data);
}

StrBuf StrBuf_non_heap(uint32_t len, const char* str) {
    return (StrBuf){
        ._is_static_buf = false,
        ._cap = len + 1,
        ._len = len,
        ._data = (char*)str,
    };
}

