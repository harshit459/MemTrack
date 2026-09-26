#include "memtrack.h"
#include <stdio.h>

int main(void)
{
    mt_init();

    void *p = mt_malloc(100);

    Allocation *allocation = mt_find(p);

    if (allocation != NULL)
    {
        printf("Found allocation:\n");
        printf("Address: %p\n", allocation->address);
        printf("Size: %zu bytes\n", allocation->size);
        printf("ID: %lu\n", allocation->id);
    }
    else
    {
        printf("Allocation not found!\n");
    }

    return 0;
}