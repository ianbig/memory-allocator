#include <stdio.h>
#include <assert.h>
#include "dmm.h"

void test_size_bool() {
    printf("sizeof bool is %ld\n", sizeof(bool));
}

void test_dmalloc() {
    printf("===== Start dmalloc test====\n");
    void * ptr = NULL;
    assert((ptr = dmalloc(16)) != NULL);
    // ptr -= METADATA_T_ALIGNED;
    // printf("size: %ld\n", ((metadata_t*)ptr)->size);
    // assert(((metadata_t*)ptr)->size - 32 == 16);
    printf("dmalloc(16) passed\n");
    assert(dmalloc(20) != NULL);
    printf("dmalloc(20) passed\n");
    
    int * arr = dmalloc(101 * sizeof(int));
    for (int i = 0; i < 101; i++) {
        arr[i] = i;
    }

    for (int i = 0; i < 101; i++) {
        assert(arr[i] == i);
        arr[i] = 0;
    }

    for (int i = 0; i < 101; i++) {
        assert(arr[i] == 0);
    }

    printf("test dmalloc write passed\n");
}

void test_dmm_init_add_header_and_footer() {
    dmalloc(0);
}

int main(int argc, char ** argv) {
    // test_dmm_init_add_header_and_footer();
    test_dmalloc();
}