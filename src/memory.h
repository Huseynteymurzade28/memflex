#ifndef MEMORY_H
#define MEMORY_H

#ifdef UNIT_TESTING
#include <stddef.h>
#include <stdint.h>
#else
#include <linux/types.h>
#endif

/* Allocation algorithms */
typedef enum
{
    ALGO_FIRST_FIT,
    ALGO_BEST_FIT,
    ALGO_WORST_FIT
} alloc_algo_t;

/* Initialize the heap manager */
void heap_init(void *start_addr, size_t size);

/* Set the allocation algorithm */
void set_allocation_algorithm(alloc_algo_t algo);

/* Dynamic memory allocation functions */
void *my_kmalloc(size_t size);
void my_kfree(void *ptr);
void *my_kcalloc(size_t num, size_t size);

/* Debugging/Info */
void print_heap_stats(void);

#endif
