#ifndef ERR_BASE_H
#define ERR_BASE_H

#include <stdio.h>

#include "token.h"

struct err_base {
    struct source_loc loc;
};

struct err_base create_err_base(struct source_loc* loc);
struct err_base create_err_base_copy(const struct source_loc* loc);

void print_err_base(FILE* out, const struct err_base* err);

void free_err_base(struct err_base* err);

#endif

