#ifndef MEMTRACK_H
#define MEMTRACK_H

#include <stddef.h>

#define mt_malloc(size) \
    mt_malloc_debug(size, __FILE__, __LINE__, __func__)

#define mt_calloc(nmemb, size) \
    mt_calloc_debug(nmemb, size, __FILE__, __LINE__, __func__)

#define mt_realloc(address, size) \
    mt_realloc_debug(address, size, __FILE__, __LINE__, __func__)

#define mt_free(address) \
    mt_free_debug(address, __FILE__, __LINE__, __func__)

typedef enum{

    MT_MALLOC,
    MT_CALLOC,
    MT_REALLOC

} AllocationType;

typedef struct {

    void *address;
    size_t size;
    unsigned long id;

    const char *file;
    const char *function;
    int line;

    AllocationType type;

} Allocation;

typedef struct AllocationNode{

    Allocation allocation;
    struct AllocationNode *next;
    
} AllocationNode;

unsigned long mt_get_total_allocations(void);
unsigned long mt_get_total_frees(void);

unsigned long mt_get_malloc_calls(void);
unsigned long mt_get_calloc_calls(void);
unsigned long mt_get_realloc_calls(void);

unsigned long mt_get_active_allocations(void);
unsigned long mt_get_peak_allocations(void);

size_t mt_get_active_bytes(void);
size_t mt_get_peak_bytes(void);

void mt_init(void);

void *mt_malloc_debug(
    size_t size,
    const char *file,
    int line,
    const char *function
);

void *mt_calloc_debug(
    size_t nmemb,
    size_t size,
    const char *file,
    int line,
    const char *function
);

void *mt_realloc_debug(
    void *address,
    size_t size,
    const char *file,
    int line,
    const char *function
);

Allocation *mt_find(void* address);

void mt_free_debug(
    void *address,
    const char *file,
    int line,
    const char *function
);
void mt_report_leaks(void);
void mt_shutdown(void);
void mt_print_stats(void);

#endif  