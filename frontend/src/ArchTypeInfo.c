#include "frontend/ArchTypeInfo.h"

#include <assert.h>

#include "util/macro_util.h"

ArchTypeInfo get_arch_type_info(TargetArch a, bool is_windows) {
    // if x86 supported add it to assert
    assert(a == ARCH_X86_64 || !is_windows);
    switch (a) {
        case ARCH_X86_64: {
            ArchIntInfo int_info;
            ArchFloatInfo float_info;
            if (is_windows) {
                int_info = (ArchIntInfo){
                    .wchar_t_size = 2,
                    .sint_size = 2,
                    .int_size = 4,
                    .lint_size = 4,
                    .llint_size = 8,
                };

                float_info = (ArchFloatInfo){
                    .float_size = 4,
                    .double_size = 8,
                    .ldouble_size = 8,
                };
            } else {
                int_info = (ArchIntInfo){
                    .wchar_t_size = 4,
                    .sint_size = 2,
                    .int_size = 4,
                    .lint_size = 8,
                    .llint_size = 8,
                };
                float_info = (ArchFloatInfo){
                    .float_size = 4,
                    .double_size = 8,
                    .ldouble_size = 16,
                };
            }

            return (ArchTypeInfo){
                .bits_in_char = 8,
                .int_info = int_info,
                .float_info = float_info, 
            };
        }
    }
    UNREACHABLE();
}

