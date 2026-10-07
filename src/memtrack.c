#include "memtrack.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define TABLE_SIZE 16

typedef struct FreedNode
{
    void *address;
    struct FreedNode *next;
} FreedNode;

static AllocationNode *table[TABLE_SIZE] = {NULL};
static FreedNode *freed_table[TABLE_SIZE] = {NULL};

static unsigned long next_allocation_id = 1;
static unsigned long total_allocations = 0;
static unsigned long total_frees = 0;

static size_t active_bytes = 0;
static size_t peak_bytes = 0;

static int insert_allocation(Allocation allocation);
static void remove_freed(void *address);

unsigned long mt_get_total_allocations(void)
{
    return total_allocations;
}

unsigned long mt_get_total_frees(void)
{
    return total_frees;
}

size_t mt_get_active_bytes(void)
{
    return active_bytes;
}

size_t mt_get_peak_bytes(void)
{
    return peak_bytes;
}

static size_t hash_address(void *address){
    
    uintptr_t value = (uintptr_t)address;

    value >>= 4;

    return value % TABLE_SIZE;
}

static void *track_allocation(void *ptr, size_t size, 
    AllocationType type, const char *file, 
    int line, const char *function)
{
    if (ptr == NULL)
        return NULL;

    remove_freed(ptr);

    Allocation allocation;

    allocation.address = ptr;
    allocation.size = size;
    allocation.id = next_allocation_id++;

    allocation.file = file;
    allocation.line = line;
    allocation.function = function;
    allocation.type = type;

    if (!insert_allocation(allocation))
    {
        free(ptr);
        return NULL;
    }

    total_allocations++;
    active_bytes += size;

    if (active_bytes > peak_bytes)
        peak_bytes = active_bytes;

    return ptr;
}

static int insert_allocation(Allocation allocation){

    size_t index = hash_address(allocation.address);

    AllocationNode *node = malloc(sizeof(AllocationNode));

    if(node == NULL)
        return 0;

    node->allocation = allocation;
    node->next =  table[index];

    table[index] = node;

    return 1;

}

static AllocationNode *remove_allocation(void *address){

    size_t index = hash_address(address);

    AllocationNode *prev = NULL;
    AllocationNode *curr = table[index];

    while(curr != NULL){

        if(curr->allocation.address == address){

            if(prev == NULL){
                table[index] = curr->next;
            }
            else{
                prev->next = curr->next;
            }

            return curr;
        }

        prev = curr;
        curr = curr->next;

    }

    return NULL;

}

static int is_freed(void *address){

    size_t index = hash_address(address);

    FreedNode *curr = freed_table[index];

    while(curr != NULL){

        if(curr->address == address){
            return 1;
        }

        curr = curr->next;
    }

    return 0;

}

static int record_free(void *address){

    size_t index = hash_address(address);

    FreedNode *node = malloc(sizeof(FreedNode));

    if(node == NULL){
        return 0;
    }

    node->address = address;
    node->next = freed_table[index];

    freed_table[index] = node;

    return 1;

}

static void remove_freed(void *address)
{
    size_t index = hash_address(address);

    FreedNode *prev = NULL;
    FreedNode *curr = freed_table[index];

    while (curr != NULL)
    {
        if (curr->address == address)
        {
            if (prev == NULL)
            {
                freed_table[index] = curr->next;
            }
            else
            {
                prev->next = curr->next;
            }

            free(curr);
            return;
        }

        prev = curr;
        curr = curr->next;
    }
}

static void cleanup_freed(void)
{
    for (size_t i = 0; i < TABLE_SIZE; i++)
    {
        FreedNode *curr = freed_table[i];

        while (curr != NULL)
        {
            FreedNode *next = curr->next;

            free(curr);

            curr = next;
        }

        freed_table[i] = NULL;
    }
}

static void cleanup_allocation(void){

    for(size_t i = 0; i < TABLE_SIZE; i++){

        AllocationNode *curr = table[i];
        
        while(curr != NULL){

            AllocationNode *next = curr->next;

            free(curr->allocation.address);
            free(curr);

            curr = next;
        }

        table[i] = NULL;

    }

}

void mt_init(void){

    next_allocation_id = 1;
    
    total_allocations = 0;
    total_frees = 0;

    active_bytes = 0;
    peak_bytes = 0;
}

void mt_shutdown(void)
{

    mt_report_leaks();

    cleanup_allocation();
    cleanup_freed();

    next_allocation_id = 1;

    total_allocations = 0;
    total_frees = 0;

    active_bytes = 0;
    peak_bytes = 0;
}

void *mt_malloc_debug(size_t size, const char *file, 
    int line, const char *function)
{
    void *ptr = malloc(size);

    return track_allocation(ptr, size, MT_MALLOC,
        file, line, function
    );
}

void *mt_calloc_debug(size_t nmemb, size_t size,
    const char *file, int line, const char *function)
{
    if (nmemb != 0 && size > SIZE_MAX / nmemb)
        return NULL;

    void *ptr = calloc(nmemb, size);

    return track_allocation(ptr, nmemb * size, MT_CALLOC,
        file, line, function
    );
}

void *mt_realloc_debug(void *address, size_t size,
    const char *file, int line, const char *function)
{

    if(address == NULL){
        void *ptr = malloc(size);

        return track_allocation(ptr, size, MT_REALLOC, 
            file, line, function);
    }

    if (size == 0)
    {
        mt_free(address);
        return NULL;
    }

    AllocationNode *node = remove_allocation(address);

    if(node == NULL){
        printf("Invalid realloc: %p\n", address);
        return NULL;
    }

    size_t old_size = node->allocation.size;

    void *new_ptr = realloc(address, size);

    if(new_ptr == NULL){

        size_t index = hash_address(address);

        node->next = table[index];
        table[index] = node;

        return NULL;
    }

    remove_freed(new_ptr);

    node->allocation.address = new_ptr;
    node->allocation.size = size;

    size_t index = hash_address(new_ptr);

    node->next = table[index];
    table[index] = node;

    active_bytes -= old_size;
    active_bytes += size;

    if(active_bytes > peak_bytes)
        peak_bytes = active_bytes;

    return new_ptr;

}

Allocation *mt_find(void *address){

    size_t index = hash_address(address);

    AllocationNode *curr = table[index];

    while(curr != NULL){

        if(curr->allocation.address == address)
            return &curr->allocation;

        curr = curr->next;
    }

    return NULL;

}

void mt_free(void *address){

    if (address == NULL)
        return;

    AllocationNode *node = remove_allocation(address);

    if(node == NULL){

        if(is_freed(address)){
            printf("Double free: %p\n", address);
        }
        else{
            printf("Invalid free: %p\n", address);
        }

        return;
    }

    if (!record_free(node->allocation.address))
    {
        printf("Warning: failed to record freed address: %p\n", node->allocation.address);
    }
    
    free(node->allocation.address);

    active_bytes -= node->allocation.size;

    free(node);

    total_frees++;

}

void mt_report_leaks(void)
{
    size_t leak_count = 0;
    size_t leaked_bytes = 0;

    for (size_t idx = 0; idx < TABLE_SIZE; idx++)
    {
        AllocationNode *curr = table[idx];

        while (curr != NULL)
        {
            printf("\n[LEAK]\n");
            printf("ID: %lu\n", curr->allocation.id);
            printf("Type: ");

            switch (curr->allocation.type)
            {
                case MT_MALLOC:
                    printf("malloc\n");
                    break;

                case MT_CALLOC:
                    printf("calloc\n");
                    break;

                case MT_REALLOC:
                    printf("realloc\n");
                    break;

                default:
                    printf("unknown\n");
            }

            printf("Address: %p\n", curr->allocation.address);
            printf("Size: %zu bytes\n", curr->allocation.size);

            printf("Allocated at: %s:%d\n",
                   curr->allocation.file,
                   curr->allocation.line);

            printf("Function: %s\n",
                   curr->allocation.function);

            leak_count++;
            leaked_bytes += curr->allocation.size;

            curr = curr->next;
        }
    }

    if (leak_count == 0)
    {
        printf("No memory leaks detected\n");
    }
    else
    {
        printf("\nTotal leaked allocations: %zu\n", leak_count);
        printf("Total leaked bytes: %zu\n", leaked_bytes);
    }
}