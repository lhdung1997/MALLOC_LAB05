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
#define BYTE8 8
#define BYTE4 4
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define INFO_SIZE 28
#define PUTSIZE(pointer,value) (*(size_t*)pointer = value) //Put value at address that pointer holds
#define PUTPTR(pointer,value) (*(long*)pointer = value)
#define GETPNEXT(pointer) (*((char*)pointer+BYTE8))
#define GETPPREV(pointer) (*((char*)pointer+2*BYTE8))
#define GETSIZE(pointer) (*((size_t*)pointer))
#define GETPAYLOADPTR(pointer) ((char*)pointer+BYTE8)
/*void pointers are not allowed to perform arithmetic operation, this MACRO will transform*/
#define CAST_TO_BYTE_PTR(pointer) ((char*)pointer)
/*Use best fit policy when free list has little of free node*/
void* find_best_fit(void* free_pHead, int rounded_size);
/*Use first fit policy when free list has plenty of free node*/
void* find_first_fit(void* free_pHead, int rounded_size);
/*Allocate more heap when there are no more space in the free list*/
void* allocateMoreHeap(size_t rounded_size);
/*Join prev blocks and next blocks with current blocks when free memory and fix link.
The argument is the pointer to the HEADER, and return the pointer to the HEADER of new coalescing block
*/
void* join(void*pointer);
void* free_pHead;
/*
* mm_init - initialize the malloc package.
*/
int mm_init(void)
{
	/*Initialize free list pointer with SPECIAL BLOCK indicates the signal of the end of the list
	With HEADER = 0
	*/
	free_pHead = mem_sbrk(8BYTE);
	PUTSIZE(free_pHead,0x0);
	return 0;
}

/*
* mm_malloc - Allocate a block by incrementing the brk pointer.
*     Always allocate a block whose size is a multiple of the alignment.
*/
void *mm_malloc(size_t size)
{
	int newsize = ALIGN(size+INFO_SIZE);
	void* p = find_first_fit(free_pHead, newsize);
	if (p != NULL)
		return p;
	return allocateMoreHeap(newsize);
}

/*
* mm_free - Freeing a block does nothing.
*/
void mm_free(void *ptr)
{
	void* ptr = CAST_TO_BYTE_POINTER(ptr)-BYTE8;
	ptr = join(ptr);
	push_front(ptr);
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
/*This is first fit placement policy returning the pointer to the payload*/
void* find_first_fit(void*free_pHead, int rounded_size)
{
	void* traverse_ptr = free_pHead;
	while (*traverse_ptr != 0x0)
	{
		if (GETSIZE(traverse_ptr) >= rounded_size)
		{
			PUTSIZE(traverse_ptr,COMBINE(rounded_size,1));
			PUTSIZE(CAST_TO_BYTE_PTR(traverse_ptr)+(rounded_size-BYTE4),COMBINE(rounded_size,1));
			return GETPAYLOADPTR(traverse_ptr);
		}
			
		else
		{
			traverse_ptr = GETPNEXT(traverse_ptr);
		}
	}
	return NULL;// return null pointer when there is no appropriate block
}
/*This is best fit policy placement policy returning the pointer to the payload*/
void* find_best_fit(void *free_pHead, int rounded_size)
{
	int fit_size = -1;
	void* result_ptr = NULL;
	void* traverse_ptr = free_pHead;
	while (*traverse_ptr != 0x0)
	{
		if (GETSIZE(traverse_ptr) >= rounded_size)
		{
			if (fit_size == -1)
			{
				fit_size = GETSIZE(traverse_ptr);
				result_ptr = traverse_ptr;
			}
			else if (GETSIZE(traverse_ptr) < fit_size)
			{
				fit_size = GETSIZE(traverse_ptr);
				result_ptr = traverse_ptr;
			}
		}
		traverse_ptr = GETPNEXT(traverse_ptr);
	}
	if (result_ptr == NULL)
		return NULL;
	PUTSIZE(result_ptr,COMBINE(rounded_size,1));
	PUTSIZE(CAST_TO_BYTE_PTR(result_ptr)+rounded_size-BYTE4,COMBINE(rounded_size,1));
	return GETPAYLOADPTR(result_ptr);
}
void* allocateMoreHeap(size_t rounded_size)
{
	void* new_heap_ptr = mem_sbrk(rounded_size);
	PUTSIZE(new_heap_ptr, COMBINE(rounded_size, 1));
	PUTSIZE(CAST_TO_BYTE_PTR(new_heap_ptr) + rounded_size-BYTE4, COMBINE(rounded_size, 1));
	return GETPAYLOADPTR(new_heap_ptr);
}
void* join(void*pointer)
{
	/*Check if the next block is free, coalescene and fix link*/
	void* next_header_ptr = CAST_TO_BYTE_POINTER(pointer) + GETSIZE(pointer);
	int isallocated = GETSIZE(next_header_ptr) & 1;
	if(isallocated == 0)
	{
		void* tmp_ptr = *(CAST_TO_BYTE_POINTER(next_header_ptr) + BYTE8);//get pnext of next_block
		void* tmp_ptr_1 = *(CAST_TO_BYTE_POINTER(next_header_ptr)+2*BYTE8);//get prev of next_block
		void* tmp_ptr_2 = *(CAST_TO_BYTE_POINTER(tmp_ptr_1)+BYTE8); //get pnext of prev of next_block
		void* tmp_ptr_3 = *(CAST_TO_BYTE_POINTER(tmp_ptr)+2*BYTE8); //get prev of pnext of next_block
		tmp_ptr_2 = *(CAST_TO_BYTE_POINTER(next_header_ptr)+BYTE8); //pnext of prev of next_block = current block->pnext
		tmp_ptr_3 = *(CAST_TO_BYTE_POINTER(next_header_ptr)+2*BYTE8); //pprev of pnext of next_block = current block->prev
		int newsize = GETSIZE(pointer)+GETSIZE(next_header_ptr);
		PUTSIZE(pointer,COMBINE(newsize,0));
		PUTSIZE(CAST_TO_BYTE_POINTER(pointer)+newsize-BYTE4,COMBINE(newsize,0));
	}
	/*Check if prev block is free,coalescene and fix link*/
	void* prev_footer_ptr = CAST_TO_BYTE_POINTER(pointer)-BYTE4;
	isallocated = GETSIZE(prev_footer_ptr) & 1;
	void* prev_header_ptr = CAST_TO_BYTE_POINTER(prev_footer_ptr) - (GETSIZE(prev_footer_ptr) - BYTE4);
	if(isallocated == 0)
	{
		void *tmp_ptr = *(CAST_TO_BYTE_POINTER(prev_header_ptr)+BYTE8); //get pnext of prev block
		void* tmp_ptr_1 = *(CAST_TO_BYTE_POINTER(prev_header_ptr)+2*BYTE8); //get pprev of prev block
		void* tmp_ptr_2 = *(CAST_TO_BYTE_POINTER(tmp_ptr_1)+BYTE8); //get pnext of pprev of prev block
		void* tmp_ptr_3 = *(CAST_TO_BYTE_POINTER(tmp_ptr)+2*BYTE8); //get pprev of pnext of prev block
		tmp_ptr_2 = *(CAST_TO_BYTE_POINTER(prev_header_ptr)+BYTE8);
		tmp_ptr_3 = *(CAST_TO_BYTE_POINTER(prev_header_ptr)+2*BYTE8);
		int newsize = GETSIZE(pointer)+GETSIZE(prev_header_ptr);
		PUTSIZE(prev_header_ptr,COMBINE(newsize,0));
		PUTSIZE(CAST_TO_BYTE_POINTER(prev_header_ptr)+newsize - BYTE4,COMBINE(newsize,0));
		pointer = prev_header_ptr;
	}
	return pointer;
}
void push_front(void* pointer)
{
	PUT(CAST_TO_BYTE_POINTER(pointer) + BYTE8,free_pHead);
	PUT(CAST_TO_BYTE_POINTER(pointer) + 2*BYTE8,0x0);
	free_pHead = pointer;
}










