#include <malloc.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "tmalloc.h"

typedef struct adata_t {
    uint64_t addr;
    size_t size;
    int line;
    const char* file;
    const char* function;

    struct adata_t* next;
} adata_t;

static adata_t* ainfo;

static void mem_err(char* s, const char* file, int line, const char* function) {
    printf("\033[0;31;1mmemory error:\033[0m %s\n", s);
    printf("in %s:%d (function %s)\n\n", file, line, function);
    exit(-1);
}

void* trace_malloc(size_t s, const char* file, int line, const char* function)
{
    void* addr =  malloc(s);

    // append new record for address
    adata_t* ninfo = malloc(sizeof(adata_t));
    *ninfo = (adata_t) {
        .addr = (uint64_t)addr,
        .size = s,
        .file = file,
        .line = line,
        .function = function,
        .next = ainfo
    };
    ainfo = ninfo;
    return addr;
}

void* trace_realloc(void* addr, size_t s, const char* file, int line, const char* function) {
    if (addr == NULL)
        return trace_malloc(s, file, line, function);

    // find previous record of address
    adata_t* oinfo = NULL;
    for (adata_t* i = ainfo; i != NULL; i = i->next)
        if (i->addr == (uint64_t)addr)
            oinfo = i;
    if (oinfo == NULL)
        mem_err("tried to reallocate an invalid address", file, line, function);

    // update previous record
    oinfo->addr = (uint64_t)realloc(addr, s);
    oinfo->size = s;
    oinfo->file = file;
    oinfo->line = line;
    oinfo->function = function;
    return (void*)(oinfo->addr);
}

void trace_free(void* addr, const char* file, int line, const char* function) {
    // find address record and remove it
    adata_t* temp;
    if (ainfo->addr == (uint64_t)addr) {
        temp = ainfo;
        ainfo = ainfo->next;
        free(temp);
    } else {
        adata_t* inf = NULL;
        for (adata_t* i = ainfo; i->next != NULL; i = i->next)
            if (i->next->addr == (uint64_t)addr)
                inf = i;
        if (inf == NULL)
            mem_err("tried to free unowned memory", file, line, function);

        temp = inf->next;
        inf->next = inf->next->next;
        free(temp);
    }

    return free(addr);
}

void tmalloc_log_show() {
#ifdef TRACE_ALLOCATIONS
    size_t memsize = 0;

    char* prev = NULL;
    int repeat = 0;
    for (adata_t* i = ainfo; i != NULL; i = i->next) {
        char* outstr = NULL;
        asprintf(&outstr, "%d bytes in %s:%d (function %s)\n", i->size, i->file, i->line, i->function);
        if (prev && strcmp(prev, outstr) == 0)
            repeat++;
        else {
            if (repeat != 0)
                printf("\t... %d identical line%c ...\n", repeat, repeat == 1 ? ' ' : 's');
            repeat = 0;
            printf("%s", outstr);
        }
        if (prev) free(prev);
        prev = outstr;
        memsize += i->size;
    }
    if (prev) free(prev);
    printf("total %ld bytes of allocated memory\n", memsize);
#endif
}
