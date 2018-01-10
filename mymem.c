#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>


/**
* Rajat Bansal 101019954
* SYSC 4001
**/
struct memoryList
{
  // doubly-linked list
  struct memoryList *last;
  struct memoryList *next;

  int size;            // How many bytes in this block?
  char alloc;          // 1 if this block is allocated,
                       // 0 if this block is free.
  void *ptr;           // location of block in memory pool.
};
void *allocateBlock(struct memoryList *block, size_t requested);
strategies myStrategy = NotSet;    // Current strategy

size_t mySize;
void *myMemory = NULL;

static struct memoryList *head;
static struct memoryList *last_block;


void initmem(strategies strategy, size_t sz)
{
	myStrategy = strategy;

	mySize = sz;

	if (myMemory != NULL)
    free(myMemory);
  if (head!=NULL)
    free(head);

  //alloc the memory structure and the blocks
	myMemory = malloc(sz);
	head = malloc(sizeof(struct memoryList));
  last_block = head;


  head->size = sz;
  head->alloc =0;
  head->last = head;
  head->next = last_block;
  head->ptr = myMemory;

}


void *mymalloc(size_t requested)
{

	assert((int)myStrategy > 0);

  struct memoryList *index=head;
  struct memoryList *temp=NULL;
  struct memoryList *empty_block =NULL;

	switch (myStrategy){

    case NotSet:
      break;

	  case First:
              if (index->alloc == 0 && index->size >= requested){
                empty_block= index;
                break;
              }
              for(index = head->next; index != head; index = index->next){
                if (index->alloc == 0 && index->size >= requested){
                  empty_block= index;
                  break;
                }
              }
              index = head;
              break;

	  case Best:

              do {
                if (index->alloc == 0 && index->size >= requested){
                  if(temp == NULL){
                    temp = index;
                  }
                  if(index->size < temp->size){
                    temp = index;
                  }
                }
                index = index->next;
              } while(index != head);
              empty_block = temp;
              break;

	  case Worst:
              do {
                if (index->alloc == 0){
                  if(temp == NULL){
                    temp = index;
                  }
                  if(index->size > temp->size){
                    temp = index;
                  }
                }
                index = index->next;
              } while(index != head);

              if (temp->size >= requested){
                empty_block = temp;
              }
              break;

	  case Next:
              index = last_block;
              if (index->alloc == 0 && index->size >= requested){
                empty_block= index;
                break;
              }
              for(index = head->next; index != head; index = index->next){
                if (index->alloc == 0 && index->size >= requested){
                  empty_block= index;
                  break;
                }
              }
              index = head;
              break;
	  }

    if(empty_block == NULL) {
      return NULL;
    }
  return allocateBlock(empty_block, requested);
}

void *allocateBlock(struct memoryList *block, size_t requested){
  if (block->size >requested){
    struct memoryList *hole = malloc (sizeof(struct memoryList));
    hole->size = block->size - requested;
    hole->alloc = 0;
    hole->next = block->next;
    hole->next->last = hole;
    hole->last = block;
    block->next = hole;
    hole->ptr = block->ptr +requested;
    block->size = requested;
    last_block = hole;
  }
  else{
    last_block = block->next;
  }
  block->alloc=1;
  return block->ptr;
}



void myfree(void* block)
{
  struct memoryList* freeblock = head;
  struct memoryList* index = head;
  do {
    if(index->ptr == block) {
      break;
    }
    index = index->next;
  } while(index != head);
  freeblock = index;
  freeblock->alloc = 0;

  if(freeblock->next != head && (freeblock->next->alloc==0)) {
    index = freeblock->next;
    freeblock->next = index->next;
    freeblock->next->last = myfree;
    freeblock->size += index->size;

    if(last_block == index) {
      last_block = freeblock;
    }
    free(index);
  }
  if(freeblock != head && (freeblock->last->alloc==0)) {
    index = freeblock->last;
    index->next = freeblock->next;
    index->next->last = index;
    index->size += freeblock->size;

    if(last_block == freeblock) {
      last_block = index;
    }
    free(freeblock);
    freeblock = index;
  }
}


int mem_holes()
{
  int counter = 0;
  struct memoryList* index = head;
  do {
    if(index->size <= mySize) {
      if (index->alloc == 0)
        counter++;
    }
    index = index->next;
  } while(index != head);

  return counter;
}

/* Get the number of bytes allocated */
int mem_allocated()
{
  int space_left= 0;

  struct memoryList* index = head;
  do {
    if(index->alloc == 0) {
      space_left += index->size;
    }
    index = index->next;
  } while(index != head);

  return mySize - space_left;
}

/* Number of non-allocated bytes */
int mem_free()
{
  int space_left= 0;

  struct memoryList* index = head;
  do {
    if(index->alloc == 0) {
      space_left += index->size;
    }
    index = index->next;
  } while(index != head);

  return space_left;
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free()
{
    int biggest = 0;
    struct memoryList* index = head;
    do {
      if(!(index->alloc) && index->size > biggest) {
        biggest = index->size;
      }
      index = index->next;
    } while(index != head);

    return biggest;

}

/* Number of free blocks smaller than "size" bytes. */
int mem_small_free(int size)
{
    int count = 0;
    struct memoryList* index = head;
    do {
      if(index->size <= size && index->alloc == 0) {
          count++;
      }
      index = index->next;
    } while(index != head);

    return count;
}

char mem_is_alloc(void *ptr)
{
    struct memoryList* index = head;
    while(index->next != head) {
      if(index->next->ptr > ptr) {
        return index->alloc;
      }
      index = index->next;
    }

    /* Iterator is now at the last block, so we assume the target is here */
    return index->alloc;
}

/*
 * Feel free to use these functions, but do not modify them.
 * The test code uses them, but you may ind them useful.
 */


//Returns a pointer to the memory pool.
void *mem_pool()
{
	return myMemory;
}

// Returns the total number of bytes in the memory pool. */
int mem_total()
{
	return mySize;
}


// Get string name for a strategy.
char *strategy_name(strategies strategy)
{
	switch (strategy)
	{
		case Best:
			return "best";
		case Worst:
			return "worst";
		case First:
			return "first";
		case Next:
			return "next";
		default:
			return "first";
	}
}

// Get strategy from name.
strategies strategyFromString(char * strategy)
{
	if (!strcmp(strategy,"best"))
	{
		return Best;
	}
	else if (!strcmp(strategy,"worst"))
	{
		return Worst;
	}
	else if (!strcmp(strategy,"first"))
	{
		return First;
	}
	else if (!strcmp(strategy,"next"))
	{
		return Next;
	}
	else
	{
		return 0;
	}
}


/*
 * These functions are for you to modify however you see fit.  These will not
 * be used in tests, but you may find them useful for debugging.
 */

/* Use this function to print out the current contents of memory. */
void print_memory()
{
	return;
}

/* Use this function to track memory allocation performance.
 * This function does not depend on your implementation,
 * but on the functions you wrote above.
 */
void print_memory_status()
{
	printf("%d out of %d bytes allocated.\n",mem_allocated(),mem_total());
	printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
	printf("Average hole size is %f.\n\n",((float)mem_free())/mem_holes());
}

/* Use this function to see what happens when your malloc and free
 * implementations are called.  Run "mem -try <args>" to call this function.
 * We have given you a simple example to start.
 */
void try_mymem(int argc, char **argv) {
        strategies strat;
	void *a, *b, *c, *d, *e;
	if(argc > 1)
	  strat = strategyFromString(argv[1]);
	else
	  strat = First;


	/* A simple example.
	   Each algorithm should produce a different layout. */

	initmem(strat,500);

	a = mymalloc(100);
	b = mymalloc(100);
	c = mymalloc(100);
	myfree(b);
	d = mymalloc(50);
	myfree(a);
	e = mymalloc(25);

	print_memory();
	print_memory_status();

}
