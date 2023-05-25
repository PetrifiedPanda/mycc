#ifndef ARG_PARSE_H
#define ARG_PARSE_H

#include <stdbool.h>

typedef enum {
    ARG_ACTION_OUTPUT_TEXT,
    ARG_ACTION_OUTPUT_BIN,
    ARG_ACTION_CONVERT_BIN_TO_TEXT,
} ArgAction;

typedef struct {
    int num_files;
    const char** files;
    const char* output_file;
    ArgAction action;
} CmdArgs;

CmdArgs parse_cmd_args(int argc, char** argv);

void free_cmd_args(const CmdArgs* args);

#endif

