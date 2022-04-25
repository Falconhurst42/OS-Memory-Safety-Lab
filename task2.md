# Task 2: Implementation Notes

Issues/Failures
 - We can't overload pointer functionality (operator*, operator[]), which means we can't directly detect out of bounds allocation
 - When updating the allocation map in the overloaded new/delete, we add or remove objects from the map, which calls new/delete and causes infinite recursion. To fix this, implemented a flag `is_internal` which turns off overloaded new/delete functionality during the execution of the new/delete function
 - When the cleanup function detects a leak, it deletes that block of memory (which in turn deletes it from the allocation map). However, this delete interferes with standard C++ range-based iteration, requiring a slightly different iteration scheme.
 - To handle bounds-overflow detection without operator overloading, `operator new` now allocates two fixed-sized buffers before and after each allocated block of memory. The program tracks these padding buffers, and when a block of memory is deleted it checks if the values of the buffers have changed (which would indicated and overflow write). NOTE: this system still has no way of detecting an overflow read.

Multithreading
 - When I attempted to run a mutlithreaded program with this library, I realized that the `is_internal` approach would not work (because it relieson the assumption that internal tracking allocations are the only allocations that occur while `is_internal` is set). Instead, I re-wrote my tracking framework to track allocations using a dynamic array managed with only `malloc()` and `free()`.
 - The tracking library is thread safe, which involves protecting the allocation tracking functionality with a mutex (which may incur some execution cost).