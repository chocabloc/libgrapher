#pragma once

#include <stddef.h>
#include <malloc.h>

#ifdef DEBUG
    #define tmalloc(s) trace_malloc(s, __FILE__, __LINE__, __FUNCTION__)
    #define trealloc(ptr, s) trace_realloc(ptr, s, __FILE__, __LINE__, __FUNCTION__)
    #define tfree(s) trace_free(s, __FILE__, __LINE__, __FUNCTION__)
#else
    #define tmalloc(s) malloc(s)
    #define trealloc(ptr, s) realloc(ptr, s)
    #define tfree(s) free(s)
#endif

#ifdef DEBUG
void* trace_malloc(size_t s, const char* file, int line, const char* function);
void trace_free(void* addr, const char* file, int line, const char* function);
void* trace_realloc(void* addr, size_t s, const char* file, int line, const char* function);
void tmalloc_log_show();
#else
    #define tmalloc_log_show()
#endif
