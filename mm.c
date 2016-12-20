/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define WORDSIZE 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define INFO_SIZE 32
#define MIN_DATA_SIZE 10000
#define PUT(pointer,value) (*(long*)pointer = value) //Put value at address that pointer holds
#define GETPNEXT(pointer) (*((long*)pointer+1))
#define GETPPREV(pointer) (*((long*)pointer+2))
#define GETSIZE(pointer) (*((long*)pointer))
/*Use best fit policy when free list has little of free node*/
void* find_best_fit(void* free_pHead,int rounded_size);
/*Use first fit policy when free list has plenty of free node*/
void* find_first_fit(void* free_pHead,int rounded_size);
/*Allocate more heap when there are no more space in the free list*/
void* allocateMoreHeap(size_t rounded_size);
void* free_pHead;
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
	/*Initialize free list pointer with SPECIAL BLOCK indicates the signal of the end of the list
	  With HEADER = FOOTER = PNEXT = PREV = 0
	*/
	free_pHead = mem_sbrk(ALIGN(INFO_SIZE));
	PUT(free_pHead,0x0);
	PUT(free_pHead+WORDSIZE,0x0); //pnext = null
	PUT(free_pHead+2*WORDSIZE,0x0); //pprev = null
	PUT(free_pHead+3*WORDSIZE,0x0);
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}
void* find_first_fit(void*free_pHead,int rounded_size)
{
	int complete_size = rounded_size + INFO_SIZE;
	void* traverse_ptr = free_pHead;
	while(*traverse_ptr!=0x0)
	{
		if(*traverse_ptr == complete_size)
			return traverse_ptr;
		else
		{
			traverse_ptr = GETPNEXT(traverse_ptr);
		}
	}
}
void* find_best_fit(void *free_pHead,int rounded_size)
{
	int complete_size = rounded_size + INFO_SIZE;
	int fit_size = -1;
	void* result_ptr = NULL;
	void* traverse_ptr = free_pHead;
	while(*traverse_ptr!=0x0)
	{
		if(GETSIZE(traverse_ptr) >= complete_size)
		{
			fit_size = GETSIZE(traverse_ptr);
			result_ptr = traverse_ptr;
		}
	}
	if(result_ptr = NULL)
		return NULL;
	return result_ptr;
}












