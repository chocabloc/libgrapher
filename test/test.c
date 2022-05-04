#include <stdio.h>
#include <dlfcn.h>

// pointers to library function(s)
static int (*fn_load_expr)(char* expr);

// test expressions
static char *tests[] = {
        "42",
        "z + 3i",
        "mag(z+1) + mag(z-1) - 1",
        "sin(real(z)) + cos(imag(z))*i",
        "pow(mag(z*(1 - 0.3*sin(10*arg(z)))), 2) - arg(z)*arg(z) + 0.5*mag(z)*i",
        "2 + (3*4 - 7 + (6*5 + (3)))",
        "z + sum(1, 2, 3, 4, 5, 7 + 4356 * cos(39))"
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
    fn_load_expr = (typeof(fn_load_expr)) dlsym(lib, "load_expr");
    if (!fn_load_expr) {
        fprintf(stderr, "error: dlsym(): %s\n", dlerror());
        return -1;
    }

    // run tests
    size_t fails = 0;
    for (size_t i = 0; i < TESTS_LEN; i++) {
        printf("\n=== test %lu: \"%s\" ===\n", i+1, tests[i]);
        if (fn_load_expr(tests[i]) == -1) {
            printf("=== test %lu failed ===\n", i+1);
            fails++;
        }
    }

    printf("\n%lu/%lu tests passed\n", TESTS_LEN - fails, TESTS_LEN);
    return 0;
}
