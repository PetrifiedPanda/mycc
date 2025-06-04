#include "frontend/arg_parse.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <stdnoreturn.h>

#include "util/mem.h"
#include "util/File.h"

static noreturn void exit_with_err_str(Str msg) {
    File_put_str_val(msg, mycc_stderr);
    exit(EXIT_FAILURE);
}
#define exit_with_err(msg) exit_with_err_str(STR_LIT(msg))

static noreturn void exit_with_err_fmt_str(Str format, ...) {
    va_list list;
    va_start(list, format);
    File_printf_varargs_impl(mycc_stderr, format, list);
    va_end(list);

    exit(EXIT_FAILURE);
}
#define exit_with_err_fmt(lit, ...)                                            \
    exit_with_err_fmt_str(STR_LIT(lit), __VA_ARGS__)

CmdArgs parse_cmd_args(int argc, char** argv) {
    CmdArgs res = {
        .num_files = 0,
        .num_include_dirs = 0,
        .files = NULL,
        .include_dirs = NULL,
        .output_file = {0, NULL},
        .action = ARG_ACTION_OUTPUT_TEXT,
    };
    for (int i = 1; i < argc; ++i) {
        const char* item = argv[i];
        if (item[0] == '-') {
            switch (item[1]) {
                case 'o': {
                    if (i == argc - 1) {
                        CmdArgs_free(&res);
                        exit_with_err(
                            "-o Option without output file argument\n");
                    }
                    const size_t sz_len = strlen(argv[i + 1]);
                    const uint32_t len = (uint32_t)sz_len;
                    assert((size_t)len == sz_len);
                    res.output_file = (CStr){len, argv[i + 1]};
                    ++i;
                    break;
                }
                case 'b':
                    res.action = ARG_ACTION_OUTPUT_BIN;
                    break;
                case 'c':
                    res.action = ARG_ACTION_CONVERT_BIN_TO_TEXT;
                    break;
                case 'I': {
                    if (i == argc - 1) {
                        CmdArgs_free(&res);
                        exit_with_err("-I Option without folder argument\n");
                    }
                    ++res.num_include_dirs;
                    res.include_dirs = mycc_realloc(res.include_dirs,
                                                    sizeof *res.include_dirs
                                                        * res.num_include_dirs);
                    const char* include_dir = argv[i + 1];
                    const size_t sz_len = strlen(include_dir);
                    const uint32_t len = (uint32_t)sz_len;
                    assert((size_t)len == sz_len);
                    res.include_dirs[res.num_include_dirs - 1] = (Str){
                        len,
                        include_dir,
                    };
                    ++i;
                    break;
                }
                default:
                    CmdArgs_free(&res);
                    exit_with_err_fmt(
                        "Invalid command line option \"-{char}\"\n",
                        item[1]);
            }
        } else {
            ++res.num_files;
            res.files = mycc_realloc(res.files,
                                     res.num_files * sizeof *res.files);
            const size_t sz_len = strlen(item);
            const uint32_t len = (uint32_t)sz_len;
            assert((size_t)len == sz_len);
            res.files[res.num_files - 1] = (CStr){len, item};
        }
    }

    if (res.num_files == 0) {
        const char* command = argv[0];
        const size_t sz_len = strlen(command);
        const uint32_t len = (uint32_t)sz_len;
        assert((size_t)len == sz_len);
        CmdArgs_free(&res);
        exit_with_err_fmt("{Str}: no input files\n", (Str){len, command});
    } else if (res.output_file.data != NULL && res.num_files > 1) {
        CmdArgs_free(&res);
        exit_with_err("Cannot write output of multiple sources in one file\n");
    }
    return res;
}

void CmdArgs_free(const CmdArgs* args) {
    mycc_free(args->files);
    mycc_free(args->include_dirs);
}
