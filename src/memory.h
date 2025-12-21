#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

/* Memory block header structure */
typedef struct block_header
{
    size_t size;               /* Size of the data part */
    int is_free;               /* 1 if free, 0 if allocated */
    struct block_header *next; /* Pointer to the next block in the list */
    struct block_header *prev; /* Pointer to the previous block */
} block_header_t;

/* Allocation algorithms */
typedef enum
{
    ALGO_FIRST_FIT,
    ALGO_BEST_FIT,
    ALGO_WORST_FIT
} alloc_algo_t;

/* Initialize the heap manager */
void heap_init(void *start_addr, size_t size);

/* High-Level Init/Cleanup (Manages backing store) */
int my_memory_init(size_t size);
void my_memory_cleanup(void);
void my_memory_reset(void);

/* Dynamic memory allocation functions */
void *my_malloc(size_t size, alloc_algo_t algo);
void my_free(void *ptr);
void *my_calloc(size_t num, size_t size, alloc_algo_t algo);

/* Debugging/Info */
void print_heap_stats(void *highlight_ptr);
void print_block_count(void);
int get_total_block_count(void);
void print_total_size(void);
void dump_heap_state(const char *filepath, int step, const char *op, void *highlight_ptr, const char *algo_name);

#endif
