/*
 * dyn_block_management.c
 *
 *  Created on: Sep 21, 2022
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//===========================
// PRINT MEM BLOCK LISTS:
//===========================

void print_mem_block_lists()
{
	cprintf("\n=========================================\n");
	struct MemBlock* blk ;
	struct MemBlock* lastBlk = NULL ;
	cprintf("\nFreeMemBlocksList:\n");
	uint8 sorted = 1 ;
	LIST_FOREACH(blk, &FreeMemBlocksList)
	{
		if (lastBlk && blk->sva < lastBlk->sva + lastBlk->size)
			sorted = 0 ;
		cprintf("[%x, %x)-->", blk->sva, blk->sva + blk->size) ;
		lastBlk = blk;
	}
	if (!sorted)	cprintf("\nFreeMemBlocksList is NOT SORTED!!\n") ;

	lastBlk = NULL ;
	cprintf("\nAllocMemBlocksList:\n");
	sorted = 1 ;
	LIST_FOREACH(blk, &AllocMemBlocksList)
	{
		if (lastBlk && blk->sva < lastBlk->sva + lastBlk->size)
			sorted = 0 ;
		cprintf("[%x, %x)-->", blk->sva, blk->sva + blk->size) ;
		lastBlk = blk;
	}
	if (!sorted)	cprintf("\nAllocMemBlocksList is NOT SORTED!!\n") ;
	cprintf("\n=========================================\n");

}

//********************************************************************************//
//********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//===============================
// [1] INITIALIZE AVAILABLE LIST:
//===============================
void initialize_MemBlocksList(uint32 numOfBlocks)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] initialize_MemBlocksList
	// Write your code here, remove the panic and write your code
	//panic("initialize_MemBlocksList() is not implemented yet...!!");
	struct MemBlock MemBlockNodes[MAX_MEM_BLOCK_CNT];
	LIST_INIT(&AvailableMemBlocksList);
	LIST_INIT(&FreeMemBlocksList);
	LIST_INIT(&AllocMemBlocksList);

	MemBlockNodes[0].sva = 0;
	MemBlockNodes[0].size = 0;
//	MemBlockNodes.prev_next_info.le_prev = NULL;
	LIST_INSERT_HEAD(&AvailableMemBlocksList, &(MemBlockNodes[0]));
	for (int i = 1; i < numOfBlocks; i++)
	{
		MemBlockNodes[i].sva = 0;
		MemBlockNodes[i].size = 0;

		MemBlockNodes[i - 1].prev_next_info.le_next = &MemBlockNodes[i];
		MemBlockNodes[i].prev_next_info.le_prev = &MemBlockNodes[i - 1];
		//		if(i == numOfBlocks - 1)
//			MemBlockNodes[i].prev_next_info.le_next = NULL;
		LIST_INSERT_TAIL(&AvailableMemBlocksList, &(MemBlockNodes[i]));
	}
}

//===============================
// [2] FIND BLOCK:
//===============================
struct MemBlock *find_block(struct MemBlock_List *blockList, uint32 va)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] find_block
	// Write your code here, remove the panic and write your code
	//panic("find_block() is not implemented yet...!!");

	struct MemBlock *iterator;
	LIST_FOREACH(iterator, blockList) {
		if(iterator->sva == va) {
		return iterator;
		}
	}
	return NULL;

}

//=========================================
// [3] INSERT BLOCK IN ALLOC LIST [SORTED]:
//=========================================
void insert_sorted_allocList(struct MemBlock *blockToInsert)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] insert_sorted_allocList
	// Write your code here, remove the panic and write your code
	//	panic("insert_sorted_allocList() is not implemented yet...!!");
	int listSize;
	listSize = LIST_SIZE(&(AllocMemBlocksList));

	if(listSize == 0) {
		LIST_INSERT_HEAD(&(AllocMemBlocksList), blockToInsert);
	}
	else {
		struct MemBlock * iterator;
		LIST_FOREACH(iterator, &(AllocMemBlocksList)) {
			if(blockToInsert->sva > iterator->sva) {
				if(LIST_NEXT(iterator) == NULL) {
					LIST_INSERT_TAIL(&(AllocMemBlocksList), blockToInsert);
				}
				else {
					if(LIST_NEXT(iterator)->sva > blockToInsert->sva) {
						LIST_INSERT_AFTER(&(AllocMemBlocksList), iterator, blockToInsert);
					}
				}
			}
			else {
				if(LIST_PREV(iterator) == NULL) {
					LIST_INSERT_HEAD(&(AllocMemBlocksList), blockToInsert);
				}
				else {
					if(LIST_PREV(iterator)->sva < blockToInsert->sva) {
						LIST_INSERT_BEFORE(&(AllocMemBlocksList), iterator, blockToInsert);
					}
				}
			}
		}
	}
}

//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
struct MemBlock *alloc_block_FF(uint32 size)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] alloc_block_FF
	// Write your code here, remove the panic and write your code
	//	panic("alloc_block_FF() is not implemented yet...!!");
	struct MemBlock * iterator;
	LIST_FOREACH(iterator, &(FreeMemBlocksList)) {
		if(iterator->size == size) {
			LIST_REMOVE(&(FreeMemBlocksList), iterator);
			return iterator;
		}
		else if(iterator->size > size) {
			struct MemBlock returnedBlock;
			returnedBlock.size = size;
			returnedBlock.sva = iterator->sva;
			iterator->size = iterator->size - size;
			iterator->sva = iterator->sva + size;
			iterator = &(returnedBlock);
			LIST_REMOVE(&(AvailableMemBlocksList), LIST_FIRST(&(AvailableMemBlocksList)));
			return iterator;
		}
	}
	return NULL;
}

//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
struct MemBlock *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] alloc_block_BF
	// Write your code here, remove the panic and write your code
	//	panic("alloc_block_BF() is not implemented yet...!!");
	struct MemBlock * iterator;
	struct MemBlock * bestFitIterator = NULL;
	uint32 firstTime = 1;
	uint32 leastSize;
	LIST_FOREACH(iterator, &(FreeMemBlocksList)) {
		if(iterator->size == size) {
			LIST_REMOVE(&(FreeMemBlocksList), iterator);
			return iterator;
		}
		else if(iterator->size > size) {
			if(firstTime) {
				bestFitIterator = iterator;
				leastSize = bestFitIterator->size;
				firstTime = 0;
			}
			else {
				if(iterator->size < leastSize) {
					bestFitIterator = iterator;
					leastSize = bestFitIterator->size;
				}
			}
		}
	}

	if(bestFitIterator != NULL) {
		struct MemBlock returnedBlock;
		struct MemBlock * headBlockInAvailable;
		returnedBlock.size = size;
		returnedBlock.sva = bestFitIterator->sva;
		bestFitIterator->size = bestFitIterator->size - size;
		bestFitIterator->sva = bestFitIterator->sva + size;
		bestFitIterator = &(returnedBlock);
		headBlockInAvailable = LIST_FIRST(&(AvailableMemBlocksList));
		LIST_REMOVE(&(AvailableMemBlocksList), headBlockInAvailable);
		return bestFitIterator;
	}
	return NULL;
}


//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
struct MemBlock *alloc_block_NF(uint32 size)
{
	//TODO: [PROJECT MS1 - BONUS] [DYNAMIC ALLOCATOR] alloc_block_NF
	// Write your code here, remove the panic and write your code
	  panic("alloc_block_NF() is not implemented yet...!!");

}



//==================================================================================//
//============================ CUSTOM FUNCTIONS ==================================//
//==================================================================================//
uint32 sva_and_size(struct MemBlock *iterator) {
	return (iterator->sva + iterator->size);
}

void remove_from_free(struct MemBlock *iterator) {
	LIST_REMOVE(&(FreeMemBlocksList), iterator);
}

void send_to_available(struct MemBlock *iterator) {
	iterator->sva = 0;
	iterator->size = 0;
	LIST_INSERT_TAIL(&(AvailableMemBlocksList), iterator);
}

void merge_next(struct MemBlock *block, struct MemBlock *iterator) {
	block->size = block->size + iterator->size;
	remove_from_free(iterator);
	send_to_available(iterator);
}

void merge_prev(struct MemBlock *block, struct MemBlock *iterator) {
	block->size = block->size + iterator->size;
	remove_from_free(iterator);
	send_to_available(iterator);
}
//===================================================
// [8] INSERT BLOCK (SORTED WITH MERGE) IN FREE LIST:
//===================================================
void insert_sorted_with_merge_freeList(struct MemBlock *blockToInsert)
{
//	cprintf("BEFORE INSERT with MERGE: insert [%x, %x)\n=====================\n", blockToInsert->sva, blockToInsert->sva + blockToInsert->size);
//	print_mem_block_lists() ;

	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] insert_sorted_with_merge_freeList
	// Write your code here, remove the panic and write your code
	//	panic("insert_sorted_with_merge_freeList() is not implemented yet...!!");

	int listSize;
	listSize = LIST_SIZE(&(FreeMemBlocksList));

	if(listSize == 0) {
		LIST_INSERT_HEAD(&(FreeMemBlocksList), blockToInsert);
	}
	else {
		struct MemBlock *iterator;
		LIST_FOREACH(iterator, &(FreeMemBlocksList)) {
			if(blockToInsert->sva > iterator->sva) {
				if(LIST_NEXT(iterator) == NULL) {
					LIST_INSERT_TAIL(&(FreeMemBlocksList), blockToInsert);
				}
				else {
					if(LIST_NEXT(iterator)->sva > blockToInsert->sva) {
						LIST_INSERT_AFTER(&(FreeMemBlocksList), iterator, blockToInsert);
					}
				}
			}
			else {
				if(LIST_PREV(iterator) == NULL) {
					LIST_INSERT_HEAD(&(FreeMemBlocksList), blockToInsert);
				}
				else {
					if(LIST_PREV(iterator)->sva < blockToInsert->sva) {
						LIST_INSERT_BEFORE(&(FreeMemBlocksList), iterator, blockToInsert);
					}
				}
			}
		}

		if(LIST_NEXT(blockToInsert) != NULL) {
			if(sva_and_size(blockToInsert) == LIST_NEXT(blockToInsert)->sva) {
				merge_next(blockToInsert, LIST_NEXT(blockToInsert));
			}
		}

		if(LIST_PREV(blockToInsert) != NULL) {
			if(sva_and_size(LIST_PREV(blockToInsert)) == blockToInsert->sva) {
				merge_prev(LIST_PREV(blockToInsert), blockToInsert);
			}
		}
	}
//	cprintf("\nAFTER INSERT with MERGE:\n=====================\n");
//	print_mem_block_lists();
}
