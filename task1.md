# Task 1: Initial Architecture

The library will include overloads of the new, delete, *, and [] operators to track when memory is allocated, deallocated, and accessed. 
 - The `operator new` overload will intercept allocations and record where the memory is allocated. This will build up a map of allocated memory, which will be referenced to ensure that all dereference, access, and delete calls are operating on valid pointers. 
 - The `operator*` and `operator[]` overloads will intercept a dereference call and check that the location in question is actually allocated.
 - The `operator delete` overload will intercept deallocation, use the allocation map to verify that it is valid, and update the allocation map accordingly

Allocated memory will be saved in a `map<void*, size_t>` which saves the location and size of each allocation

Memory leaks will be detected by a `Cleanup()` function to be called at the end of the program. This function will check the allocation map to see if any memory has failed to be deleted (i.e. memory leak).