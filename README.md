 Custom Memory Allocator in C

A from-scratch implementation of a user-space memory allocator in C, supporting malloc, free, and realloc, with proper alignment, block splitting, and bidirectional coalescing using boundary tags (footers).

This project was built to deeply understand heap management, memory layout, and fragmentation control, and is interview-grade for systems / low-level roles.

✨ Features

Custom my_malloc, my_free, and my_realloc

Explicit free list (singly linked)

Proper alignment using max_align_t

Block splitting to reduce internal fragmentation

Forward + backward coalescing

Boundary tags (footers) for O(1) backward coalescing

In-place realloc growth when possible

Safe fallback realloc with copy when required

Comprehensive test suite validating allocator invariants

📐 Memory Layout

Each heap block is laid out as:

[ Header | Payload | Footer ]

Header (block_t)
struct {
    size_t size;        // payload size
    int free;           // allocation status
    struct block *next; // next block in heap
}

Footer (Boundary Tag)
size_t size; // duplicate payload size


The footer allows constant-time access to the previous block, enabling efficient backward coalescing without a doubly linked list.

🧠 Design Decisions
Alignment

All payloads are aligned to ALLOCATOR_ALIGNMENT

max_align_t ensures correctness for all standard types

Allocation Strategy

First-fit search over the explicit free list

Simple, predictable, and easy to reason about

Fragmentation Control

Block splitting on allocation

Forward coalescing on free

Backward coalescing using boundary tags

Invariant maintained:

After free, no two adjacent free blocks exist.

🔁 realloc Strategy

my_realloc(ptr, new_size) handles all standard cases:

realloc(NULL, size) → malloc(size)

realloc(ptr, 0) → free(ptr)

Shrinking:

Reuse same block

Split if beneficial

Growing:

Attempt in-place expansion using next free block

Otherwise, allocate → copy → free

This mirrors real allocator behavior and avoids unnecessary copies.

🧪 Testing

The test suite validates:

Basic allocation and deallocation

Alignment guarantees

Block splitting and reuse

Forward and backward coalescing

realloc behavior:

NULL case

Shrink

In-place growth

Fallback growth

Data preservation

All tests pass consistently.

🛠 Build & Run

From the project root:

gcc src/allocator.c tests/test_basic.c -Iinclude -o test_allocator
./test_allocator


Expected output ends with:

realloc tests passed
ALL basic tests passed

📁 Project Structure
custom_memory_allocator/
├── include/
│   └── allocator.h
├── src/
│   └── allocator.c
├── tests/
│   └── test_basic.c
└── README.md

🎯 What This Project Demonstrates

Deep understanding of heap allocators

Memory layout and pointer arithmetic

Fragmentation and coalescing strategies

Correct and efficient realloc semantics

Systems-level reasoning in C

This is not a toy allocator — it implements the same core ideas used in real allocators (glibc, dlmalloc) at a simplified, educational scale.

🚫 Out of Scope (By Design)

Thread safety

Bins / size classes

mmap-based large allocations

Heap trimming (brk shrink)

These were intentionally excluded to keep the focus on correctness and learning, not production tuning.
