#ifndef ERR_BASE_H
#define ERR_BASE_H

#include "token.h"

struct err_base {
    char* file;
    struct file_loc loc;
};

void print_err_base(const struct err_base* err);

void free_err_base(struct err_base* err);

#endif

