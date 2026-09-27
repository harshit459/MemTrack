#include "memtrack.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define TABLE_SIZE 16

static AllocationNode *table[TABLE_SIZE] = {NULL};

static unsigned long next_allocation_id = 1;
static unsigned long total_allocations = 0;
static unsigned long total_frees = 0;

static size_t active_bytes = 0;
static size_t peak_bytes = 0;

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

    return value % TABLE_SIZE;
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

    cleanup_allocation();

    next_allocation_id = 1;
    
    total_allocations = 0;
    total_frees = 0;

    active_bytes = 0;
    peak_bytes = 0;
}

void *mt_malloc(size_t size){

    void *ptr = malloc(size);

    if(ptr == NULL)
        return NULL;

    Allocation allocation;
    allocation.address = ptr;
    allocation.size = size;
    allocation.id = next_allocation_id++;

    if(!insert_allocation(allocation)){

        free(ptr);

        return NULL;
    }

    total_allocations++;
    active_bytes += size;

    if(active_bytes > peak_bytes)
        peak_bytes = active_bytes;

    return ptr;
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

    AllocationNode *node = remove_allocation(address);

    if(node == NULL){
        printf("Invalid free: %p\n", address);
        return;
    }
    
    free(node->allocation.address);

    active_bytes -= node->allocation.size;

    free(node);

    total_frees++;

}