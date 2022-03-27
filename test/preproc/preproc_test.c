#include <stdio.h>

extern void tokenizer_test();
extern void preproc_macro_test();

void preproc_test() {
    printf("Starting preprocessor test\n");
    tokenizer_test();
    preproc_macro_test();
    printf("Preprocessor test successful\n");
}

