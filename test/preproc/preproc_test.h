#ifndef PREPROC_TEST_H
#define PREPROC_TEST_H

#include "../test.h"

TEST_SUITE_DECL(preproc_macro);
TEST_SUITE_DECL(tokenizer);

TEST_SUITE_LIST_BEGIN(preproc) {
    TEST_SUITE_LIST_ITEM(preproc_macro),
    TEST_SUITE_LIST_ITEM(tokenizer)
} TEST_SUITE_LIST_END(preproc);

#endif
