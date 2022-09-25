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

typedef struct  metadata_footer
{
  bool isAllocate;
  size_t size;
} metadata_footer_t;

#define METADATA_FOOTER_T_ALIGN (ALIGN(sizeof(metadata_footer_t)))



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

void initHeader(metadata_t * blockToInit) {
  blockToInit->isAllocate = false;
  blockToInit->size = 0;
  blockToInit->prev = NULL;
  blockToInit->next = NULL;
}

void insertToFreeList(metadata_t* blockToPush) {
  if (blockToPush->size == 0) {
    int * p = 0x0;
    *p = 10;
  }
  if (freelist) {
    blockToPush->next = freelist;
    freelist->prev = blockToPush;
  }
  freelist = blockToPush;
}

void removeList(metadata_t * toRemove) {
  if (freelist == toRemove) {
    freelist = freelist->next;
  }

  if (toRemove->prev != NULL) {
    toRemove->prev->next = toRemove->next;
  }

  if (toRemove->next != NULL) {
    toRemove->next->prev = toRemove->prev;
  }

  toRemove->next = NULL;
  toRemove->prev = NULL;
}

void * coalease(void * ptr) {
  metadata_t * prev_block = ptr - ((metadata_footer_t*)(ptr - METADATA_FOOTER_T_ALIGN))->size;
  metadata_t * next_block = ptr + ((metadata_t*)ptr)->size;
  metadata_t * block_header = NULL;
  metadata_footer_t * block_footer = NULL;
  void * coaleased_ptr = NULL;
  size_t sz = 0;

  if (prev_block->isAllocate && next_block->isAllocate) {
    block_header = ptr;
    block_footer = ptr + block_header->size - METADATA_FOOTER_T_ALIGN;
    block_header->isAllocate = false;
    block_footer->isAllocate = false;
    return block_header;
  } 
  
  if (prev_block->isAllocate && !next_block->isAllocate) {
    removeList(next_block);
    block_header = ptr;
    block_footer = ptr + ((metadata_t*)ptr)->size + next_block->size - METADATA_FOOTER_T_ALIGN;
    sz = ((metadata_t*)ptr)->size + next_block->size;
  } else if (!prev_block->isAllocate && next_block->isAllocate) {
    removeList(prev_block);
    block_header = prev_block;
    block_footer = ptr + ((metadata_t*)ptr)->size - METADATA_FOOTER_T_ALIGN;
    sz = ((metadata_t*)ptr)->size + prev_block->size;
  } else if (!prev_block->isAllocate && !next_block->isAllocate) {
    removeList(next_block);
    removeList(prev_block);
    block_header = prev_block;
    block_footer = ptr + ((metadata_t*)ptr)->size + next_block->size - METADATA_FOOTER_T_ALIGN;
    sz = prev_block->size + next_block->size + ((metadata_t*)ptr)->size;
  }

  block_header->size = sz;
  block_footer->size = sz;
  block_header->isAllocate = false;
  block_footer->isAllocate = false;
  coaleased_ptr = block_header;

  return coaleased_ptr;
}

/** *
 * @brief initialized header and footer to specified block
 * @param block to initialize
 * @param size size of this block (header + request_size + footer)
 * @param isAllocate indicate whether block is allocated
 */
void add_header_footer(void * block, size_t size, bool isAllocate) {
  metadata_t * header = block;
  header->size = size;
  header->isAllocate = isAllocate;
  header->prev = NULL;
  header->next = NULL;

  metadata_footer_t * footer = block + size - METADATA_FOOTER_T_ALIGN;
  footer->size = size;
  footer->isAllocate = isAllocate;
}


/*
  * use policy (first fit) to find the free block
  * @return found block address, if return NULL means size is not enough
*/
void * findFreeBlock(size_t requestedSize) {
  metadata_t * findPtr = freelist;
  while (findPtr != NULL) {
    if (!findPtr->isAllocate && findPtr->size >= requestedSize) {
      removeList(findPtr);
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
  if (((metadata_t *)blockToSplit)->size <= bytesRequest) {
    return blockToSplit;
  }

  size_t freeblock_size = ((metadata_t *)blockToSplit)->size - bytesRequest;
  if (freeblock_size < (METADATA_FOOTER_T_ALIGN + METADATA_T_ALIGNED)) {
    return blockToSplit;
  }

  void * freeblock = blockToSplit + bytesRequest;
  add_header_footer(freeblock, freeblock_size, false);
  insertToFreeList(freeblock);

  return blockToSplit;
}

void test_dmm_init_add_header() {
    size_t correctSize = ALIGN(MAX_HEAP_SIZE) - (METADATA_T_ALIGNED + (METADATA_FOOTER_T_ALIGN * 2));
    assert(freelist->size == correctSize);
    printf("===== dmm_init Test passed!======\n");
    exit(0);
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

  // add a prologue header and footer
  add_header_footer(freelist, METADATA_T_ALIGNED + METADATA_FOOTER_T_ALIGN, true);
  void * ptr = freelist;
  ptr += METADATA_T_ALIGNED + METADATA_FOOTER_T_ALIGN;
  size_t freelist_size = max_bytes - (METADATA_T_ALIGNED + 2 * METADATA_FOOTER_T_ALIGN);
  // add epilogue footer
  metadata_footer_t * footerPtr = ptr + freelist_size;
  footerPtr->size = METADATA_FOOTER_T_ALIGN;
  footerPtr->isAllocate = true;
   // init freelist header
  freelist = ptr;
  add_header_footer(freelist, freelist_size, false);
  // print_freelist();

  return true;
}

void* dmalloc(size_t numbytes) {
  if(freelist == NULL) {
    if(!dmalloc_init()) {
      return NULL;
    }
    // test_dmm_init_add_header();
  }

  assert(numbytes > 0);

  /* your code here */
  numbytes = ALIGN(numbytes) + METADATA_T_ALIGNED + METADATA_FOOTER_T_ALIGN;
  void * foundBlock = findFreeBlock(numbytes);
  if (foundBlock == NULL) {
    return NULL;
  }

  foundBlock = splitBlock(foundBlock, numbytes);
  add_header_footer(foundBlock, numbytes, true);

  return foundBlock + METADATA_T_ALIGNED;
}

void dfree(void* ptr) {
  /* your code here */
  metadata_t * ptr_header = ptr - METADATA_T_ALIGNED;
  ptr_header = coalease(ptr_header);
  insertToFreeList(ptr_header);
  // print_freelist();
}



