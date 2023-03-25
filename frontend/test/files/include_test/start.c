
extern int n;

#include "i1.h"

char start_char;
#define INCLUDE_LOC "long_include_name_that_will_not_fit_in_static_buffer.h"
#include INCLUDE_LOC

#include "i1.h"
