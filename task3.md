# Task 3: Evaluation
## Final Architecture
The library includes overloads of the new and delete operators to track when memory is allocated and deallocated. It is designed to detect invalid delete calls, memory leaks, and overflow writes.
 - The `operator new` overload intercepts allocations and record where the memory is allocated in a global map. This builds up a map of allocated memory which saves the location and size of each allocation and will be referenced to ensure that all dereference, access, and delete calls are operating on valid pointers. The overload also places 4-byte buffers on either side of an allocation which are initialized to zero. These are designed to detect overflow writes.
 - The `operator delete` overload intercepts deallocation. If the deallocated location is in the memory map, the program checks that the buffers have not been altered. If the location is not in the map, the program notifies the user that an invalid delete has been called at the given location. If the buffers have been latered, the program notifies the user that an overflow write has occurred.
 - The `checkForLeaks()` function should be called at the end of the program. If any allocations remain in the memory map, then the program notifies the user of a memory leak and deletes those allocations (this final delete is important, even though the program is ending, because it detects overflow writes).
 - The internal map of allocated memory is stored as `void* allocated`, an array of consecutive `void*, size_t` pairs which is managed entirely with `malloc()` and `free()` to avoid recursively triggering the memory manager

## Usage
To use this library, the user just needs to include the `memSafe.h` header file and call the `checkForLeaks()` function at the end of their program. If the program detects any errors, it will output a message to standard out (but will not terminate execution).

## Benchmarking
The library was benchmarked by comparing the performance of a program wihtout the library compared to its performance when the `memSafe.h` header was included and `checkForLeaks()` was called. The two metric measured were execution time (measured by using the `chrono` library) and memory usage (measured by counting bytes allocated by malloc in the `memSafe` header, categorized into "required" bytes and "extra" bytes).

First, the library was benchmarked with a "worst case scenario" program (`benchmark.h`). This program created 100 threads which each allocated and delete a 8-bit integer 1000 times. This represents a worst case scenario in terms of execution time because the entire execution of the program is calling memory allocation and deallocation. This represents a worst-case scenario in terms of memory usage beacuse the program makes thousands of single-byte allocations, each of which incurs the fixed tracking cost for the minimum base allocation. In my opinion, this benchmark represents the absolute worst-case scenario for execution and allocation cost. Any real-world application will certainly incur less relative cost than 40x execution time and 10x memory usage.
| Metric | Avg wo/lib | Avg w/lib | Cost |
| - | - | - | - |
| Execution time | 7.9 s | 26.6 s | 34x Slower |
| Memory Usage | 0.1 MB | 0.9 MB | 9x Larger | 
As the table indicates, running this benchmark using the library incurred significant execution cost, because the process of tracking memory usage is significantly more costly than allocating it. Likewise, it presented a significant cost to memory allocation, because each 1-byte allocation incurred 8 bytes of padding. The tracking overhead was relatively light, as each allocation was immediately deleted, keeping the allocation map small. However, in benchmarks where I instead allocated a vector pointers to of 8-bit integers, allocated aech integer, and then deallocated each integer, the relative memory overhead actually decreased to a 2.15x increase. This is because, for every allocation that the library is tracking, the host program must also hold a pointer to the allocation (and those pointer in this case are much larger than the memory block they point to). For a large number of small simultaneous allocations, the size of this tracking far outstrips the size of the acutal memory it is tracking, and the relative overhead approaches a factor of 2 ( sizeof(void*) / (sizeof(void*) + sizeof(size_t)) ).

Second, the library was benchmarked with a "best case scenario" program (`future.h`). This program searches the numbers 1 to 10,000,000 for digitally delicate primes. It splits this search accross a number of threads and repeats it with different umbers of threads to compare efficiency. This represents a best case scenario in terms of execution time because the program spends a loooong time performing calculations on the CPU compared to relatively little allocation.
| Metric | Avg wo/lib | Avg w/lib | Cost |
| - | - | - | - |
| Execution time |  90 s |  94 s | Negligible |
| Memory Usage | 306000 bytes | 479176 bytes | 1.5x Larger |
In this case, the cost in execution time is negligible at less than 5%. The memory cost is also reduced, because the program allocates memory in much larger chunks, which reduces the relative tracking overhead.

## Potential Additional Features
 - Better error messages which inform the user of line number / call stack information
 - A better means of checking for invalid deletes (current method is linear search, which incurs execution when a lot of stuff is being tracked)
 - rework use of `void*` in tracking infrastructure to eliminate compiler warnings
 - Do this all with a custom compiler to gain better control over allocation/deallocation rather than relying on overloads of new/delete