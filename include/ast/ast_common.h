#ifndef AST_COMMON_H
#define AST_COMMON_H

#include "error.h"

inline void ast_alloc_fail() {
    set_error(ERR_ALLOC_FAIL, "Failed to allocate Node contents");
}

#endif