#include <stdio.h>
#include <dlfcn.h>

// pointers to library function(s)
static int (*lg_load)(char*);
static void (*lg_init)(void);

// test expressions
static char *tests[] = {
        "42",
        "sin(x) + cos(y) - 1",
        "max(x^2, 2*x, abs(x*y))"
        
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
    lg_load = (typeof(lg_load))dlsym(lib, "lg_load");
    lg_init = (typeof(lg_init))dlsym(lib, "lg_init");
    if (!lg_load || !lg_init) {
        fprintf(stderr, "error: dlsym(): %s\n", dlerror());
        return -1;
    }

    // initialize library
    lg_init();

    // run tests
    size_t fails = 0;
    for (size_t i = 0; i < TESTS_LEN; i++) {
        printf("\n=== test %lu: \"%s\" ===\n", i+1, tests[i]);
        if (lg_load(tests[i]) == -1) {
            printf("=== test %lu failed ===\n", i+1);
            fails++;
        }
    }

    printf("\n%lu/%lu tests passed\n", TESTS_LEN - fails, TESTS_LEN);
    return 0;
}
