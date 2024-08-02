#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stddef.h>

#include "frontend/Token.h"

#include "frontend/preproc/preproc.h"

#include "util/macro_util.h"

#include "testing/testing.h"

#define ASSERT_TOKEN_KIND(got, expected)                                       \
    do {                                                                       \
        TokenKind got_kind = (got), expected_kind = (expected);                \
        if (got_kind != expected_kind) {                                       \
            PRINT_ASSERT_ERR("Expected {Str} but got {Str}",                   \
                             TokenKind_str(expected_kind),                     \
                             TokenKind_str(got_kind));                         \
        }                                                                      \
    } while (0)

#define ASSERT_VALUE_KIND(got, expected)                                       \
    do {                                                                       \
        ValueKind got_kind = (got), expected_kind = (expected);                \
        if (got_kind != expected_kind) {                                       \
            PRINT_ASSERT_ERR("Expected {Str} but got {Str}",                   \
                             ValueKind_str(expected_kind),                     \
                             ValueKind_str(got_kind));                         \
        }                                                                      \
    } while (0)

#define ASSERT_STR_LIT_KIND(got, expected)                                     \
    do {                                                                       \
        StrLitKind got_kind = (got), expected_kind = (expected);               \
        if (got_kind != expected_kind) {                                       \
            PRINT_ASSERT_ERR("Expected {Str} but got {Str}",                   \
                             StrLitKind_str(expected_kind),                    \
                             StrLitKind_str(got_kind));                        \
        }                                                                      \
    } while (0)

typedef struct TestPreprocRes {
    TokenArr toks;
    FileInfo file_info;
} TestPreprocRes;

TestPreprocRes tokenize(CStr file);
TestPreprocRes tokenize_string(Str str, Str file);

void TestPreprocRes_free(const TestPreprocRes* res);

void test_compare_files(CStr got_file, CStr ex_file);

#define STR_BUF_NON_HEAP(lit) StrBuf_non_heap(ARR_LEN(lit) - 1, lit)

StrBuf StrBuf_non_heap(uint32_t len, const char* str);

#endif

