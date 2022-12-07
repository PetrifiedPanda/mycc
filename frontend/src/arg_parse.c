#include "frontend/arg_parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "util/annotations.h"

enum {
    ARG_PARSE_MAX_ALLOWED_FILES = sizeof(struct cmd_args){0}.files
                                  / sizeof *(struct cmd_args){0}.files,
};

static PRINTF_FORMAT(1, 2) _Noreturn void exit_with_err(const char* format,
                                                        ...) {
    va_list list;
    va_start(list, format);
    vfprintf(stderr, format, list);
    va_end(list);

    exit(EXIT_FAILURE);
}

struct cmd_args parse_cmd_args(int argc, char** argv) {
    struct cmd_args res = {
        .num_files = 0,
        .files = {0},
        .output_file = NULL,
        .action = ARG_ACTION_OUTPUT_TEXT,
    };
    for (int i = 1; i < argc; ++i) {
        const char* item = argv[i];
        if (item[0] == '-') {
            switch (item[1]) {
                case 'o':
                    if (i == argc - 1) {
                        exit_with_err(
                            "-o Option without output file argument\n");
                    }
                    res.output_file = argv[i + 1];
                    ++i;
                    break;
                case 'b':
                    res.action = ARG_ACTION_OUTPUT_BIN;
                    break;
                case 'c':
                    res.action = ARG_ACTION_CONVERT_BIN_TO_TEXT;
                    break;
                default:
                    exit_with_err("Invalid command line option \"-%c\"",
                                  item[1]);
            }
        } else {
            if (res.num_files >= ARG_PARSE_MAX_ALLOWED_FILES) {
                exit_with_err(
                    "Too many files given as argument, maximum is %d\n",
                    ARG_PARSE_MAX_ALLOWED_FILES);
            }
            res.files[res.num_files] = item;
            ++res.num_files;
        }
    }

    if (res.num_files == 0) {
        exit_with_err("%s: no input files\n", argv[0]);
    } else if (res.output_file != NULL && res.num_files > 1) {
        exit_with_err("Cannot write output of multiple sources in one file\n");
    }
    return res;
}
