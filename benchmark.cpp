#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include "memSafe.h"

void alloc_lots(size_t s) {
    uint8_t* dummy;
    for(int i = 0; i < s; i++) {
        dummy = new uint8_t;
        delete dummy;
    }
}

int main() {
    const int SIZE = 1000;
    const int THREADS = 100;
    auto start = std::chrono::steady_clock::now();

    // scoping trick to auto-delete thread vec
    if(true) {
        std::vector<std::thread> thread_vec(THREADS);
        for(int i = 0; i < THREADS; i++)
            thread_vec[i] = std::thread(alloc_lots, SIZE);
        for(int i = 0; i < THREADS; i++)
            thread_vec[i].join();
    }

    checkForLeaks();

    auto end = std::chrono::steady_clock::now();
    std::cout << "threads: " << THREADS << ", size: " << SIZE << "    Took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us" << std::endl;
}