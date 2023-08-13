#ifndef MYCC_FRONTEND_ARG_PARSE_H
#define MYCC_FRONTEND_ARG_PARSE_H

#include <stdbool.h>

#include "util/Str.h"

typedef enum {
    ARG_ACTION_OUTPUT_TEXT,
    ARG_ACTION_OUTPUT_BIN,
    ARG_ACTION_CONVERT_BIN_TO_TEXT,
} ArgAction;

typedef struct {
    int num_files;
    CStr* files;
    int num_include_dirs;
    Str* include_dirs;
    CStr output_file;
    ArgAction action;
} CmdArgs;

CmdArgs parse_cmd_args(int argc, char** argv);

void CmdArgs_free(const CmdArgs* args);

#endif

