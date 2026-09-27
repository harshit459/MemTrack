#include "memtrack.h"
#include <stdio.h>

int main(void)
{
    mt_init();

    void *p1 = mt_malloc(100);
    void *p2 = mt_malloc(200);

    printf("Before re-init:\n");
    printf("Allocations: %lu\n", mt_get_total_allocations());
    printf("Active bytes: %zu\n", mt_get_active_bytes());

    mt_init();

    printf("\nAfter re-init:\n");
    printf("Allocations: %lu\n", mt_get_total_allocations());
    printf("Frees: %lu\n", mt_get_total_frees());
    printf("Active bytes: %zu\n", mt_get_active_bytes());
    printf("Peak bytes: %zu\n", mt_get_peak_bytes());

    printf("\nLookup after re-init:\n");
    printf("p1: %s\n", mt_find(p1) == NULL ? "NOT FOUND" : "FOUND");
    printf("p2: %s\n", mt_find(p2) == NULL ? "NOT FOUND" : "FOUND");

    return 0;
}