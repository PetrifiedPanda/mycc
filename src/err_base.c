#include "err_base.h"

#include <stdio.h>
#include <stdlib.h>

void print_err_base(const struct err_base* err) {
    printf("%s(%zu, %zu):\n",
           err->loc.file,
           err->loc.file_loc.line,
           err->loc.file_loc.index);
}

void free_err_base(struct err_base* err) {
    free_source_loc(&err->loc);
}

