#include "test.h"

#include "preproc/preproc_test.h"
#include "parser/parser_test.h"
#include "util/util_test.h"

jmp_buf test_jump_buf;

TEST_MAIN_BEGIN(11) {
    TEST_MAIN_ADD_LIST(util);
    TEST_MAIN_ADD_LIST(preproc);
    TEST_MAIN_ADD_LIST(parser);
}
TEST_MAIN_END()

