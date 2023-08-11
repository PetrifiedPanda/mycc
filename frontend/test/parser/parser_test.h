#ifndef PARSER_TEST_H
#define PARSER_TEST_H

#include "testing/testing.h"

TEST_SUITE_DECL(parser_file);
TEST_SUITE_DECL(ParserState);
TEST_SUITE_DECL(parser_error);

TEST_SUITE_LIST_BEGIN(parser){
    TEST_SUITE_LIST_ITEM(parser_file),
    TEST_SUITE_LIST_ITEM(ParserState),
    TEST_SUITE_LIST_ITEM(parser_error),
} TEST_SUITE_LIST_END(parser);

#endif

