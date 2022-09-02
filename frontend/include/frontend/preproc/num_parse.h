#ifndef NUM_PARSE_H
#define NUM_PARSE_H

#include "frontend/value.h"
#include "frontend/arch_type_info.h"

#include "frontend/preproc/preproc_err.h"

struct value parse_num_constant(const char* spell,
                                size_t len,
                                struct preproc_err* err,
                                const struct arch_int_info* int_info);

#endif

