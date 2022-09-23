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
    dfree(ptr);
    printf("dmalloc(16) passed\n");
    assert(dmalloc(20) != NULL);
    dfree(ptr);
    printf("dmalloc(20) passed\n");
    
    char * arr = dmalloc(26 * sizeof(*arr));
    for (int i = 0; i < 26; i++) {
        arr[i] = 'a' + i;
    }

    for (int i = 0; i < 26; i++) {
        assert(arr[i] == ('a' + i));
        arr[i] = 'a';
    }

    for (int i = 0; i < 26; i++) {
        assert(arr[i] == 'a');
    }
    dfree(arr);
    printf("test dmalloc write passed\n");
}

void test_dfree_no_coalease() {
    printf("======= test_dfree_no_coalease() =======\n");
    char * prev = dmalloc(sizeof(*prev) * 800);
    char * mid = dmalloc(sizeof(*mid) * 800);
    char * next = dmalloc(sizeof(*next) * 800);

    dfree(mid);
    char * newArr = dmalloc(sizeof(*newArr) * 800);
    assert(mid == newArr);
    printf("Test: test_dfree_no_coalease() passed!\n");
}

void test_dfree_coalease_prev() {
    printf("======= test_dfree_coalease_prev() =======\n");
    char * prev = dmalloc(sizeof(*prev) * 800);
    char * mid = dmalloc(sizeof(*mid) * 800);
    char * next = dmalloc(sizeof(*next) * 800);

    dfree(mid);
    dfree(prev);

    char * newArr = dmalloc(sizeof(*newArr) * 800);
    assert(newArr == prev);
    printf("Test test_dfree_coalease_prev() passed!\n");    
}
void test_dfree_coalease_next() {
    printf("======= test_dfree_coalease_next() =======\n");
    char * prev = dmalloc(sizeof(*prev) * 800);
    char * mid = dmalloc(sizeof(*mid) * 800);
    char * next = dmalloc(sizeof(*next) * 800);

    dfree(mid);
    dfree(next);

    char * newArr = dmalloc(sizeof(*newArr) * 800);
    assert(newArr == mid);
    printf("Test test_dfree_coalease_next() passed!\n");   
}
void test_dfree_coalease_prev_next() {
    printf("======= test_dfree_prev_next() =======\n");
    char * prev = dmalloc(sizeof(*prev) * 800);
    char * mid = dmalloc(sizeof(*mid) * 800);
    char * next = dmalloc(sizeof(*next) * 800);

    dfree(prev);
    dfree(next);
    dfree(mid);

    char * newArr = dmalloc(sizeof(*newArr) * 800);
    assert(newArr == prev);
    printf("Test test_dfree_coalease_prev_next() passed!\n"); 
}

void test_dfree_coalease_next_prev() {
    printf("======= test_dfree_coalease_next_prev() =======\n");
    char * prev = dmalloc(sizeof(*prev) * 800);
    char * mid = dmalloc(sizeof(*mid) * 800);
    char * next = dmalloc(sizeof(*next) * 800);

    dfree(next);
    dfree(prev);

    char * newArr = dmalloc(sizeof(*newArr) * 800);
    assert(newArr == prev);
    printf("Test test_dfree_coalease_prev() passed!\n"); 
}

void test_recyle_all_block() {
    printf("======= test_recyle_all_block() =======\n");
    char * prev = dmalloc(sizeof(*prev) * 1800);
    char * mid = dmalloc(sizeof(*mid) * 600);
    char * next = dmalloc(sizeof(*next) * 900); 

    dfree(prev);
    dfree(mid);
    dfree(next);

    // assert(ALIGN(MAX_HEAP_SIZE) - 64 == freelist->size); // remove prologue header and footer, and epilogue footer
    printf("test_recyle_all_block() passed!\n");
}

void test_dmm_init_add_header_and_footer() {
    dmalloc(0);
}

int main(int argc, char ** argv) {
    // test_dmm_init_add_header_and_footer();
    test_dmalloc();
    test_dfree_no_coalease();
    test_dfree_coalease_prev();
    test_dfree_coalease_next();
    test_dfree_coalease_prev_next();
    test_dfree_coalease_next_prev();
    test_recyle_all_block();
}