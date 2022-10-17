#include "frontend/arch_type_info.h"

#include <assert.h>

#include "util/annotations.h"

struct arch_type_info get_arch_type_info(enum arch a, bool is_windows) {
    // if x86 supported add it to assert
    assert(a == ARCH_X86_64 || !is_windows);
    switch (a) {
        case ARCH_X86_64: {
            struct arch_int_info int_info;
            if (is_windows) {
                int_info = (struct arch_int_info){
                    .sint_size = 2,
                    .int_size = 4,
                    .lint_size = 4,
                    .llint_size = 8,
                };
            } else {
                int_info = (struct arch_int_info){
                    .sint_size = 2,
                    .int_size = 4,
                    .lint_size = 8,
                    .llint_size = 8,
                };
            }

            return (struct arch_type_info){
                .int_info = int_info,
                .float_info = {
                    .float_size = 4,
                    .double_size = 8,
                    .ldouble_size = 8,
                },
            };
        }
    }
    UNREACHABLE();
}

