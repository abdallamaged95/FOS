#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

// KHEAP intervals
#define START 0xF6000000
#define END   0xFFFFF000

// Bytes mapped by a page
#define PAGE_SIZE	4096

// KHEAP pages number
#define NUM_OF_PAGES ((END - START) / PAGE_SIZE)

// Size of Memblocks to allocate NUM_OF_PAGES
#define REAL_SIZE ((NUM_OF_PAGES) * sizeof(struct MemBlock))

// Size of Memblocks to allocate NUM_OF_PAGES rounded
#define REAL_SIZE_ROUNDED ROUNDUP(NUM_OF_PAGES * sizeof(struct MemBlock), PAGE_SIZE)

//==================================================================//
//==================================================================//
//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)//
//==================================================================//
//==================================================================//

void initialize_dyn_block_system()
{
        //TODO: [PROJECT MS2] [KERNEL HEAP] initialize_dyn_block_system
		struct MemBlock* AvailableMemBlocksNodes = (struct MemBlock *) START;
		struct MemBlock* FreeMemBlocksNodes;

		MAX_MEM_BLOCK_CNT = NUM_OF_PAGES;

        LIST_INIT(&FreeMemBlocksList);
        LIST_INIT(&AllocMemBlocksList);

        allocate_chunk(ptr_page_directory, START, REAL_SIZE, (PERM_WRITEABLE | PERM_PRESENT | PERM_USED));

        AvailableMemBlocksNodes->sva = START;
        AvailableMemBlocksNodes->size = PAGE_SIZE;

        LIST_INSERT_HEAD(&AvailableMemBlocksList, AvailableMemBlocksNodes);


        for(int i = 1; i < (MAX_MEM_BLOCK_CNT - 1); i++)
        {
        		struct MemBlock* tmp = AvailableMemBlocksNodes;
                AvailableMemBlocksNodes++;

                tmp->prev_next_info.le_next = AvailableMemBlocksNodes;
                AvailableMemBlocksNodes->prev_next_info.le_prev = tmp;

				AvailableMemBlocksNodes->sva = START + i * PAGE_SIZE;
				AvailableMemBlocksNodes->size = PAGE_SIZE;

                LIST_INSERT_TAIL(&AvailableMemBlocksList, AvailableMemBlocksNodes);
        }
		struct MemBlock* tmp = AvailableMemBlocksNodes;

        AvailableMemBlocksNodes++;

        tmp->prev_next_info.le_next = AvailableMemBlocksNodes;
        AvailableMemBlocksNodes->prev_next_info.le_prev = tmp;

        FreeMemBlocksNodes = AvailableMemBlocksNodes;

        FreeMemBlocksNodes->sva = START + REAL_SIZE_ROUNDED;
        FreeMemBlocksNodes->size = END - FreeMemBlocksNodes->sva;

        LIST_INSERT_HEAD(&FreeMemBlocksList,FreeMemBlocksNodes);
}
void* kmalloc(unsigned int size)
{
	struct MemBlock* block;

	uint32 va;
	size = ROUNDUP(size, PAGE_SIZE);

	if(isKHeapPlacementStrategyFIRSTFIT())
	{

		block = alloc_block_FF(size);

		if(block!=NULL)
		{
			va = block->sva;
		    allocate_chunk(ptr_page_directory, va, size, (PERM_WRITEABLE | PERM_PRESENT | PERM_USED));
		}

		else
		 return NULL;

	}

	else if(isKHeapPlacementStrategyBESTFIT())
	{

		block = alloc_block_BF(size);

		if(block!=NULL)
		{
			va = block->sva;
		    allocate_chunk(ptr_page_directory, va, size, (PERM_WRITEABLE | PERM_PRESENT | PERM_USED));
		}

		else
		 return NULL;

	}

	else if(isKHeapPlacementStrategyNEXTFIT())
	{

		block = alloc_block_NF(size);

		if(block!=NULL)
		{
			va = block->sva;
		    allocate_chunk(ptr_page_directory, va, size, (PERM_WRITEABLE | PERM_USED));
		}

		else
		 return NULL;

	}
	insert_sorted_allocList(block);

	return (void*) va;
	}
void kfree(void* virtual_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kfree

	uint32 va = (uint32) virtual_address;

	struct MemBlock* block = (find_block(&(AllocMemBlocksList), va));
	va = ROUNDDOWN(va, PAGE_SIZE);

	if(block != NULL)
	{
		for(int i = va; i <ROUNDUP(block->size, PAGE_SIZE) + va; i+=PAGE_SIZE)
			unmap_frame(ptr_page_directory, i);

		LIST_REMOVE(&(AllocMemBlocksList), block);

		insert_sorted_with_merge_freeList(block);
	}
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kheap_virtual_address
	struct FrameInfo* frame = to_frame_info(physical_address);
    return frame->va;

}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kheap_physical_address
	uint32* page_index = NULL;
	uint32 frame;

	int flag = get_page_table(ptr_page_directory, virtual_address, &page_index);

 	if (flag != TABLE_NOT_EXIST)
	{
	    frame = page_index[PTX(virtual_address)];
		frame = frame >> 12;

		return (frame * PAGE_SIZE) + (virtual_address & 0x00000FFF);
	}
	return 0;
}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT MS2 - BONUS] [KERNEL HEAP] krealloc
	// Write your code here, remove the panic and write your code
	panic("krealloc() is not implemented yet...!!");
}
