#include <stdio.h>  // needed for size_t
#include <unistd.h> // needed for sbrk
#include <assert.h> // needed for asserts
#include "dmm.h"
//14.62sec for stresstest 3//
/* You can improve the below metadata structure using the concepts from Bryant
 * and OHallaron book (chapter 9).
 */
void *findCorrectFreeList(size_t numbytes);

typedef struct metadata_H {
  /* size_t is the return type of the sizeof operator. Since the size of an
   * object depends on the architecture and its implementation, size_t is used
   * to represent the maximum size of any object in the particular
   * implementation. size contains the size of the data object or the number of
   * free bytes
   */
  size_t size;
  struct metadata_H *next;
  struct metadata_H *prev;
} metadata_t;

void assignCorrectFreeList(metadata_t *ptr, metadata_t *assign_ptr);

typedef struct metadata_F {
  size_t size;
} metadata_f;

/* freelist maintains all the blocks which are not in use; freelist is kept
 * sorted to improve coalescing efficiency
 */

size_t METADATA_F_ALIGNED = ALIGN(sizeof(metadata_f));

static metadata_t *freelist = NULL;
/* THE INITIAL FREE LIST, WITH MAX_HEAP_SIZE AS THE SIZE CLASS*/

static metadata_t *freelist32_64 = NULL;
static metadata_t *freelist64_128 = NULL;
static metadata_t *freelist128_256 = NULL;
static metadata_t *freelist256_512 = NULL;
static metadata_t *freelist512_1024 = NULL;
static metadata_t *freelist1024_2048 = NULL;
static metadata_t *freelist2048_4096 = NULL;
static metadata_t *freelist4096_8192 = NULL;
static metadata_t *freelist8192_ = NULL;
static metadata_t *tempFreelist = NULL;

//static metadata_t* freelist_target = NULL;
static metadata_t *heap_start;
static metadata_f *heap_end;

bool heapIsCompletelyFull = false;
bool dmalloc_init_called = false;

void *dmalloc(size_t numbytes) {
  printf("-----start of dmalloc-----\n");
  size_t
      currBlkSize = METADATA_T_ALIGNED + ALIGN(numbytes) + METADATA_F_ALIGNED;
  /* CHECK IF THERE'S STILL SPACE IN THE HEAP */
  /* initialize through sbrk call first time */
  if (!dmalloc_init_called) {
    printf("dmalloc_init()...\n");
    dmalloc_init_called = true;
    if (!dmalloc_init())
      return NULL;
  }

  assert(numbytes > 0);

  freelist = findCorrectFreeList(currBlkSize);
  /* find the appropriate pointer for splitting */
  metadata_t *currBlkHeader;
  metadata_t *ptr;
  ptr = freelist;

  size_t tempBlkSize = currBlkSize;
  while (ptr == NULL
      || ptr->size < currBlkSize + METADATA_T_ALIGNED + METADATA_F_ALIGNED) {
//      either the free list does not exist or this specific block is too small
    if (ptr == NULL) {
//        the free list does not exist
      if (freelist == freelist8192_) {
        printf("No block is big enough\n");
        return NULL;
      }
      tempBlkSize = tempBlkSize * 2;
      freelist = findCorrectFreeList(tempBlkSize);
      ptr = freelist;
    } else {
//        this specific block is too small, go to next free block
      printf("size:%zd < %zd\n", ptr->size, currBlkSize);
      ptr = ptr->next;
      if (ptr != NULL && ptr->next != NULL && ptr == ptr->next) {
        printf("LOOP!\n");
      }
      if (ptr == NULL) {
        if (freelist == freelist8192_) {
          printf("last possible block doesn't match! No block is big enough\n");
          return NULL;
        }
        /* the end of this free list, go to next free list */
        tempBlkSize = tempBlkSize * 2;
        freelist = findCorrectFreeList(tempBlkSize);
        ptr = freelist;
      }
    }
  }

  /* found the block! */
  currBlkHeader = ptr;
  printf("FOUND THE FREELIST: %d and THE BLOCK: %d with SIZE: %d\n",
         freelist,
         ptr,
         ptr->size);

  /* ------  splitting the target block --------- */

  /* ------  CASE 1: the block is completely filled --------- */
  if (currBlkHeader->size == currBlkSize) {
    /* ------  CASE 1-A: the first free block is being filled --------- */
    if (freelist == currBlkHeader) {
      /* ------  CASE 1-A-A: this is NOT the last free block --------- */
      printf("CASE 1-A-A\n");
      if (freelist->next != NULL) {
        freelist = freelist->next;
        currBlkHeader->next = NULL;
        freelist->prev = NULL;
        assignCorrectFreeList(freelist, freelist);
      }
        /* ------  CASE 1-A-B: this is the last free block --------- */
      else {
        /* EVERYTIME THE HEAP IS COMPLETELY FILLED, FREELIST IS RETURN TO HEAP_START */
        printf("CASE 1-A-B\n");
        assignCorrectFreeList(freelist, NULL);
      }
    }
      /* ------  CASE 1-B: it's NOT the first free block that's being filled --------- */
    else {
      /* ------  CASE 1-B-A: this IS THE LAST FREE BLOCK --------- */
      printf("CASE 1-B-A\n");
      if (currBlkHeader->next == NULL) {
        currBlkHeader->prev->next = NULL;
        currBlkHeader->prev = NULL;
        currBlkHeader->next = NULL;
      }
        /* ------  CASE 1-B-B: this IS NOT THE LAST FREE BLOCK --------- */
      else {
        printf("CASE 1-B-B\n");
        currBlkHeader->prev->next = currBlkHeader->next;
        currBlkHeader->next->prev = currBlkHeader->prev;
        currBlkHeader->next = NULL;
        currBlkHeader->prev = NULL;
      }
    }
  }
    /* ------  CASE 2: the block is NOT completely filled --------- */
  else {
    printf("CASE 2\n");
    /* create next block header */
    metadata_t *remainBlkHeader = (void *) currBlkHeader + currBlkSize;
    /* set next block header */
    remainBlkHeader->size = currBlkHeader->size - currBlkSize;
    metadata_t *remainBlkFreeList = findCorrectFreeList(remainBlkHeader->size);

    if (freelist != remainBlkFreeList) {
//        the remaining block is no longer belongs to the current free list
//        so we need to assign this block to a different free list, which is
//        remainBlkFreeList, and we also need to remove the current block from
//        the current free list
      if (currBlkHeader->prev != NULL)
        currBlkHeader->prev->next = currBlkHeader->next;
      if (currBlkHeader->next != NULL)
        currBlkHeader->next->prev = currBlkHeader->prev;

      if (remainBlkFreeList == NULL) {
//          if this remainBlkFreeList is NULL, then this is an empty list, and
//          all we need is to make remainBlkFreeList equal to remainBlkHeader
        assignCorrectFreeList(remainBlkHeader, remainBlkHeader);
        remainBlkHeader->next = NULL;
        remainBlkHeader->prev = NULL;
      } else {
        ptr = remainBlkFreeList;
        if (ptr > remainBlkHeader) {
//            the start of the free list is already after remainBlkHeader
//            remainBlkHeader will be the first block
          remainBlkHeader->next = ptr;
          ptr->prev = remainBlkHeader;
          remainBlkHeader->prev = NULL;
        } else {
//          go to a block that's right after remainBlkHeader
//          or, in the case where remainBlkHeader is the last block, go to the
//          block just before it
          while (ptr < remainBlkHeader) {
            if (ptr->next != NULL) {
              ptr = ptr->next;
            } else {
              break;
            }
          }
          if (ptr > remainBlkHeader) {
//            ptr is a block that's right after remainBlkHeader
            ptr->prev->next = remainBlkHeader;
            remainBlkHeader->prev = ptr->prev;
            remainBlkHeader->next = ptr;
            ptr->prev = remainBlkHeader;
          } else {
//              remainBlkHeader is the last block
            ptr->next = remainBlkHeader;
            remainBlkHeader->prev = ptr;
            remainBlkHeader->next = NULL;
          }
        }
      }
    } else {
//        the remaining block still fits in the free list, this is easier
      remainBlkHeader->next = currBlkHeader->next;
      remainBlkHeader->prev = currBlkHeader->prev;
      if (remainBlkHeader->next != NULL)
        remainBlkHeader->next->prev = remainBlkHeader;
      if (remainBlkHeader->prev != NULL) {
        remainBlkHeader->prev->next = remainBlkHeader;
      } else {
        assignCorrectFreeList(remainBlkHeader, remainBlkHeader);
      }
    }

    /* set next block footer */
    metadata_f *nextBlkFtr =
        (void *) remainBlkHeader + remainBlkHeader->size - METADATA_F_ALIGNED;
    nextBlkFtr->size = remainBlkHeader->size;
    /* create current block footer */
    metadata_f *currBlkFtr = (void *) remainBlkHeader - METADATA_F_ALIGNED;
    /* set current block footer */
    currBlkFtr->size = currBlkSize;
    /* set current block header */
    currBlkHeader->size = currBlkSize;
    currBlkHeader->next = NULL;
    currBlkHeader->prev = NULL;
  }

  /* return the correct pointer address */
  return (void *) currBlkHeader + METADATA_T_ALIGNED;
}

void dfree(void *ptr) {
  /* your code here */
  metadata_t *currBlkHeader = (void *) ptr - METADATA_T_ALIGNED;
  printf("CUREENT BLOCK SIZE: %zd\n", currBlkHeader->size);
  printf("1\n");
  freelist = findCorrectFreeList(currBlkHeader->size);

  if (freelist == NULL) {
    assignCorrectFreeList(currBlkHeader, currBlkHeader);
  } else if (currBlkHeader < freelist) {
    printf("currBlkHeader is before freelist\n");
    if ((void *) currBlkHeader + currBlkHeader->size == (void *) freelist) {
      /* merge needed */
      /* set freePointer header */
      currBlkHeader->size += freelist->size;
      currBlkHeader->next = freelist->next;
      currBlkHeader->prev = NULL;

      if (freelist->next != NULL)
        freelist->next->prev = currBlkHeader;

      /* modify freelist header */
      freelist->next = NULL;
      freelist->prev = NULL;
      /* set merged footer */
      metadata_f *mergedFooter =
          (void *) currBlkHeader + currBlkHeader->size - METADATA_F_ALIGNED;
      mergedFooter->size = currBlkHeader->size;
      /* adjust freelist */
      assignCorrectFreeList(freelist, currBlkHeader);
    } else {
      /* merge NOT needed */
      /* set freePointer header */
      currBlkHeader->next = freelist;
      freelist->prev = currBlkHeader;
      currBlkHeader->prev = NULL;
      /* adjust freelist */
      assignCorrectFreeList(currBlkHeader, currBlkHeader);
    }
  } else {
    printf("currBlkHeader is after freelist\n");
    /* find the blocks between which freePointer lies */
    metadata_t *ptr = freelist;

    /* if this is not the last free block and the next free block is before the freeing block: */
    while (ptr->next != NULL && ptr->next < currBlkHeader) {
      ptr = ptr->next;
    }

    if (ptr->next == NULL) {
      /*if the previous block is free */
      if ((void *) ptr + ptr->size == (void *) currBlkHeader) {
        /* update merged header */
        ptr->size += currBlkHeader->size;
        /* update footer */
        metadata_f *mergedFtr = (void *) ptr + ptr->size - METADATA_F_ALIGNED;
        mergedFtr->size = ptr->size;
      } else {
        /*if the previous block is allocated */
        /* create next and prev pointers to add the block to freelist */
        currBlkHeader->prev = ptr;
        ptr->next = currBlkHeader;
        currBlkHeader->next = NULL;
      }
    } else {
      /*if it is in the middle: */
      metadata_t *nextFreePointer = ptr->next;
      bool previousBlockIsFree =
          (void *) currBlkHeader == (void *) ptr + ptr->size ? true : false;
      bool nextBlockIsFree = (void *) currBlkHeader + currBlkHeader->size
                                 == (void *) nextFreePointer ? true : false;
      int CASE;

      if (previousBlockIsFree && nextBlockIsFree) {
        CASE = 1;
      } else if (previousBlockIsFree && !nextBlockIsFree) {
        CASE = 2;
      } else if (!previousBlockIsFree && nextBlockIsFree) {
        CASE = 3;
      } else {
        CASE = 4;
      }
      metadata_f *newFtr;
      switch (CASE) {
        case 1:
          /* realign pointers */
          if (nextFreePointer->next != NULL) {
            /* if next free pointer is not the last free block */
            ptr->next = nextFreePointer->next;
            ptr->next->prev = ptr;
          } else {
            /* if next free pointer IS the last free block */
            ptr->next = NULL;
          }

          nextFreePointer->prev = NULL;
          nextFreePointer->next = NULL;
          currBlkHeader->prev = NULL;
          currBlkHeader->next = NULL;
          /* adjust size in header */
          ptr->size += currBlkHeader->size + nextFreePointer->size;
          /* adjust size in new footer */
          newFtr = (void *) nextFreePointer + nextFreePointer->size
              - METADATA_F_ALIGNED;
          newFtr->size = ptr->size;
          break;

        case 2:
          /* adjust size in header */
          ptr->size += currBlkHeader->size;
          /* adjust size in new footer */
          newFtr = (void *) ptr + ptr->size - METADATA_F_ALIGNED;
          newFtr->size = ptr->size;
          currBlkHeader->prev = NULL;
          currBlkHeader->next = NULL;
          break;

        case 3:
          /* realign pointers */
          ptr->next = currBlkHeader;
          currBlkHeader->prev = ptr;
          currBlkHeader->next = nextFreePointer->next;

          if (nextFreePointer->next != NULL) {
            nextFreePointer->next->prev = currBlkHeader;
          }

          nextFreePointer->prev = NULL;
          nextFreePointer->next = NULL;
          /* adjust size in new header */
          currBlkHeader->size += nextFreePointer->size;
          /* adjust size in footer */
          newFtr = (void *) nextFreePointer + nextFreePointer->size
              - METADATA_F_ALIGNED;
          newFtr->size = currBlkHeader->size;
          break;

        case 4:
          /* realign pointers */
          ptr->next = currBlkHeader;
          currBlkHeader->prev = ptr;
          currBlkHeader->next = nextFreePointer;
          nextFreePointer->prev = currBlkHeader;
          break;
      }
    }
  }
}

bool dmalloc_init() {
  /* Two choices:
   * 1. Append prologue and epilogue blocks to the start and the
   * end of the freelist
   *
   * 2. Initialize freelist pointers to NULL
   *
   * Note: We provide the code for 2. Using 1 will help you to tackle the
   * corner cases succinctly.
   */
  size_t max_bytes = ALIGN(MAX_HEAP_SIZE);
  printf("max_bytes %zd\n", max_bytes);
  /* returns heap_region, which is initialized to freelist */
  freelist = (metadata_t *) sbrk(max_bytes);
  printf("pointer: %u\n", freelist);

  /* Q: Why casting is used? i.e., why (void*)-1?  WHY? */
  if (freelist == (void *) -1)
    return false;
  /*setting heap_start*/
  heap_start = freelist;
  heap_start->next = NULL;
  heap_start->prev = NULL;
  heap_start->size = max_bytes;
  /*setting heap_end*/
  heap_end = (void *) freelist + max_bytes - METADATA_F_ALIGNED;
  heap_end->size = max_bytes;
  assignCorrectFreeList(heap_start, heap_start);
  return true;
}

/* for debugging; can be turned off through -NDEBUG flag*/
void print_freelist() {
  metadata_t *freelist_head = NULL;

  while (freelist_head != NULL) {
    DEBUG("\tFreelist Size:%zd, Head:%p, Prev:%p, Next:%p\t",
          freelist_head->size,
          freelist_head,
          freelist_head->prev,
          freelist_head->next);
    freelist_head = freelist_head->next;
  }

  DEBUG("\n");
}

void *findCorrectFreeList(size_t size) {
  if (size <= 64) {
    printf("FOUND freelist32_64: %d\n",
           freelist32_64 != NULL ? freelist32_64 : -1);
    return freelist32_64;
  } else if (size <= 128) {
    printf("FOUND freelist64_128: %d\n",
           freelist64_128 != NULL ? freelist64_128 : -1);
    return freelist64_128;
  } else if (size <= 256) {
    printf("FOUND freelist128_256: %d\n",
           freelist128_256 != NULL ? freelist128_256 : -1);
    return freelist128_256;
  } else if (size <= 512) {
    printf("FOUND freelist256_512: %d\n",
           freelist256_512 != NULL ? freelist256_512 : -1);
    return freelist256_512;
  } else if (size <= 1024) {
    printf("FOUND freelist512_1024: %d\n",
           freelist512_1024 != NULL ? freelist512_1024 : -1);
    return freelist512_1024;
  } else if (size <= 2048) {
    printf("FOUND freelist1024_2048: %d\n",
           freelist1024_2048 != NULL ? freelist1024_2048 : -1);
    return freelist1024_2048;
  } else if (size <= 4096) {
    printf("FOUND freelist2048_4096: %d\n",
           freelist2048_4096 != NULL ? freelist2048_4096 : -1);
    return freelist2048_4096;
  } else if (size <= 8192) {
    printf("FOUND freelist4096_8192: %d\n",
           freelist4096_8192 != NULL ? freelist4096_8192 : -1);
    return freelist4096_8192;
  } else {
    printf("FOUND freelist8192_: %d\n",
           freelist8192_ != NULL ? freelist8192_ : -1);
    return freelist8192_;
  }
}

void assignCorrectFreeList(metadata_t *ptr, metadata_t *assign_ptr) {
  if (ptr->size <= 64) {
    freelist32_64 = assign_ptr;
  } else if (ptr->size <= 128) {
    freelist64_128 = assign_ptr;
  } else if (ptr->size <= 256) {
    freelist128_256 = assign_ptr;
  } else if (ptr->size <= 512) {
    freelist256_512 = assign_ptr;
  } else if (ptr->size <= 1024) {
    freelist512_1024 = assign_ptr;
  } else if (ptr->size <= 2048) {
    freelist1024_2048 = assign_ptr;
  } else if (ptr->size <= 4096) {
    freelist2048_4096 = assign_ptr;
  } else if (ptr->size <= 8192) {
    freelist4096_8192 = assign_ptr;
  } else {
    freelist8192_ = assign_ptr;
  }
}