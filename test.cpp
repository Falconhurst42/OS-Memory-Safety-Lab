#include "memSafe.h"
#include <iostream>

using namespace std;

void bufferoverrun() {
    int *p = new int[10];
    p[-1] = 10; //Buffer overrun to the negative side
    p[10] = 10; //Buffer overrun to the positive side
}

void memoryleak() {
    int *p = new int[100];

    //No call to delete, so this pointer is a memory leak
}

void invaliddelete_1() {
    int *p;
    delete p;
}

void invaliddelete_2() {
    int *p = new int;
    delete p;

    //invalid delete
    delete p;
}

int main() {
    const int SIZE = 10;
    uint32_t** arr_arr = new uint32_t*[SIZE];
    for(int i = 0; i < SIZE; i++) {
        arr_arr[i] = new uint32_t[SIZE];
        for(int j = 0; j < SIZE; j++) {
            arr_arr[i][j] = 0xFFFF - j;
        }
    }
    for(int i = 0; i < SIZE; i++) {
        delete [] arr_arr[i];
    }
    delete [] arr_arr;
    /*invaliddelete_1();
    cout << endl;
    invaliddelete_2();
    cout << endl;
    bufferoverrun();
    cout << endl;
    memoryleak();
    cout << endl;*/
    checkForLeaks();
}