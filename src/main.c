#include "memtrack.h"
#include <stdio.h>
#include <stdint.h>

int main(void)
{
    mt_init();

    printf("=== TEST 1: realloc(NULL, size) ===\n");

    int *arr = mt_realloc(NULL, 5 * sizeof(int));

    printf("Pointer: %p\n", (void *)arr);
    printf("Active bytes: %zu\n", mt_get_active_bytes());

    printf("\n=== TEST 2: realloc larger ===\n");

    arr = mt_realloc(arr, 10 * sizeof(int));

    printf("New pointer: %p\n", (void *)arr);
    printf("Active bytes: %zu\n", mt_get_active_bytes());

    Allocation *allocation = mt_find(arr);

    if (allocation != NULL)
    {
        printf("Tracked size: %zu\n", allocation->size);
        printf("Allocation ID: %lu\n", allocation->id);
    }

    printf("\n=== TEST 3: realloc smaller ===\n");

    arr = mt_realloc(arr, 2 * sizeof(int));

    printf("Pointer: %p\n", (void *)arr);
    printf("Active bytes: %zu\n", mt_get_active_bytes());

    printf("\n=== TEST 4: realloc failure ===\n");

    void *result = mt_realloc(arr, SIZE_MAX);

    if (result == NULL)
        printf("Realloc failed as expected\n");

    printf("Active bytes after failure: %zu\n", mt_get_active_bytes());

    if (mt_find(arr) != NULL)
        printf("Original allocation still tracked\n");

    printf("\n=== TEST 5: realloc(ptr, 0) ===\n");

    result = mt_realloc(arr, 0);

    if (result == NULL)
        printf("Returned NULL as expected\n");

    printf("Active bytes: %zu\n", mt_get_active_bytes());

    if (mt_find(arr) == NULL)
        printf("Allocation removed successfully\n");

    printf("\n=== TEST 6: double free after realloc(ptr, 0) ===\n");

    mt_free(arr);

    printf("\n=== FINAL SHUTDOWN ===\n");

    mt_shutdown();

    return 0;
}