#ifndef PREPROC_TEST_H
#define PREPROC_TEST_H

#include "testing/testing.h"

TEST_SUITE_DECL(tokenizer);
TEST_SUITE_DECL(tokenizer_error);
TEST_SUITE_DECL(preproc_macro_expansion);
TEST_SUITE_DECL(preproc_macro_parser);
TEST_SUITE_DECL(num_parser);

TEST_SUITE_LIST_BEGIN(preproc) {
    TEST_SUITE_LIST_ITEM(tokenizer),
    TEST_SUITE_LIST_ITEM(tokenizer_error),
    TEST_SUITE_LIST_ITEM(preproc_macro_expansion),
    TEST_SUITE_LIST_ITEM(preproc_macro_parser),
    TEST_SUITE_LIST_ITEM(num_parser),
} TEST_SUITE_LIST_END(preproc);

#endif
