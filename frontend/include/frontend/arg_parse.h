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
    uint32_t num_files;
    uint32_t num_include_dirs;
    CStr* files;
    Str* include_dirs;
    CStr output_file;
    ArgAction action;
    bool enable_new_parser;
} CmdArgs;

CmdArgs parse_cmd_args(int argc, char** argv);

void CmdArgs_free(const CmdArgs* args);

#endif

