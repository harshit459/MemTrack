#ifndef MEMTRACK_H
#define MEMTRACK_H

#include <stddef.h>

typedef struct {
    void *address;
    size_t size;
    unsigned long id;
} Allocation;

typedef struct AllocationNode{
    Allocation allocation;
    struct AllocationNode *next;
} AllocationNode;

unsigned long mt_get_total_allocations(void);
unsigned long mt_get_total_frees(void);

size_t mt_get_active_bytes(void);
size_t mt_get_peak_bytes(void);

void mt_init(void);

void *mt_malloc(size_t size);
void *mt_calloc(size_t nmemb, size_t size);
void *mt_realloc(void *address, size_t size);

Allocation *mt_find(void* address);
void mt_free(void *address);
void mt_report_leaks(void);
void mt_shutdown(void);

#endif  