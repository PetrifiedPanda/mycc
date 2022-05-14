#include "err_base.h"

#include <stdio.h>
#include <stdlib.h>

void print_err_base(const struct err_base* err) {
    printf("%s(%zu, %zu):\n", err->file, err->loc.line, err->loc.index);
}

void free_err_base(struct err_base* err) {
    free(err->file);
}

