# Dynamic Memory Manager (User Space)

This project implements a custom Dynamic Memory Manager in user space using `sbrk`. It provides functionality similar to standard C library functions (`malloc`, `free`, `calloc`) and includes a benchmarking suite to compare different allocation algorithms.

The project demonstrates fundamental operating system concepts such as memory allocation strategies, fragmentation management, and performance benchmarking.

## Features

- **Custom Memory Allocation:** Implements `my_malloc`, `my_free`, and `my_calloc`.
- **Allocation Algorithms:**
  - **First-Fit:** Allocates the first free block that fits the requested size.
  - **Best-Fit:** Allocates the smallest free block that fits the requested size (minimizes wasted space).
  - **Worst-Fit:** Allocates the largest free block (leaves large holes).
- **Fragmentation Management:** Implements **Coalescing** to merge adjacent free blocks and reduce external fragmentation.
- **Doubly Linked List:** Uses a doubly linked list structure to manage memory blocks efficiently.
- **JSON Output:** Benchmark results are saved to `results.json` for further analysis.
- **Visualization:** A Rust-based terminal visualization tool is included to view benchmark results.

## Project Structure

```
memflex/
├── Makefile            # Build script
├── src/
│   ├── main.c          # Benchmark runner and tests
│   ├── memory.c        # Implementation of the memory manager
│   └── memory.h        # Header file with function prototypes and structs
├── viz/                # Rust visualization tool
├── results.json        # Output file from benchmarks
└── README.md           # This file
```

## Prerequisites

- __C Project__:
  - GCC Compiler
  - Make tool
- __Visualization__:
  - Rust (Cargo)

## How to Build and Run

### 1. Build and Run Allocator Benchmarks

**Open a terminal** in the project directory.

**Compile and Run:**

```bash
make run-main
```

This command compiles the project, runs the benchmarks, prints statistics to the console, and generates `results.json`.

### 2. Run Visualization

After generating `results.json`, you can visualize the results using the Rust tool.

**Navigate to the viz directory:**

```bash
cd viz
```

**Run the visualization:**

```bash
cargo run
```

This will launch a terminal-based UI showing:
- **Execution Time**: Comparison of execution speed for different algorithms.
- **Total Block Count**: Comparison of fragmentation (managed as total memory blocks).

_Press 'q' to exit the visualization._

## Understanding the Output

The `results.json` file contains:
- `name`: Algorithm name (e.g., FIRST_FIT).
- `time`: Execution time in seconds.
- `total_blocks`: Total number of memory blocks (used + free) remaining after operations, serving as a metric for fragmentation.

## Configuration

You can modify test parameters in `src/main.c` (e.g., `BENCH_INITIAL_ALLOCS`, `BENCH_FREES`).

## License

This project is for educational purposes.
