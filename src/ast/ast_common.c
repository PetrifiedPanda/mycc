#include "ast/ast_common.h"

void ast_alloc_fail() {
    set_error(ERR_ALLOC_FAIL, "Failed to allocate Node contents");
}