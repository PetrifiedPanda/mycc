#include "test_util.h"

#include <stdio.h>
#include <stdlib.h>

#include "test_asserts.h"

char* read_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    ASSERT_NOT_NULL(f);

    int result;

    result = fseek(f, 0, SEEK_END);
    ASSERT_INT(result, 0);

    size_t fsize = ftell(f);
    result = fseek(f, 0, SEEK_SET);
    ASSERT_INT(result, 0);

    char* res = malloc(sizeof(char) * (fsize + 1));
    ASSERT_NOT_NULL(res);

    size_t chars_read = fread(res, 1, fsize, f);
    ASSERT_SIZE_T(chars_read, fsize);
    res[fsize] = '\0';

    fclose(f);
    return res;
}
