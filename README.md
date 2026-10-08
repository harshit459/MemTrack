# MemTrack

MemTrack is a lightweight runtime memory debugging and leak detection library written in C.

It tracks dynamic memory allocations, detects common memory-management errors, records allocation source information, and provides runtime memory statistics.

## Features

* Tracks `malloc`, `calloc`, `realloc`, and `free`
* Memory leak detection
* Double-free detection
* Invalid free and invalid realloc detection
* `realloc()` failure handling
* Freed-address reuse handling
* `calloc()` overflow protection
* Allocation IDs and metadata
* Source file, line, and function tracking
* Allocation type tracking
* Detailed free diagnostics
* Active and peak memory statistics
* Active and peak allocation counts
* Custom hash table with separate chaining
* Automated stress, collision, and edge-case tests

## Architecture

```text
               User Program
                    |
                    v
               MemTrack API
        +------+------+------+------+
        |      |             |      |
      malloc  calloc       realloc  free
        |      |             |      |
        +------+------+------+------+
                    |
                    v
              Tracking Layer
             /               \
            v                 v
    Allocation Hash      Freed Address
        Table                Table
            \                 /
             \               /
              v             v
                Diagnostics
                    |
          +---------+---------+
          |                   |
      Leak Report       Memory Statistics
```

MemTrack uses the standard C allocation functions internally rather than implementing its own memory allocator.

Each active allocation is stored in a custom hash table along with its metadata.

## Allocation Metadata

Every tracked allocation contains:

```text
Address
Size
Allocation ID
Source file
Source line
Source function
Allocation type
```

The public allocation APIs use macros to capture the caller's location:

```c
#define mt_malloc(size) \
    mt_malloc_debug(size, __FILE__, __LINE__, __func__)
```

This allows leak reports to identify where an allocation originated.

Example:

```text
[LEAK]
ID: 12
Type: malloc
Address: 0x55...
Size: 100 bytes
Allocated at: main.c:24
Function: main
```

## Hash Table

MemTrack uses a hash table with separate chaining for active allocations.

The allocation address is converted to `uintptr_t` and hashed into a fixed number of buckets.

```text
Bucket 0  -> AllocationNode -> AllocationNode
Bucket 1  -> NULL
Bucket 2  -> AllocationNode
...
Bucket 15 -> AllocationNode -> AllocationNode
```

Separate chaining allows multiple addresses to occupy the same bucket.

The address is shifted before hashing to reduce the effect of alignment-related low bits on typical 64-bit systems.

For a well-distributed table, allocation lookup, insertion, and removal are `O(1)` on average.

## Memory Error Detection

### Memory Leaks

At shutdown, MemTrack scans the allocation table for allocations that are still active.

The report includes:

* Allocation ID
* Type
* Address
* Size
* Source location

### Double Free

Freed addresses are maintained separately so MemTrack can distinguish a double free from an invalid free.

Example:

```text
[ERROR] Double Free
Address: 0x55...

Originally allocated at: tests/test_memtrack.c:120
Allocation function: main
Previously freed at: tests/test_memtrack.c:125
Free function: main
```

### Invalid Free

If an address exists in neither the active allocation table nor the freed-address table, MemTrack reports an invalid free.

### Invalid Realloc

Before calling the system `realloc()`, MemTrack verifies that the address belongs to an active tracked allocation.

### Address Reuse

When a new allocation receives an address that previously belonged to a freed allocation, the old freed-address record is removed.

This prevents a valid new allocation from being incorrectly reported as a double free.

## Realloc Handling

`realloc()` requires special handling because the memory address may change.

```text
Existing allocation
        |
        v
 Remove from hash table
        |
        v
     realloc()
      /     \
   fail     success
    |          |
    v          v
Restore     Update address
node        and size
```

If `realloc()` fails, the original allocation remains valid and its tracking information is restored.

If it succeeds, the existing allocation record is updated and inserted into the appropriate hash bucket.

The allocation ID and original allocation metadata are preserved.

## Memory Statistics

MemTrack tracks:

```text
Allocation Operations
  malloc calls
  calloc calls
  realloc calls
  free calls

Memory Usage
  Active allocations
  Peak allocations
  Active bytes
  Peak bytes
```

Statistics can be displayed using:

```c
mt_print_stats();
```

Example:

```text
===== MemTrack Statistics =====

Allocation Operations
  malloc calls:       120
  calloc calls:        25
  realloc calls:       43
  free calls:         150

Memory Usage
  Active allocations:  38
  Peak allocations:    72
  Active bytes:       3420
  Peak bytes:         12840
```

## Project Structure

```text
MemTrack/
├── include/
│   └── memtrack.h
├── src/
│   ├── main.c
│   └── memtrack.c
├── tests/
│   └── test_memtrack.c
├── build/
├── .gitignore
├── LICENSE
├── Makefile
└── README.md
```

`main.c` contains a demonstration program, while `test_memtrack.c` contains the automated test suite.

## Build and Run

Build the project:

```bash
make
```

Run the demonstration:

```bash
./memtrack
```

Run the test suite:

```bash
make test
```

Clean generated build files:

```bash
make clean
```

## Testing

The test suite covers:

* `malloc` and `calloc` tracking
* `realloc` growth and shrinking
* `realloc()` failure
* `realloc(ptr, 0)`
* Double frees
* Invalid frees
* Invalid reallocations
* `free(NULL)`
* Address reuse
* `malloc(0)`
* `calloc(0, size)`
* `calloc()` overflow
* Same-size reallocations
* Hash-table collisions
* Hash-bucket movement
* Allocation metadata
* Memory statistics
* Stress testing with large numbers of allocations
* Leak detection
* Shutdown and reset behavior

## Design Decisions

### Wrapper-Based Tracking

MemTrack wraps the standard C allocation functions instead of implementing a custom allocator. This keeps the project focused on runtime tracking, diagnostics, and memory analysis.

### Separate Chaining

Separate chaining was chosen for hash collisions because multiple allocation addresses can map to the same bucket.

### Source-Location Macros

`__FILE__`, `__LINE__`, and `__func__` are captured at the caller's location through macros. This makes allocation and free diagnostics more useful.

### Realloc Failure Handling

The allocation record is temporarily removed before calling `realloc()`. If the operation fails, the original record is restored because the original memory is still valid.

## Limitations and Future Work

MemTrack currently requires programs to use the MemTrack APIs:

```c
mt_malloc()
mt_calloc()
mt_realloc()
mt_free()
```

It does not automatically intercept direct calls to the standard allocation functions.

Possible future extensions include:

* `LD_PRELOAD`-based automatic interception
* Stack trace collection
* Thread-safe tracking
* Configurable logging
* Allocation filtering
* Runtime visualization

## License

This project is licensed under the [MIT License](LICENSE).
