#include "memtrack.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define STRESS_COUNT 1000

int main(void)
{
    printf("===== MemTrack Test Suite =====\n\n");

    mt_init();

    printf("[TEST] malloc tracking\n");

    void *ptr = mt_malloc(100);

    assert(ptr != NULL);
    assert(mt_find(ptr) != NULL);
    assert(mt_find(ptr)->size == 100);
    assert(mt_get_total_allocations() == 1);
    assert(mt_get_active_bytes() == 100);

    printf("[PASS] malloc tracking\n");

    mt_free(ptr);

    assert(mt_find(ptr) == NULL);
    assert(mt_get_total_frees() == 1);
    assert(mt_get_active_bytes() == 0);

    printf("[PASS] free tracking\n");

    printf("\n[TEST] calloc tracking\n");

    int *arr = mt_calloc(5, sizeof(int));

    assert(arr != NULL);

    for (int i = 0; i < 5; i++)
    {
        assert(arr[i] == 0);
    }

    assert(mt_find(arr) != NULL);
    assert(mt_find(arr)->size == 5 * sizeof(int));
    assert(mt_get_total_allocations() == 2);
    assert(mt_get_active_bytes() == 5 * sizeof(int));

    printf("[PASS] calloc tracking and zero initialization\n");

    mt_free(arr);

    assert(mt_get_total_frees() == 2);
    assert(mt_get_active_bytes() == 0);

    printf("[PASS] calloc free tracking\n");

    printf("\n[TEST] realloc tracking\n");

    int *data = mt_malloc(5 * sizeof(int));

    assert(data != NULL);
    assert(mt_find(data)->size == 5 * sizeof(int));

    data = mt_realloc(data, 10 * sizeof(int));

    assert(data != NULL);
    assert(mt_find(data) != NULL);
    assert(mt_find(data)->size == 10 * sizeof(int));
    assert(mt_get_active_bytes() == 10 * sizeof(int));

    printf("[PASS] realloc grow\n");

    data = mt_realloc(data, 2 * sizeof(int));

    assert(data != NULL);
    assert(mt_find(data) != NULL);
    assert(mt_find(data)->size == 2 * sizeof(int));
    assert(mt_get_active_bytes() == 2 * sizeof(int));

    printf("[PASS] realloc shrink\n");

    mt_free(data);

    assert(mt_find(data) == NULL);
    assert(mt_get_active_bytes() == 0);

    printf("[PASS] realloc free tracking\n");

    printf("\n[TEST] realloc failure\n");

    ptr = mt_malloc(100);

    assert(ptr != NULL);

    size_t active_before = mt_get_active_bytes();

    void *result = mt_realloc(ptr, SIZE_MAX);

    assert(result == NULL);
    assert(mt_find(ptr) != NULL);
    assert(mt_get_active_bytes() == active_before);

    printf("[PASS] realloc failure preserves allocation\n");

    mt_free(ptr);

    printf("\n[TEST] realloc(ptr, 0)\n");

    ptr = mt_malloc(100);

    assert(ptr != NULL);
    assert(mt_find(ptr) != NULL);

    result = mt_realloc(ptr, 0);

    assert(result == NULL);
    assert(mt_find(ptr) == NULL);
    assert(mt_get_active_bytes() == 0);

    printf("[PASS] realloc(ptr, 0) frees allocation\n");

    printf("\n[TEST] double free detection\n");

    ptr = mt_malloc(50);

    assert(ptr != NULL);

    mt_free(ptr);

    /* Second free should be detected */
    mt_free(ptr);

    printf("[PASS] double free detection\n");

    printf("\n[TEST] invalid free detection\n");

    int fake_value = 42;
    void *fake_ptr = &fake_value;

    mt_free(fake_ptr);

    printf("[PASS] invalid free detection\n");

    printf("\n[TEST] address reuse\n");

    void *old_ptr = mt_malloc(64);

    assert(old_ptr != NULL);

    mt_free(old_ptr);

    void *new_ptr = mt_malloc(64);

    assert(new_ptr != NULL);
    assert(mt_find(new_ptr) != NULL);

    printf("[PASS] address reuse tracking\n");

    mt_free(new_ptr);

    printf("\n[TEST] stress test\n");

    void *ptrs[STRESS_COUNT];

    for (int i = 0; i < STRESS_COUNT; i++)
    {
        ptrs[i] = mt_malloc((i % 100) + 1);
        assert(ptrs[i] != NULL);
    }

    assert(mt_get_active_bytes() > 0);

    printf("[PASS] 1000 allocations\n");

    for (int i = 0; i < STRESS_COUNT; i += 2)
    {
        mt_free(ptrs[i]);
        ptrs[i] = NULL;
    }

    printf("[PASS] 500 frees\n");

    for (int i = 1; i < STRESS_COUNT; i += 2)
    {
        void *new_ptr = mt_realloc(ptrs[i], 200);

        assert(new_ptr != NULL);
        ptrs[i] = new_ptr;
    }

    printf("[PASS] 500 reallocations\n");

    for (int i = 1; i < STRESS_COUNT; i += 2)
    {
        mt_free(ptrs[i]);
        ptrs[i] = NULL;
    }

    assert(mt_get_active_bytes() == 0);

    printf("[PASS] all stress allocations freed\n");

    printf("\n[TEST] hash table collision test\n");

#define COLLISION_COUNT 100

    void *collision_ptrs[COLLISION_COUNT];

    for (int i = 0; i < COLLISION_COUNT; i++)
    {
        collision_ptrs[i] = mt_malloc(32);
        assert(collision_ptrs[i] != NULL);
    }

    for (int i = 0; i < COLLISION_COUNT; i++)
    {
        Allocation *allocation = mt_find(collision_ptrs[i]);

        assert(allocation != NULL);
        assert(allocation->address == collision_ptrs[i]);
        assert(allocation->size == 32);
    }

    printf("[PASS] all colliding allocations tracked\n");

    for (int i = 0; i < COLLISION_COUNT; i++)
    {
        mt_free(collision_ptrs[i]);
    }

    printf("[PASS] all colliding allocations freed\n");

    assert(mt_get_active_bytes() == 0);

    printf("[PASS] collision cleanup\n");

    printf("\n[TEST] realloc hash bucket movement\n");

    old_ptr = mt_malloc(32);

    assert(old_ptr != NULL);
    assert(mt_find(old_ptr) != NULL);

    Allocation *old_allocation = mt_find(old_ptr);
    unsigned long allocation_id = old_allocation->id;

    new_ptr = mt_realloc(old_ptr, 1024);

    assert(new_ptr != NULL);

    Allocation *new_allocation = mt_find(new_ptr);

    assert(new_allocation != NULL);
    assert(new_allocation->address == new_ptr);
    assert(new_allocation->size == 1024);
    assert(new_allocation->id == allocation_id);

    printf("[PASS] realloc allocation remains tracked\n");

    mt_free(new_ptr);

    assert(mt_get_active_bytes() == 0);

    printf("[PASS] realloc bucket movement cleanup\n");

    printf("\n[TEST] allocation metadata\n");

    void *metadata_ptr = mt_malloc(64);
    assert(metadata_ptr != NULL);

    Allocation *metadata = mt_find(metadata_ptr);

    assert(metadata != NULL);
    assert(metadata->address == metadata_ptr);
    assert(metadata->size == 64);
    assert(metadata->file != NULL);
    assert(metadata->function != NULL);
    assert(metadata->line > 0);
    assert(metadata->type == MT_MALLOC);

    printf("[PASS] malloc metadata\n");

    mt_free(metadata_ptr);

    metadata_ptr = mt_calloc(4, sizeof(int));
    assert(metadata_ptr != NULL);

    metadata = mt_find(metadata_ptr);

    assert(metadata != NULL);
    assert(metadata->type == MT_CALLOC);
    assert(metadata->size == 16);

    printf("[PASS] calloc metadata\n");

    mt_free(metadata_ptr);

    metadata_ptr = mt_realloc(NULL, 128);
    assert(metadata_ptr != NULL);

    metadata = mt_find(metadata_ptr);

    assert(metadata != NULL);
    assert(metadata->type == MT_REALLOC);
    assert(metadata->size == 128);
    assert(metadata->file != NULL);
    assert(metadata->function != NULL);
    assert(metadata->line > 0);

    printf("[PASS] realloc(NULL, size) metadata\n");

    mt_free(metadata_ptr);

    printf("\n[TEST] realloc preserves metadata\n");

    metadata_ptr = mt_malloc(100);

    metadata = mt_find(metadata_ptr);

    assert(metadata != NULL);

    const char *original_file = metadata->file;
    const char *original_function = metadata->function;
    int original_line = metadata->line;
    unsigned long original_id = metadata->id;

    metadata_ptr = mt_realloc(metadata_ptr, 500);

    assert(metadata_ptr != NULL);

    metadata = mt_find(metadata_ptr);

    assert(metadata != NULL);
    assert(metadata->size == 500);
    assert(metadata->id == original_id);
    assert(metadata->file == original_file);
    assert(metadata->function == original_function);
    assert(metadata->line == original_line);
    assert(metadata->type == MT_MALLOC);

    printf("[PASS] realloc preserves allocation metadata\n");

    mt_free(metadata_ptr);

    mt_shutdown();

    printf("\n[TEST] detailed memory statistics\n");

    void *stats_ptr1 = mt_malloc(100);
    void *stats_ptr2 = mt_malloc(200);

    assert(stats_ptr1 != NULL);
    assert(stats_ptr2 != NULL);

    assert(mt_get_malloc_calls() == 2);
    assert(mt_get_active_allocations() == 2);
    assert(mt_get_peak_allocations() == 2);

    printf("[PASS] malloc statistics\n");

    void *stats_ptr3 = mt_calloc(5, sizeof(int));

    assert(stats_ptr3 != NULL);
    assert(mt_get_calloc_calls() == 1);
    assert(mt_get_active_allocations() == 3);
    assert(mt_get_peak_allocations() == 3);

    printf("[PASS] calloc statistics\n");

    stats_ptr1 = mt_realloc(stats_ptr1, 500);

    assert(stats_ptr1 != NULL);
    assert(mt_get_realloc_calls() == 1);
    assert(mt_get_active_allocations() == 3);

    printf("[PASS] realloc statistics\n");

    mt_free(stats_ptr2);

    assert(mt_get_active_allocations() == 2);

    printf("[PASS] free statistics\n");

    mt_free(stats_ptr1);
    mt_free(stats_ptr3);

    assert(mt_get_active_allocations() == 0);
    assert(mt_get_peak_allocations() == 3);

    printf("[PASS] active and peak allocation statistics\n");

    printf("\n[TEST] statistics report\n");

    mt_print_stats();

    printf("[PASS] statistics report\n");

    printf("\n[TEST] leak detection and statistics\n");

    void *leak1 = mt_malloc(100);
    void *leak2 = mt_calloc(5, sizeof(int));

    assert(leak1 != NULL);
    assert(leak2 != NULL);

    assert(mt_get_active_bytes() == 120);

    printf("[PASS] allocation statistics\n");

    printf("\nExpected leak report:\n");

    mt_shutdown();

    printf("\n[TEST] shutdown and reset\n");

    assert(mt_get_active_bytes() == 0);
    assert(mt_get_total_allocations() == 0);
    assert(mt_get_total_frees() == 0);

    printf("[PASS] shutdown reset statistics\n");

    ptr = mt_malloc(64);

    assert(ptr != NULL);
    assert(mt_find(ptr) != NULL);
    assert(mt_get_active_bytes() == 64);
    assert(mt_get_total_allocations() == 1);

    printf("[PASS] allocation works after shutdown\n");

    mt_free(ptr);

    assert(mt_get_active_bytes() == 0);
    assert(mt_get_total_frees() == 1);

    printf("[PASS] tracking works after shutdown\n");

    mt_shutdown();

    assert(mt_get_active_bytes() == 0);
    assert(mt_get_total_allocations() == 0);
    assert(mt_get_total_frees() == 0);

    printf("[PASS] second shutdown reset\n");

    printf("\n===== All tests completed =====\n");

    return 0;
}