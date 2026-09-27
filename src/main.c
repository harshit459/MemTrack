#include "memtrack.h"
#include <stdio.h>

int main(void)
{
    mt_init();

    void *p1 = mt_malloc(100);
    void *p2 = mt_malloc(200);
    void *p3 = mt_malloc(300);

    printf("p1: %p\n", p1);
    printf("p2: %p\n", p2);
    printf("p3: %p\n", p3);

    printf("Total allocations: %lu\n", mt_get_total_allocations());
    printf("Active bytes: %zu\n", mt_get_active_bytes());
    printf("Peak bytes: %zu\n", mt_get_peak_bytes());

    printf("\nFreeing p2...\n");
    mt_free(p2);

    printf("Active bytes: %zu\n", mt_get_active_bytes());

    printf("Total frees: %lu\n", mt_get_total_frees());

    printf("\np1 lookup: %s\n",
           mt_find(p1) != NULL ? "FOUND" : "NOT FOUND");

    printf("p2 lookup: %s\n",
           mt_find(p2) != NULL ? "FOUND" : "NOT FOUND");

    printf("p3 lookup: %s\n",
           mt_find(p3) != NULL ? "FOUND" : "NOT FOUND");

    mt_free(p1);
    mt_free(p3);

    printf("Active bytes: %zu\n", mt_get_active_bytes());
    printf("Peak bytes: %zu\n", mt_get_peak_bytes());

    return 0;
}