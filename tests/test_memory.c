#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#define UNIT_TESTING

// Mock kernel macros
#define KERN_INFO ""
#define KERN_CONT ""
#define KERN_ERR ""
#define printk printf

// Include the source code directly to test it in user space
#include "../src/memory.c"

// Wrapper to match user request
void *myFirstFitMalloc(size_t size)
{
    set_allocation_algorithm(ALGO_FIRST_FIT);
    return my_kmalloc(size);
}

void test_initialization()
{
    printf("Running test_initialization...\n");
    size_t heap_size = 4096;
    void *heap = malloc(heap_size);
    heap_init(heap, heap_size);

    // Check if head is initialized correctly (accessing static var from memory.c)
    assert(head != NULL);
    assert(head->size == heap_size - sizeof(block_header_t));
    assert(head->is_free == 1);

    free(heap);
    printf("test_initialization PASSED\n");
}

void test_first_fit_logic()
{
    printf("Running test_first_fit_logic...\n");

    size_t heap_size = 1024 * 1024;
    void *heap = malloc(heap_size);
    heap_init(heap, heap_size);

    // Allocate p1, p2, p3
    void *p1 = myFirstFitMalloc(100);
    void *p2 = myFirstFitMalloc(500);
    void *p3 = myFirstFitMalloc(200);

    assert(p1 != NULL);
    assert(p2 != NULL);
    assert(p3 != NULL);

    // Free p2 to create a hole
    my_kfree(p2);

    // Allocate p4 (300 bytes). Should fit in p2's hole.
    // Since it's First Fit, and p2 is the first free block large enough (assuming p1 is still used),
    // it should take p2's place.
    void *p4 = myFirstFitMalloc(300);

    assert(p4 == p2);

    // Allocate p5 (300 bytes). Should NOT fit in the remaining part of p2 (500 - 300 - header = ~180 < 300).
    // So it should go after p3.
    void *p5 = myFirstFitMalloc(300);
    assert(p5 > p3);

    free(heap);
    printf("test_first_fit_logic PASSED\n");
}

void test_coalescing()
{
    printf("Running test_coalescing...\n");

    size_t heap_size = 4096;
    void *heap = malloc(heap_size);
    heap_init(heap, heap_size);

    void *p1 = myFirstFitMalloc(100);
    void *p2 = myFirstFitMalloc(100);
    void *p3 = myFirstFitMalloc(100);

    // Free p1 and p3. p2 is still used.
    my_kfree(p1);
    my_kfree(p3);

    // Free p2. Now p1, p2, p3 are all free and adjacent. They should coalesce into one big block.
    my_kfree(p2);

    // The head should now be one big free block (or close to it, depending on implementation details)
    // Actually, head points to p1's block.
    assert(head->is_free == 1);
    // Size should be roughly original size (minus some overhead if any was lost, but coalescing should recover it)
    // The initial size was heap_size - sizeof(block_header_t).
    // After splitting, we had headers. Coalescing should merge headers back.
    assert(head->size == heap_size - sizeof(block_header_t));
    assert(head->next == NULL);

    free(heap);
    printf("test_coalescing PASSED\n");
}

void test_best_fit_logic()
{
    printf("Running test_best_fit_logic...\n");

    size_t heap_size = 1024 * 1024;
    void *heap = malloc(heap_size);
    heap_init(heap, heap_size);
    set_allocation_algorithm(ALGO_BEST_FIT);

    // Allocate p1(100), p2(500), p3(100), p4(200), p5(100)
    void *p1 = my_kmalloc(100);
    void *p2 = my_kmalloc(500);
    void *p3 = my_kmalloc(100);
    void *p4 = my_kmalloc(200);
    void *p5 = my_kmalloc(100);

    // Free p2 (500) and p4 (200)
    my_kfree(p2);
    my_kfree(p4);

    // Allocate p6 (150).
    // Best Fit should pick p4 (200) because 200 is closer to 150 than 500 is.
    void *p6 = my_kmalloc(150);

    assert(p6 == p4);

    free(heap);
    printf("test_best_fit_logic PASSED\n");
}

void test_worst_fit_logic()
{
    printf("Running test_worst_fit_logic...\n");

    // Use a smaller heap to control the "Rest" block size
    size_t heap_size = 4096;
    void *heap = malloc(heap_size);
    heap_init(heap, heap_size);
    set_allocation_algorithm(ALGO_WORST_FIT);

    // Allocate p1(100), p2(500), p3(100), p4(200), p5(100)
    void *p1 = my_kmalloc(100);
    void *p2 = my_kmalloc(500);
    void *p3 = my_kmalloc(100);
    void *p4 = my_kmalloc(200);
    void *p5 = my_kmalloc(100);

    // Allocate a filler to reduce the size of the final free block (Rest)
    // Total allocated so far: 100+500+100+200+100 = 1000.
    // Headers approx 6 * 32 = 192.
    // We want Rest < 500.
    // Current Rest ~ 4096 - 1192 = 2904.
    // Let's allocate 2500.
    void *p_fill = my_kmalloc(2500);
    assert(p_fill != NULL);

    // Free p2 (500) and p4 (200)
    my_kfree(p2);
    my_kfree(p4);

    // Now free blocks are:
    // p2: 500
    // p4: 200
    // Rest: ~400 (4096 - 1000 - 2500 - headers)
    // Largest is p2 (500).

    // Allocate p6 (150).
    // Worst Fit should pick p2 (500) because 500 > 200 and 500 > Rest.
    void *p6 = my_kmalloc(150);

    assert(p6 == p2);

    free(heap);
    printf("test_worst_fit_logic PASSED\n");
}

int main()
{
    test_initialization();
    test_first_fit_logic();
    test_best_fit_logic();
    test_worst_fit_logic();
}
