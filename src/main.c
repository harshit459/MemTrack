#include "memtrack.h"
#include <stdio.h>

int main(void)
{
    mt_init();

    void *p1 = mt_malloc(100);
    void *p2 = mt_malloc(200);

    int value = 10;

    printf("Before freeing:\n");
    printf("Active bytes: %zu\n", mt_get_active_bytes());

    mt_free(p1);
    mt_free(p1);

    printf("\nAfter freeing p1:\n");
    printf("Active bytes: %zu\n", mt_get_active_bytes());

    mt_free(p2);

    printf("\nAfter freeing p2:\n");
    printf("Active bytes: %zu\n", mt_get_active_bytes());

    mt_free(&value);

    mt_shutdown();

    return 0;
}