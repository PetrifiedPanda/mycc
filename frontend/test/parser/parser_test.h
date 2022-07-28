#ifndef PARSER_TEST_H
#define PARSER_TEST_H

#include "testing/testing.h"

TEST_SUITE_DECL(parser_expr);
TEST_SUITE_DECL(parser_statement);
TEST_SUITE_DECL(parser_file);
TEST_SUITE_DECL(parser_state);
TEST_SUITE_DECL(parser_misc);

TEST_SUITE_LIST_BEGIN(parser){
    TEST_SUITE_LIST_ITEM(parser_expr),
    TEST_SUITE_LIST_ITEM(parser_statement),
    TEST_SUITE_LIST_ITEM(parser_file),
    TEST_SUITE_LIST_ITEM(parser_state),
    TEST_SUITE_LIST_ITEM(parser_misc),
} TEST_SUITE_LIST_END(parser);

#endif

