#ifndef ERR_BASE_H
#define ERR_BASE_H

#include <stdio.h>

#include "file_info.h"
#include "token.h"

struct err_base {
    struct source_loc loc;
};

struct err_base create_err_base(struct source_loc loc);

void print_err_base(FILE* out,
                    const struct file_info* file_info,
                    const struct err_base* err);

#endif

