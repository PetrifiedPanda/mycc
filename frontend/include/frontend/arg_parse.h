#ifndef ARG_PARSE_H
#define ARG_PARSE_H

#include <stdbool.h>

enum {
    ARG_PARSE_MAX_ALLOWED_FILES = 32,
};

enum arg_action {
    ARG_ACTION_OUTPUT_TEXT,
    ARG_ACTION_OUTPUT_BIN,
    ARG_ACTION_CONVERT_BIN_TO_TEXT,
};

struct cmd_args {
    int num_files;
    const char* files[ARG_PARSE_MAX_ALLOWED_FILES];
    const char* output_file;
    enum arg_action action;
};

struct cmd_args parse_cmd_args(int argc, char** argv);

#endif

