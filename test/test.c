#include <stdio.h>
#include <dlfcn.h>

// pointers to library function(s)
static int (*fn_compile_expr)(char* expr);

// test expressions
static char *tests[] = {
        "42",
        "z + 3i",
        "mag(z+1) + mag(z-1) - 1",
        "sin(real(z)) + cos(imag(z))*i"
};
#define TESTS_LEN (sizeof(tests) / sizeof(tests[0]))

int main(void) {
    printf("test: opening libgrapher.so\n");
    void* lib = dlopen("libgrapher.so", RTLD_LAZY);
    if (!lib) {
        fprintf(stderr, "error: dlopen(): %s\n", dlerror());
        return -1;
    }

    // get the function(s)
    fn_compile_expr = (typeof(fn_compile_expr)) dlsym(lib, "compile_expr");
    if (!fn_compile_expr) {
        fprintf(stderr, "error: dlsym(): %s\n", dlerror());
        return -1;
    }

    // run tests
    size_t fails = 0;
    for (size_t i = 0; i < TESTS_LEN; i++) {
        printf("\n=== test %lu: \"%s\" ===\n", i+1, tests[i]);
        if (fn_compile_expr(tests[i]) == -1) {
            printf("=== test %lu failed ===\n", i+1);
            fails++;
        }
    }

    printf("\n%lu/%lu tests passed\n", TESTS_LEN - fails, TESTS_LEN);
    return 0;
}
