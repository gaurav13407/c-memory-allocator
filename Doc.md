Custom Memory Allocator — Interview Recall Document

(malloc / free / realloc with boundary-tag coalescing)

1. What did I build? (30-second intro)

I implemented a user-space memory allocator in C that provides malloc, free, and realloc.
It uses an explicit free list, supports block splitting, forward and backward coalescing, and uses boundary tags (footers) to enable O(1) backward coalescing.
realloc supports in-place shrinking, in-place growth, and safe fallback allocation with data preservation.

That alone already sounds strong.

2. Core problem an allocator solves

The allocator manages a contiguous heap region and must:

Give memory to users (malloc)

Take it back (free)

Resize it safely (realloc)

Avoid fragmentation

Maintain alignment

Be correct under arbitrary allocation/free order

The hard part is not allocating memory,
the hard part is reusing memory without fragmentation or corruption.

3. Block layout (this is foundational)

Each heap block looks like this:

[ HEADER | PAYLOAD | FOOTER ]

Header (metadata)

Contains:

size → payload size

free → allocation status

next → next block in heap order

Footer (boundary tag)

Contains:

size → duplicate of payload size

Why footer exists

The footer allows this:

“From the current block, jump backwards to the previous block in O(1) time.”

This enables backward coalescing without a doubly linked list.

4. Alignment strategy

All payloads are aligned to ALLOCATOR_ALIGNMENT

max_align_t ensures correctness for any C type

Size rounding is done using a bit-mask macro

Why alignment matters:

Misaligned access → UB or performance penalties

Allocators must guarantee alignment

5. Allocation strategy (malloc)
High-level steps

Reject size 0

Align requested size

Search free list (first-fit)

If found:

Split block if large enough

Mark as used

If not found:

Request more memory from OS (sbrk)

Return pointer to payload

Why first-fit?

Simple

Predictable

Easy to reason about for learning

Performance tuning was intentionally not the goal.

6. Block splitting (internal fragmentation control)

When allocating from a free block:

If block is much larger than requested size:

Split into:

Allocated block

Smaller free block

Both blocks get correct headers and footers

Invariant:

Never create unusable tiny fragments.

7. Freeing memory (free)

Freeing is not just marking a flag.

Steps:

Mark block as free

Coalesce backward

Coalesce forward

Goal:

After free, no two adjacent free blocks exist.

This invariant is crucial.

8. Coalescing — the heart of the allocator
Forward coalescing

If next block is free:

Merge current + next

Update size and next pointer

Rewrite footer

Backward coalescing (boundary tags)

Use footer of previous block

Jump backwards in O(1)

If previous block is free:

Merge previous + current

Fix list links

Rewrite footer

Why this matters:

Prevents external fragmentation

Enables future large allocations

Without coalescing:

Heap grows even when free memory exists.

9. Why boundary tags (footer method)?

Two ways to support backward coalescing:

Doubly linked list

Boundary tags (footers)

Chosen: Boundary tags

Reasons:

Less metadata overhead

Cache-friendlier

Classic allocator design (dlmalloc, glibc lineage)

Interviewers like it

10. realloc — the hardest part (and most important)

realloc tests everything:

splitting

coalescing

pointer correctness

data preservation

realloc has 5 semantic cases
Case 1: realloc(NULL, size)

→ Behaves like malloc(size)

Why:

Allows uniform allocation logic

Case 2: realloc(ptr, 0)

→ Behaves like free(ptr)
→ Returns NULL

Case 3: Shrinking

If current block is already large enough:

Keep same pointer

Split if useful

No copy

This avoids unnecessary work.

Case 4: In-place growth (optimal path)

If next block is free and combined size is enough:

Merge blocks

Split if extra space remains

Return same pointer

This is real allocator behavior.

Key interview point:

realloc may return the same pointer.

Case 5: Fallback

If in-place growth fails:

Allocate new block

Copy min(old_size, new_size)

Free old block

Return new pointer

Important correctness rule:

If realloc fails, the original pointer is still valid.

11. NULL semantics (very interview-important)
malloc

Returns NULL → allocation failed

malloc(0) → may return NULL or dummy pointer

realloc

realloc(NULL, size) → malloc

realloc(ptr, 0) → free

Return NULL → failure, original block untouched

This distinction is frequently tested.

12. Tests — what did I prove?

The test suite verifies:

Correct allocation & deallocation

Alignment guarantees

Block reuse after free

Forward + backward coalescing correctness

realloc correctness:

NULL case

shrink

in-place growth

fallback growth

data preservation

Important insight:

Tests do not assume realloc must move memory — because the standard does not guarantee that.

13. What I intentionally did NOT implement (and why)

Not included by design:

Thread safety

Bins / size classes

mmap-based large allocations

Heap shrinking (brk)

Reason:

Goal was learning allocator internals, not building production libc.

Interviewers respect this decision when explained clearly.

14. Key invariants I maintained

You should remember these:

Payloads are always aligned

Every block has a correct footer

After free, no adjacent free blocks exist

realloc never loses data

realloc never frees old memory on failure

These invariants define correctness.

15. One-minute interview explanation (memorize)

“I built a custom allocator with an explicit free list and boundary-tag coalescing.
Blocks have headers and footers, which lets me coalesce both forward and backward in O(1).
malloc uses first-fit and splitting, free merges adjacent free blocks, and realloc handles shrinking, in-place growth, and fallback allocation with data preservation.
The focus was correctness and fragmentation control rather than performance tuning.”

That answer is excellent.

16. Final takeaway

This project demonstrates:

Real understanding of heap allocators

Pointer arithmetic and memory layout

Fragmentation control

realloc semantics (which many people get wrong)
