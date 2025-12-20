#ifdef UNIT_TESTING
#include <stdio.h>
#include <stdint.h>
#else
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#endif
#include "memory.h"

/* Memory block header structure */
typedef struct block_header
{
    size_t size;               /* Size of the data part */
    int is_free;               /* 1 if free, 0 if allocated */
    struct block_header *next; /* Pointer to the next block in the list */
    struct block_header *prev; /* Pointer to the previous block */
} block_header_t;

/* Global variables */
static void *heap_start = NULL;
static block_header_t *head = NULL;
static alloc_algo_t current_algo = ALGO_FIRST_FIT;

/* Helper: Align size to 8 bytes */
static size_t align(size_t n)
{
    return (n + 7) & ~7;
}

void heap_init(void *start_addr, size_t size)
{
    heap_start = start_addr;

    /* Initialize the first block covering the entire heap */
    head = (block_header_t *)start_addr;
    head->size = size - sizeof(block_header_t);
    head->is_free = 1;
    head->next = NULL;
    head->prev = NULL;

    printk(KERN_INFO "MyMemory: Heap initialized at: %p Size: %zu\n", start_addr, size);
}

void set_allocation_algorithm(alloc_algo_t algo)
{
    current_algo = algo;
    printk(KERN_INFO "MyMemory: Allocation algorithm set to: ");
    if (algo == ALGO_FIRST_FIT)
        printk(KERN_CONT "First Fit\n");
    else if (algo == ALGO_BEST_FIT)
        printk(KERN_CONT "Best Fit\n");
    else if (algo == ALGO_WORST_FIT)
        printk(KERN_CONT "Worst Fit\n");
}

/* Split a block if it's large enough to hold the requested size + a new header + some data */
static void split_block(block_header_t *block, size_t size)
{
    /* Minimum size for a new block: header + 8 bytes */
    size_t min_block_size = sizeof(block_header_t) + 8;

    if (block->size >= size + min_block_size)
    {
        /* Calculate address of the new block */
        block_header_t *new_block = (block_header_t *)((uint8_t *)block + sizeof(block_header_t) + size);

        new_block->size = block->size - size - sizeof(block_header_t);
        new_block->is_free = 1;
        new_block->next = block->next;
        new_block->prev = block;

        if (block->next)
        {
            block->next->prev = new_block;
        }

        block->next = new_block;
        block->size = size;
    }
}

/* Coalesce free blocks */
static void coalesce(block_header_t *block)
{
    /* Try to merge with next */
    if (block->next && block->next->is_free)
    {
        block->size += sizeof(block_header_t) + block->next->size;
        block->next = block->next->next;
        if (block->next)
        {
            block->next->prev = block;
        }
    }

    /* Try to merge with prev */
    if (block->prev && block->prev->is_free)
    {
        block->prev->size += sizeof(block_header_t) + block->size;
        block->prev->next = block->next;
        if (block->next)
        {
            block->next->prev = block->prev;
        }
    }
}

void *my_kmalloc(size_t size)
{
    /* DEBUG: Prove we are using custom allocator */
    printk(KERN_INFO "MyMemory: my_kmalloc called for %zu bytes\n", size);

    if (size == 0)
        return NULL;

    size = align(size);
    block_header_t *curr_block = head;
    block_header_t *best_block = NULL;

    while (curr_block)
    {
        if (curr_block->is_free && curr_block->size >= size)
        {
            if (current_algo == ALGO_FIRST_FIT)
            {
                best_block = curr_block;
                break;
            }
            else if (current_algo == ALGO_BEST_FIT)
            {
                if (!best_block || curr_block->size < best_block->size)
                {
                    best_block = curr_block;
                }
            }
            else if (current_algo == ALGO_WORST_FIT)
            {
                if (!best_block || curr_block->size > best_block->size)
                {
                    best_block = curr_block;
                }
            }
        }
        curr_block = curr_block->next;
    }

    if (best_block)
    {
        split_block(best_block, size);
        best_block->is_free = 0;
        return (void *)((uint8_t *)best_block + sizeof(block_header_t));
    }

    return NULL; /* Out of memory */
}

void my_kfree(void *ptr)
{
    if (!ptr)
        return;

    /* Get header */
    block_header_t *block = (block_header_t *)((uint8_t *)ptr - sizeof(block_header_t));
    block->is_free = 1;

    /* Merge with neighbors to reduce fragmentation */
    coalesce(block);
}

void *my_kcalloc(size_t num, size_t size)
{
    size_t total = num * size;
    void *ptr = my_kmalloc(total);
    if (ptr)
    {
        /* Zero out memory */
        uint8_t *p = (uint8_t *)ptr;
        for (size_t i = 0; i < total; i++)
        {
            p[i] = 0;
        }
    }
    return ptr;
}

void print_heap_stats(void)
{
    block_header_t *curr_block = head;
    printk(KERN_INFO "MyMemory: --- Heap Stats ---\n");
    while (curr_block)
    {
        printk(KERN_INFO "MyMemory: Block at: %p Size: %zu %s\n",
               curr_block, curr_block->size, curr_block->is_free ? "[FREE]" : "[USED]");
        curr_block = curr_block->next;
    }
    printk(KERN_INFO "MyMemory: ------------------\n");
}
