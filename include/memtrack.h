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

void mt_init(void);
void *mt_malloc(size_t size);
Allocation *mt_find(void* address);

#endif  