#include "frontend/arg_parse.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"
#include "util/File.h"

static _Noreturn void exit_with_err(Str format, ...) {
    va_list list;
    va_start(list, format);
    File_printf_varargs_impl(mycc_stderr(), format, list);
    va_end(list);

    exit(EXIT_FAILURE);
}

CmdArgs parse_cmd_args(int argc, char** argv) {
    CmdArgs res = {
        .num_files = 0,
        .files = NULL,
        .output_file = {0, NULL},
        .action = ARG_ACTION_OUTPUT_TEXT,
    };
    for (int i = 1; i < argc; ++i) {
        const char* item = argv[i];
        if (item[0] == '-') {
            switch (item[1]) {
                case 'o':
                    if (i == argc - 1) {
                        exit_with_err(
                            STR_LIT("-o Option without output file argument\n"));
                    }
                    res.output_file = (CStr){strlen(argv[i + 1]), argv[i + 1]};
                    ++i;
                    break;
                case 'b':
                    res.action = ARG_ACTION_OUTPUT_BIN;
                    break;
                case 'c':
                    res.action = ARG_ACTION_CONVERT_BIN_TO_TEXT;
                    break;
                default:
                    exit_with_err(STR_LIT("Invalid command line option \"-{char}\"\n"),
                                  item[1]);
            }
        } else {
            ++res.num_files;
            res.files = mycc_realloc(res.files,
                                     res.num_files * sizeof *res.files);
            res.files[res.num_files - 1] = (CStr){strlen(item), item};
        }
    }

    if (res.num_files == 0) {
        exit_with_err(STR_LIT("{Str}: no input files\n"), (Str){strlen(argv[0]), argv[0]});
    } else if (res.output_file.data != NULL && res.num_files > 1) {
        exit_with_err(STR_LIT("Cannot write output of multiple sources in one file\n"));
    }
    return res;
}

void CmdArgs_free(const CmdArgs* args) {
    mycc_free(args->files);
}
