#include "memtrack.h"
#include <stdlib.h>
#include <stdint.h>

#define TABLE_SIZE 16

static AllocationNode *table[TABLE_SIZE] = {NULL};

static unsigned long next_allocation_id = 1;

void mt_init(void){
    next_allocation_id = 1;
}

static size_t hash_address(void *address){
    
    uintptr_t value = (uintptr_t)address;

    return value % TABLE_SIZE;
}

static void insert_allocation(Allocation allocation){

    size_t index = hash_address(allocation.address);

    AllocationNode *node = malloc(sizeof(AllocationNode));

    if(node == NULL)
        return;

    node->allocation = allocation;
    node->next =  table[index];

    table[index] = node;

}

void *mt_malloc(size_t size){

    void *ptr = malloc(size);

    if(ptr == NULL)
        return NULL;

    Allocation allocation;
    allocation.address = ptr;
    allocation.size = size;
    allocation.id = next_allocation_id++;

    insert_allocation(allocation);

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