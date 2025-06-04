#include "testing/asserts.h"

#include "frontend/ast/ast.h"

#include "../test_helpers.h"

TEST(redefine_typedef_error) {
    TestPreprocRes preproc_res = tokenize_string(STR_LIT("typedef int MyInt;\n"
                                                     "typedef char MyInt;\n"),
                                             STR_LIT("a file"), &(PreprocInitialStrings){0});

    ParserErr err = ParserErr_create();

    AST ast = parse_ast(&preproc_res.toks, &err);
    ASSERT_UINT(ast.len, 0);
    ASSERT(err.kind == PARSER_ERR_REDEFINED_SYMBOL);
    ASSERT(err.was_typedef_name);
    const uint32_t val_idx = ast.toks.val_indices[err.err_token_idx];
    const Str got_spell = StrBuf_as_str(
        &ast.toks.identifiers[val_idx]);
    ASSERT_STR(got_spell, STR_LIT("MyInt"));

    AST_free(&ast);
    TestPreprocRes_free(&preproc_res);
}

TEST_SUITE_BEGIN(parser_error){
    REGISTER_TEST(redefine_typedef_error),
} TEST_SUITE_END()
