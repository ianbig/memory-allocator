#include <stdio.h>
#include <assert.h>
#include "dmm.h"

void test_size_bool() {
    printf("sizeof bool is %ld\n", sizeof(bool));
}

void test_dmalloc() {
    printf("===== Start dmalloc test====\n");
    assert(dmalloc(16) != NULL);
    printf("dmalloc(16) passed\n");
    assert(dmalloc(20) != NULL);
    printf("dmalloc(20) passed\n");
    
    int * arr = dmalloc(101 * sizeof(int));
    for (int i = 0; i < 101; i++) {
        arr[i] = i;
    }

    for (int i = 0; i < 101; i++) {
        assert(arr[i] == i);
    }

    printf("test dmalloc write passed\n");
}

int main(int argc, char ** argv) {
    test_dmalloc();
}