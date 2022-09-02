#include "frontend/arch_type_info.h"

#include "util/annotations.h"

struct arch_type_info get_arch_type_info(enum arch a) {
    switch (a) {
        case ARCH_X86_64:
            return (struct arch_type_info){
                .int_info = {
                    .sint_size = 2,
                    .int_size = 4,
                    .lint_size = 8,
                    .llint_size = 8,
                },
                .float_info = {
                    .float_size = 4,
                    .double_size = 8,
                    .ldouble_size = 8,
                },
            };
    }
    UNREACHABLE();
}
