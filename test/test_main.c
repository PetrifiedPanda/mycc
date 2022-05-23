#include "test.h"

#include "preproc/preproc_test.h"
#include "parser/parser_test.h"

jmp_buf test_jump_buf;

GET_EXTERN_SUITE(error);

TEST_MAIN_BEGIN(8) {
    TEST_MAIN_ADD_LIST(preproc);
    TEST_MAIN_ADD_LIST(parser);
}
TEST_MAIN_END()
