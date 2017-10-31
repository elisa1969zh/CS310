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

void *dmalloc(size_t numbytes) {

  /* CHECK IF THERE'S STILL SPACE IN THE HEAP */
  if (heapIsCompletelyFull) {
    return NULL;
  } else {
    /* initialize through sbrk call first time */
    if (freelist == NULL) {
      if (!dmalloc_init())
        return NULL;
    }

    assert(numbytes > 0);
    /* determine size of requested block */
    size_t
        blockSize = METADATA_T_ALIGNED + ALIGN(numbytes) + METADATA_F_ALIGNED;
    size_t tempBlockSize = blockSize;
    freelist = findCorrectFreeList(blockSize);
    /* find the appropriate pointer for splitting */
    metadata_t *targetPointer;
    metadata_t *candidatePointer;
    candidatePointer = freelist;

    while ((candidatePointer == NULL)
        || (candidatePointer->size < ALIGN(numbytes))
            && (candidatePointer->size != blockSize)) {
      /* this block is too small */
      if ((candidatePointer == NULL) || (candidatePointer->next == NULL)) {
        if ((candidatePointer != NULL) && (freelist == freelist8192_)) {
          /* last possible block doesn't match! No block is big enough */
          return NULL;
        } else {
          tempBlockSize = tempBlockSize * 2;
          freelist = findCorrectFreeList(tempBlockSize);
          candidatePointer = freelist;
        }
      } else {
        /* go to next free block */
        candidatePointer = candidatePointer->next;
      }
    }

    /* found the block! */
    targetPointer = candidatePointer;

    /* ------  splitting the target block --------- */

    /* ------  CASE 1: the block is completely filled --------- */
    if (targetPointer->size == blockSize) {
      /* ------  CASE 1-A: the first free block is being filled --------- */
      if (freelist == targetPointer) {
        /* ------  CASE 1-A-A: this is NOT the last free block --------- */
        if (freelist->next != NULL) {
          freelist = freelist->next;
          targetPointer->next = NULL;
          freelist->prev = NULL;
          assignCorrectFreeList(freelist, freelist);
        }
          /* ------  CASE 1-A-B: this is the last free block --------- */
        else {
          /* EVERYTIME THE HEAP IS COMPLETELY FILLED, FREELIST IS RETURN TO HEAP_START */
          assignCorrectFreeList(freelist, NULL);
        }
      }
        /* ------  CASE 1-B: it's NOT the first free block that's being filled --------- */
      else {
        /* ------  CASE 1-B-A: this IS THE LAST FREE BLOCK --------- */
        if (targetPointer->next == NULL) {
          targetPointer->prev->next = NULL;
          targetPointer->prev = NULL;
          targetPointer->next = NULL;
        }
          /* ------  CASE 1-B-B: this IS NOT THE LAST FREE BLOCK --------- */
        else {
          targetPointer->prev->next = targetPointer->next;
          targetPointer->next->prev = targetPointer->prev;
          targetPointer->next = NULL;
          targetPointer->prev = NULL;
        }
      }
    }
      /* ------  CASE 2: the block is NOT completely filled --------- */
    else {
      printf("CASE 2\n");
      /* create next block header */
      metadata_t *header = (void *) targetPointer + blockSize;
      /* set next block header */
      header->size = targetPointer->size - blockSize;
      metadata_t *targetfreelist = findCorrectFreeList(header->size);

      if (targetfreelist == NULL) {
        assignCorrectFreeList(header, header);
        header->next = NULL;
        header->prev = NULL;
      } else {
        while (targetfreelist->next != NULL && targetfreelist->next < header) {
          targetfreelist = targetfreelist->next;
        }

        if (targetfreelist > header) {
          header->next = targetfreelist;
          header->prev = targetfreelist->prev;
          if (header->prev != NULL) {
            header->prev->next = header;
          }
          targetfreelist->prev = header;
        } else {
          if (targetfreelist->next != NULL) {
            targetfreelist->next->prev = header;
            header->next = targetfreelist->next;
            targetfreelist->next = header;
            header->prev = targetfreelist;
          } else {
            header->next = NULL;
            targetfreelist->next = header;
            header->prev = targetfreelist;
          }

        }
      }

      /* align pointers to next block header */
      if (header->next != NULL) {
        header->next->prev = header;
      }

      if (header->prev != NULL) {
        header->prev->next = header;
      }

      /* set next block footer */
      metadata_f
          *nextFooter = (void *) header + header->size - METADATA_F_ALIGNED;
      nextFooter->size = header->size;
      /* create current block footer */
      metadata_f *currentFooter = (void *) header - METADATA_F_ALIGNED;
      /* set current block footer */
      currentFooter->size = blockSize;
      /* set current block header */
      targetPointer->size = blockSize;
      targetPointer->next = NULL;
      targetPointer->prev = NULL;
      /* update freelist if freelist's block becomes allocated */
      freelist =
          ((void *) freelist == (void *) targetPointer) ? header : freelist;
    }

    /* return the correct pointer address */
    return (void *) targetPointer + METADATA_T_ALIGNED;
  }
}

void dfree(void *ptr) {
  /* your code here */
  metadata_t *freePointer = (void *) ptr - METADATA_T_ALIGNED;

  /* CHECK IF HEAP IS COMPLETELY FILLED, IF YES, HEAP WILL NOT BE FULL ANYMORE*/
  if (heapIsCompletelyFull) {
    heapIsCompletelyFull = false;
    freelist = freePointer;
    freelist->next = NULL;
    freelist->prev = NULL;
  } else {
    if (freePointer < freelist) {
      if ((void *) freePointer + freePointer->size == (void *) freelist) {
        /* set freePointer header */
        freePointer->size += freelist->size;
        freePointer->next = freelist->next;

        if (freelist->next != NULL) {
          freelist->next->prev = freePointer;
        }

        /* modify freelist header */
        freelist->next = NULL;
        freelist->prev = NULL;
        /* set merged footer */
        metadata_f *mergedFooter2AA =
            (void *) freePointer + freePointer->size - METADATA_F_ALIGNED;
        mergedFooter2AA->size = freePointer->size;
        /* adjust freelist */
        freelist = freePointer;
      } else {
        /* set freePointer header */
        freePointer->next = freelist;
        freelist->prev = freePointer;
        freePointer->prev = NULL;
        /* adjust freelist */
        freelist = freePointer;
      }
    } else {
      /* find the blocks between which freePointer lies */
      metadata_t *previousFreePointer = freelist;

      /* if this is not the last free block and the next free block is before the freeing block: */
      while (previousFreePointer->next != NULL
          && previousFreePointer->next < freePointer) {
        previousFreePointer = previousFreePointer->next;
      }

      if (previousFreePointer->next == NULL) {

        /*if the previous block is free */
        if ((void *) previousFreePointer + previousFreePointer->size
            == (void *) freePointer) {
          /* update merged header */
          previousFreePointer->size += freePointer->size;
          /* update footer */
          metadata_f *mergedFooterF_B_A_A =
              (void *) previousFreePointer + previousFreePointer->size
                  - METADATA_F_ALIGNED;
          mergedFooterF_B_A_A->size = previousFreePointer->size;
        }
          /*if the previous block is allocated */
        else {
          /* create next and prev pointers to add the block to freelist */
          freePointer->prev = previousFreePointer;
          previousFreePointer->next = freePointer;
          freePointer->next = NULL;
        }
      }
        /*if it is in the middle: */
      else {
        metadata_t *nextFreePointer = previousFreePointer->next;
        bool previousBlockIsFree = ((void *) freePointer
            == (void *) previousFreePointer + previousFreePointer->size) ? true
                                                                         : false;
        bool nextBlockIsFree = ((void *) freePointer + freePointer->size
            == (void *) nextFreePointer) ? true : false;
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

        switch (CASE) {
          case 1:


            /* realign pointers */
            if (nextFreePointer->next != NULL) {
              /* if next free pointer is not the last free block */
              previousFreePointer->next = nextFreePointer->next;
              nextFreePointer->next->prev = previousFreePointer;
            } else {
              /* if next free pointer IS the last free block */
              previousFreePointer->next = NULL;
            }

            nextFreePointer->prev = NULL;
            nextFreePointer->next = NULL;
            freePointer->prev = NULL;
            freePointer->next = NULL;
            /* adjust size in header */
            previousFreePointer->size +=
                freePointer->size + nextFreePointer->size;
            /* adjust size in new footer */
            metadata_f *newFooterF_B_B_1 =
                (void *) nextFreePointer + nextFreePointer->size
                    - METADATA_F_ALIGNED;
            newFooterF_B_B_1->size = previousFreePointer->size;
            break;

          case 2:
            /* adjust size in header */
            previousFreePointer->size += freePointer->size;
            /* adjust size in new footer */
            metadata_f *newFooterf_B_B_2 =
                (void *) previousFreePointer + previousFreePointer->size
                    - METADATA_F_ALIGNED;
            newFooterf_B_B_2->size = previousFreePointer->size;
            break;

          case 3:
            /* realign pointers */
            previousFreePointer->next = freePointer;
            freePointer->prev = previousFreePointer;
            freePointer->next = nextFreePointer->next;

            if (nextFreePointer->next != NULL) {
              nextFreePointer->next->prev = freePointer;
            }

            nextFreePointer->prev = NULL;
            nextFreePointer->next = NULL;
            /* adjust size in new header */
            freePointer->size += nextFreePointer->size;
            /* adjust size in footer */
            metadata_f *newFooterF_B_B_3 =
                (void *) nextFreePointer + nextFreePointer->size
                    - METADATA_F_ALIGNED;
            newFooterF_B_B_3->size = freePointer->size;
            break;

          case 4:
            /* realign pointers */
            previousFreePointer->next = freePointer;
            freePointer->prev = previousFreePointer;
            freePointer->next = nextFreePointer;
            nextFreePointer->prev = freePointer;
            break;
        }
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
  printf("METADATA_F_ALIGNED: %zd\n", METADATA_F_ALIGNED);
  printf("METADATA_T_ALIGNED: %zd\n", METADATA_T_ALIGNED);
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

void *findCorrectFreeList(size_t numbytes) {
  if (numbytes <= 64) {
    return freelist32_64;
  } else if (numbytes <= 128) {
    return freelist64_128;
  } else if (numbytes <= 256) {
    return freelist128_256;
  } else if (numbytes <= 512) {
    return freelist256_512;
  } else if (numbytes <= 1024) {
    return freelist512_1024;
  } else if (numbytes <= 2048) {
    return freelist1024_2048;
  } else if (numbytes <= 4096) {
    return freelist2048_4096;
  } else if (numbytes <= 8192) {
    return freelist4096_8192;
  } else {
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