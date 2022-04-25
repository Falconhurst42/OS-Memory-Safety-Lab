#ifndef MEM_SAFE_H
#define MEM_SAFE_H

#include <map>
#include <list>
#include <algorithm>
#include <iostream>
#include <mutex>
#include <cstring>

using pad = uint32_t;
const uint8_t pad_bytes = sizeof(pad);
const uint8_t loc_bytes = sizeof(void*);
const uint8_t size_bytes = sizeof(size_t);
const uint8_t record_bytes = loc_bytes + size_bytes;

//uint64_t base_alloc = 0;
//uint64_t tracking_alloc = 0;
//uint64_t padding_alloc = 0;

//bool record_debug = false;
//bool alloc_debug = false;

std::mutex mtx;

// track allocations without new/delete
void* allocated;
size_t size = 0, count = 0;

void print_alloc() {
    std::cout << "BEGIN ALLOC: " << std::endl;
    for(int i = 0; i < count; i++) {
        std::cout << "    Record #" << i << ": p = " << *((void**)(allocated + (i * record_bytes))) << ", n = " << *((size_t*)(allocated + (i * record_bytes) + loc_bytes)) << std::endl;
    }
    std::cout << "END ALLOC: " << std::endl << std::endl;
}

// expand allocated without new/delete
void expand() {
//if(record_debug) std::cout << "Expanding from " << size << " to " << size*2 << std::endl;
    void* temp = malloc(size * record_bytes * 2);
    //tracking_alloc += size * record_bytes * 2;
    memcpy(temp, allocated, size * record_bytes);
    size *= 2;
    free(allocated);
    allocated = temp;
}

// record new allocation
void record_alloc(void* p, size_t n) {
    if(size == 0) {
        size = 8;
        allocated = malloc(size * record_bytes);
        //tracking_alloc += size * record_bytes;
    }
    if (count >= size) {
        expand();
    }
    // copy loc and size
    memcpy(allocated + (count * record_bytes), &p, loc_bytes);
    memcpy(allocated + (count * record_bytes) + loc_bytes, &n, size_bytes);
//if(record_debug) std::cout << "    Creating record #" << count << " @ " << allocated + (count * record_bytes);
//if(record_debug) std::cout << ".  Contents: " << *((void**)(allocated + (count * record_bytes))) << ",  " << *((size_t*)(allocated + (count * record_bytes) + loc_bytes)) << std::endl;
    count++;
}

// check if something is allocated
void* get_allocated(void* p) {
    for(int i = 0; i < count; i++) {
 //std::cout << "    Comparing record #" << i << " @ " << allocated + (i * record_bytes);
 //std::cout << ".  Contents: " << *((void**)(allocated + (i * record_bytes))) << ",  " << *((size_t*)(allocated + (i * record_bytes) + loc_bytes)) << std::endl;
        if(memcmp(allocated + (i * record_bytes), &p, loc_bytes) == 0) {
 //std::cout << "Returning " << allocated + (i * record_bytes) << std::endl;
            return allocated + (i * record_bytes);
        }
    }
    return (void*)0;
}

// record new allocation
void delete_alloc(void* record) {
    size_t dist = ((ptrdiff_t)record - (ptrdiff_t)allocated) / record_bytes;
//if(record_debug) std::cout << "    Deleting record #" << dist << " @ " << record;
//if(record_debug) std::cout << ".  Contents: " << *((void**)(allocated + (dist * record_bytes))) << ",  " << *((size_t*)(allocated + (dist * record_bytes) + loc_bytes)) << std::endl;
    count--;
    memmove(record, record + record_bytes, (count - dist) * record_bytes);
    // #### #### XXXX #### #### ####
    // dist = 2
    // count = 5 (after dec)
    // moving 3 records
}

// allocate memory and log in map
void* operator new(size_t n) {
    void* p = malloc(n + 2*pad_bytes);
//if(alloc_debug) std::cout << "  Allocated " << n << " bytes at " << p + pad_bytes << std::endl;
    *((pad*)p) = 0;
    *((pad*)(p + n + pad_bytes)) = 0;
    mtx.lock();
    //padding_alloc += 2*pad_bytes;
    //base_alloc += n;
    record_alloc(p + pad_bytes, n);
    mtx.unlock();
    return p + pad_bytes;
}

// if memory is actually allocated, de-log and delete
// else, output error
void operator delete(void* p) {
    mtx.lock();
    void* record = get_allocated(p);
    if(record != (void*)0) {
//if(alloc_debug) std::cout << "  Deleting at " << p << std::endl;
        // check that padding has not changed
        if(*((pad*)(p - pad_bytes)) != 0) {
            std::cout << "Detected underflow write at " << p - pad_bytes << std::endl;
        } 
        if(*((pad*)(p + *((size_t*)(record + loc_bytes)))) != 0) {
            std::cout << "Detected overflow write at " << p + *((size_t*)(record + loc_bytes)) + pad_bytes << std::endl;
        } 
            delete_alloc(record);
            free(p - pad_bytes);      // free block starting with pre-padding
    } else {
        std::cout << "Invalid delete at " << p << std::endl;
        //print_alloc();
    }
    mtx.unlock();
}

// if memory is actually allocated, de-log and delete []
// else, output error
void operator delete[](void* p) {
    ::operator delete(p);
}

// notify user of all outstanding allocations and delete then
void checkForLeaks() {
    while (count != 0) {
        void* p = *((void**)(allocated));
        size_t n = *((size_t*)(allocated + loc_bytes));
//if(record_debug) std::cout << "    Checking record #" << 0 << " @ " << allocated;
//if(record_debug) std::cout << ".  Contents: " << *((void**)(allocated)) << ",  " << *((size_t*)(allocated + loc_bytes)) << std::endl;
        std::cout << "Leaked " << n << " bytes of memory at " << p << std::endl;
        delete p;
    }

    free(allocated);

    //std::cout << "Base alloc: " << base_alloc << std::endl;
    //std::cout << "Tracking alloc: " << tracking_alloc << std::endl;
    //std::cout << "PAdding alloc: " << padding_alloc << std::endl;
}

#endif