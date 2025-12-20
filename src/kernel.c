#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include "memory.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OSUser1");
MODULE_DESCRIPTION("Dynamic Memory Manager Demo Module");

/* Pointer to the memory pool we will allocate from Linux kernel */
static void *my_heap_pool = NULL;
/* Size of our custom heap */
static size_t heap_size = 1024 * 1024; /* 1 MB */

static int __init my_module_init(void)
{
    printk(KERN_INFO "MyMemory: Module loaded.\n");

    /* Allocate a large chunk of memory from the kernel to manage ourselves */
    my_heap_pool = vmalloc(heap_size);
    if (!my_heap_pool)
    {
        printk(KERN_ERR "MyMemory: Failed to vmalloc %zu bytes\n", heap_size);
        return -ENOMEM;
    }

    /* Initialize our custom heap manager with this memory */
    heap_init(my_heap_pool, heap_size);

    /* --- DEMO: First Fit --- */
    set_allocation_algorithm(ALGO_FIRST_FIT);

    printk(KERN_INFO "MyMemory: Allocating 3 pointers (100, 500, 200 bytes)...\n");
    void *p1 = my_kmalloc(100);
    void *p2 = my_kmalloc(500);
    void *p3 = my_kmalloc(200);

    print_heap_stats();

    printk(KERN_INFO "MyMemory: Freeing p2 (500 bytes)...\n");
    my_kfree(p2);
    print_heap_stats();

    printk(KERN_INFO "MyMemory: Allocating p4 (300 bytes) - Should fit in p2's hole with First Fit\n");
    void *p4 = my_kmalloc(300);
    print_heap_stats();

    /* Clean up for next test */
    my_kfree(p1);
    my_kfree(p3);
    my_kfree(p4);

    /* --- DEMO: Best Fit --- */
    printk(KERN_INFO "MyMemory: \n--- Switching to Best Fit ---\n");
    set_allocation_algorithm(ALGO_BEST_FIT);

    void *a = my_kmalloc(100);
    void *b = my_kmalloc(2000); /* Large hole */
    void *c = my_kmalloc(100);
    void *d = my_kmalloc(500); /* Small hole */
    void *e = my_kmalloc(100);

    my_kfree(b); /* Free 2000 */
    my_kfree(d); /* Free 500 */

    printk(KERN_INFO "MyMemory: Holes available: ~2000 and ~500. Allocating 400.\n");
    /* First Fit would take the 2000 block (it comes first). Best Fit should take the 500 block. */

    void *f = my_kmalloc(400);
    print_heap_stats();

    /* Clean up demo allocations */
    my_kfree(a);
    my_kfree(c);
    my_kfree(e);
    my_kfree(f);

    /* --- DEMO: Worst Fit --- */
    printk(KERN_INFO "MyMemory: \n--- Switching to Worst Fit ---\n");
    set_allocation_algorithm(ALGO_WORST_FIT);

    /* Create fragmentation:
       Alloc: [100][500][100][200][100]
       Free:       [500]     [200]
    */
    void *w1 = my_kmalloc(100);
    void *w2 = my_kmalloc(500);
    void *w3 = my_kmalloc(100);
    void *w4 = my_kmalloc(200);
    void *w5 = my_kmalloc(100);

    my_kfree(w2); /* Free 500 */
    my_kfree(w4); /* Free 200 */

    printk(KERN_INFO "MyMemory: Holes available: ~500 and ~200. Allocating 150.\n");
    /* Best Fit would take 200. Worst Fit should take 500. */

    void *w6 = my_kmalloc(150);
    print_heap_stats();

    /* Clean up */
    my_kfree(w1);
    my_kfree(w3);
    my_kfree(w5);
    my_kfree(w6);

    printk(KERN_INFO "MyMemory: Demo Complete.\n");

    return 0;
}

static void __exit my_module_exit(void)
{
    if (my_heap_pool)
    {
        vfree(my_heap_pool);
        printk(KERN_INFO "MyMemory: Heap pool freed.\n");
    }
    printk(KERN_INFO "MyMemory: Module unloaded.\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
