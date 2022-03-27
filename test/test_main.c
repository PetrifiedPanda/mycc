#include <stdio.h>

extern void error_test();

extern void preproc_test();

extern void parser_test();

int main() {
    error_test();
    preproc_test();
    parser_test();
    printf("All tests successful\n");
}
