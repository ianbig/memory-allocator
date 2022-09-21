#include <stdio.h>  // needed for size_t etc.
#include <unistd.h> // needed for sbrk etc.
#include <sys/mman.h> // needed for mmap
#include <assert.h> // needed for asserts
#include <stdlib.h>
#include "dmm.h"

/* 
 * The lab handout and code guide you to a solution with a single free list containing all free
 * blocks (and only the blocks that are free) sorted by starting address.  Every block (allocated
 * or free) has a header (type metadata_t) with list pointers, but no footers are defined.
 * That solution is "simple" but inefficient.  You can improve it using the concepts from the
 * reading.
 */

/* 
 *size_t is the return type of the sizeof operator.   size_t type is large enough to represent
 * the size of the largest possible object (equivalently, the maximum virtual address).
 */

typedef struct metadata {
  bool isAllocate;
  size_t size;
  struct metadata* next;
  struct metadata* prev;
} metadata_t;


/*
 * Head of the freelist: pointer to the header of the first free block.
 */

static metadata_t* freelist = NULL;


 /* for debugging; can be turned off through -NDEBUG flag*/
/*

This code is here for reference.  It may be useful.
Warning: the NDEBUG flag also turns off assert protection.

*/
void print_freelist(); 

#ifdef NDEBUG
	#define DEBUG(M, ...)
	#define PRINT_FREELIST print_freelist
#else
	#define DEBUG(M, ...) fprintf(stderr, "[DEBUG] %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
	#define PRINT_FREELIST
#endif


void print_freelist() {
  metadata_t *freelist_head = freelist;
  while(freelist_head != NULL) {
    DEBUG("\tFreelist Size:%zd, Head:%p, Prev:%p, Next:%p\t",
	  freelist_head->size,
	  freelist_head,
	  freelist_head->prev,
	  freelist_head->next);
    freelist_head = freelist_head->next;
  }
  DEBUG("\n");
}

void insertToFreeList(metadata_t* blockToPush) {
  if (freelist) {
    blockToPush->next = freelist;
    freelist->prev = blockToPush;
  }
  freelist = blockToPush;
}

void initBlock(metadata_t * blockToInit) {
  blockToInit->isAllocate = false;
  blockToInit->size = 10;
  blockToInit->prev = NULL;
  blockToInit->next = NULL;
}

void removeList(metadata_t * toRemove) {
  if (freelist == toRemove) {
    freelist = freelist->next;
  }

  if (toRemove->prev) {
    toRemove->prev->next = toRemove->next;
  }

  if (toRemove->next) {
    toRemove->next->prev = toRemove->prev;
  }

  toRemove->next = NULL;
  toRemove->prev = NULL;
}


/*
  * use policy (first fit) to find the free block
  * @return found block address, if return NULL means size is not enough
*/
void * findFreeBlock(size_t requestedSize) {
  metadata_t * findPtr = freelist;
  while (findPtr != NULL) {
    if (!findPtr->isAllocate && findPtr->size >= requestedSize) {
      return findPtr;
    }
    findPtr = findPtr->next;
  }
  return findPtr;
}

/**
 * split block to size of bytesRequest, and insert the rest of block to free list
 * */
void * splitBlock(void * blockToSplit, size_t bytesRequest) {
  if (((metadata_t *)blockToSplit)->size == bytesRequest) {
    return blockToSplit;
  }

  removeList(blockToSplit);

  void * freeblock = blockToSplit + bytesRequest;
  initBlock(freeblock);
  ((metadata_t *)freeblock)->size = ((metadata_t *)blockToSplit)->size - bytesRequest;
  insertToFreeList(freeblock);

  ((metadata_t *)blockToSplit)->isAllocate = 1;
  ((metadata_t *)blockToSplit)->size = bytesRequest;
  return blockToSplit;
}

void* dmalloc(size_t numbytes) {
  if(freelist == NULL) {
    if(!dmalloc_init()) {
      return NULL;
    }
  }

  assert(numbytes > 0);

  /* your code here */
  numbytes = ALIGN(numbytes) + METADATA_T_ALIGNED;
  void * foundBlock = findFreeBlock(numbytes);
  if (foundBlock == NULL) {
    fprintf(stderr, "Error: heap size not enough\n");
    exit(EXIT_FAILURE);
  }

  foundBlock = splitBlock(foundBlock, numbytes);
  print_freelist();

  return foundBlock + METADATA_T_ALIGNED;
}

void dfree(void* ptr) {
  /* your code here */
}

/*
 * Allocate heap_region slab with a suitable syscall.
 */
bool dmalloc_init() {
  size_t max_bytes = ALIGN(MAX_HEAP_SIZE);

  /*
   * Get a slab with mmap, and put it on the freelist as one large block, starting
   * with an empty header.
   */
  freelist = (metadata_t*)
     mmap(NULL, max_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (freelist == (void *)-1) {
    perror("dmalloc_init: mmap failed");
    return false;
  }
  freelist->isAllocate = false;
  freelist->next = NULL;
  freelist->prev = NULL;
  freelist->size = max_bytes-METADATA_T_ALIGNED;

  // TODO: add prologue and epilogue block
  return true;
}



