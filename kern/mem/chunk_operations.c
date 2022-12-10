/*
 * chunk_operations.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include <kern/trap/fault_handler.h>
#include <kern/disk/pagefile_manager.h>
#include "kheap.h"
#include "memory_manager.h"


/******************************/
/*[1] RAM CHUNKS MANIPULATION */
/******************************/

//===============================
// 1) CUT-PASTE PAGES IN RAM:
//===============================
//This function should cut-paste the given number of pages from source_va to dest_va
//if the page table at any destination page in the range is not exist, it should create it
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int cut_paste_pages(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 num_of_pages)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] cut_paste_pages
	// Write your code here, remove the panic and write your code
	//panic("cut_paste_pages() is not implemented yet...!!");

	source_va = ROUNDDOWN(source_va, PAGE_SIZE);
	dest_va = ROUNDDOWN(dest_va, PAGE_SIZE);

	uint32 *source_va_page_table = NULL;
	uint32 *dest_va_page_table = NULL;

	uint32 dest_va_copy = dest_va;
	for(int i = 0; i < num_of_pages; i++) {
		struct FrameInfo *frame_checker = get_frame_info(page_directory, dest_va_copy, &dest_va_page_table);
		if(frame_checker != NULL) {
			return -1;
		}
		dest_va_copy += PAGE_SIZE;
	}

	dest_va_page_table = NULL;
	dest_va_copy = dest_va;
	for(int i = 0; i < num_of_pages; i++) {
		int page_checker = get_page_table(page_directory, dest_va_copy, &dest_va_page_table);
		if(page_checker == TABLE_NOT_EXIST) {
			create_page_table(page_directory, dest_va_copy);
		}
		dest_va_copy += PAGE_SIZE;
	}

	dest_va_page_table = NULL;
	for(int i = 0; i < num_of_pages; i++) {
		get_page_table(page_directory, source_va, &source_va_page_table);
		get_page_table(page_directory, dest_va, &dest_va_page_table);
		struct FrameInfo *frame_to_be_mapped = get_frame_info(page_directory, source_va, &source_va_page_table);
		int permissions = source_va_page_table[PTX(source_va)] & 0x00000FFF;
		map_frame(page_directory, frame_to_be_mapped, dest_va, permissions);
		unmap_frame(page_directory, source_va);
		source_va += PAGE_SIZE;
		dest_va += PAGE_SIZE;
	}
	return 0;
}

//===============================
// 2) COPY-PASTE RANGE IN RAM:
//===============================
//This function should copy-paste the given size from source_va to dest_va
//if the page table at any destination page in the range is not exist, it should create it
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int copy_paste_chunk(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 size)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] copy_paste_chunk
	// Write your code here, remove the panic and write your code
	//panic("copy_paste_chunk() is not implemented yet...!!");

	uint32 limit_source = source_va + size;
	uint32 limit_dest = dest_va + size;
	size = ROUNDUP(source_va + size ,PAGE_SIZE);
	uint32 source_page = ROUNDDOWN(source_va, PAGE_SIZE);
	uint32 dest_page = ROUNDDOWN(dest_va, PAGE_SIZE);

	while (source_page != size)
	{
		uint32 *source_page_table = NULL;
		uint32 *dest_page_table = NULL;
		struct FrameInfo *source_frame = NULL;
		struct FrameInfo *dest_frame = NULL;
		uint32 dest_perms ,source_perms;

		dest_frame = get_frame_info(page_directory, dest_page, &dest_page_table);
		if (dest_page_table == NULL){
			create_page_table(page_directory ,dest_page);
			pt_set_page_permissions(page_directory ,dest_page ,PERM_WRITEABLE ,0);
		}

		source_perms = pt_get_page_permissions(page_directory ,source_page);
		if (dest_frame == NULL){
			int ret = allocate_frame(&dest_frame);
			if (ret != 0){
				cprintf("NO MEMORY\n");
				return -1;
			}
			map_frame(page_directory ,dest_frame ,dest_page ,((source_perms & PERM_USER) | PERM_WRITEABLE));
		}

		dest_perms = pt_get_page_permissions(page_directory ,dest_page);
		if (!(dest_perms & PERM_WRITEABLE)){
			cprintf("READ ONLY\n");
			return -1;
		}

		dest_page += PAGE_SIZE;
		source_page += PAGE_SIZE;
	}

	while(source_va != limit_source)
	{
		unsigned char *tmp1 = (unsigned char *)(source_va);
		unsigned char *tmp2 = (unsigned char *)(dest_va);
		*tmp2 = *tmp1;

		dest_va += 1;
		source_va += 1;
	}

	return 0;
}

//===============================
// 3) SHARE RANGE IN RAM:
//===============================
//This function should share the given size from dest_va with the source_va
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int share_chunk(uint32* page_directory, uint32 source_va,uint32 dest_va, uint32 size, uint32 perms)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] share_chunk
	// Write your code here, remove the panic and write your code
	//panic("share_chunk() is not implemented yet...!!");

	source_va = ROUNDDOWN(source_va ,PAGE_SIZE);
	dest_va = ROUNDDOWN(dest_va ,PAGE_SIZE);
	size = ROUNDUP(source_va + size ,PAGE_SIZE);
	if(perms == PERM_USER)
		size += PAGE_SIZE;
	while(source_va != size)
	{
		uint32 *dest_page_table = NULL ,*source_page_table = NULL;
		struct FrameInfo *dest_frame = NULL ,*source_frame = NULL;

		source_frame = get_frame_info(page_directory ,source_va ,&source_page_table);
		if (source_page_table != NULL)
		{
			dest_frame = get_frame_info(page_directory ,dest_va ,&dest_page_table);
			if (dest_frame != NULL){
				cprintf("Destination Page is already mapped\n");
				return -1;
			}
			if (dest_page_table == NULL){
				create_page_table(page_directory ,dest_va);
				get_page_table(page_directory ,dest_va ,&dest_page_table);
				pt_clear_page_table_entry(page_directory ,dest_va);
			}

			map_frame(page_directory ,source_frame ,dest_va ,perms);
			uint32 pa1 = source_page_table[PTX(source_va)];
			uint32 pa2 = dest_page_table[PTX(dest_va)] ;
			if (perms == PERM_USER){
				cprintf("source : %x ,%x ,dest : %x ,%x\n",source_va ,pa1 ,dest_va ,pa2);
				cprintf("\nnot equal############################################################\n");
			}

		}
		else{
			cprintf("\nSource Doesn't Exist 555555555555555555555555555555555555555555555\n");
			return -1;
		}

		dest_va += PAGE_SIZE;
		source_va += PAGE_SIZE;
	}

	return 0;
}

//===============================
// 4) ALLOCATE CHUNK IN RAM:
//===============================
//This function should allocate in RAM the given range [va, va+size)
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int allocate_chunk(uint32* page_directory, uint32 va, uint32 size, uint32 perms)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] allocate_chunk
	struct FrameInfo* frames;
	uint32* table = NULL;

	int flag;
	size = ROUNDUP(va + size, PAGE_SIZE);
	va = ROUNDDOWN(va, PAGE_SIZE);

	while(va != size)
	{
	 flag = get_page_table(page_directory, va, &table);

	if (flag ==  TABLE_NOT_EXIST)
		create_page_table(page_directory, va);

	struct FrameInfo* frames = get_frame_info(page_directory, va, &table);

	if (frames == NULL)
	{

		flag = allocate_frame(&frames);

		if (flag != E_NO_MEM)
		{
			map_frame(page_directory, frames, va, perms);
			frames->va = va;
		}

		else
			return -1;

		va+=PAGE_SIZE;
	}
	else
		return -1;
	}
	return 0;
}


/*BONUS*/
//=====================================
// 5) CALCULATE ALLOCATED SPACE IN RAM:
//=====================================
void calculate_allocated_space(uint32* page_directory, uint32 sva, uint32 eva, uint32 *num_tables, uint32 *num_pages)
{
	//TODO: [PROJECT MS2 - BONUS] [CHUNK OPERATIONS] calculate_allocated_space
	// Write your code here, remove the panic and write your code
//	panic("calculate_allocated_space() is not implemented yet...!!");

	sva = ROUNDDOWN(sva, PAGE_SIZE);
	eva = ROUNDUP(eva, PAGE_SIZE);

	uint32 *sva_page_table_current = NULL;
	uint32 *sva_page_table_new = NULL;

	uint32 one_table_page_at_least = 0;

	uint32 num_of_tables = 0;
	uint32 num_of_pages = 0;
	while(sva != eva) {
		int table_checker = get_page_table(page_directory, sva, &sva_page_table_new);
		if(table_checker == TABLE_IN_MEMORY) {
			one_table_page_at_least = 1;
			if((sva_page_table_new[PTX(sva)] & PERM_PRESENT) != 0) {
				num_of_pages++;

				if(sva_page_table_current == NULL) {
					sva_page_table_current = sva_page_table_new;
					num_of_tables++;
				}
				else if(sva_page_table_current != NULL && sva_page_table_current != sva_page_table_new) {
					num_of_tables++;
				}
			}
		}
		sva += PAGE_SIZE;
	}

	if(one_table_page_at_least && num_of_tables == 0) {
		num_of_tables++;
	}

	*num_pages = num_of_pages;
	*num_tables = num_of_tables;
}

/*BONUS*/
//=====================================
// 6) CALCULATE REQUIRED FRAMES IN RAM:
//=====================================
// calculate_required_frames:
// calculates the new allocation size required for given address+size,
// we are not interested in knowing if pages or tables actually exist in memory or the page file,
// we are interested in knowing whether they are allocated or not.
uint32 calculate_required_frames(uint32* page_directory, uint32 sva, uint32 size)
{
	//TODO: [PROJECT MS2 - BONUS] [CHUNK OPERATIONS] calculate_required_frames
	// Write your code here, remove the panic and write your code
	panic("calculate_required_frames() is not implemented yet...!!");
}

//=================================================================================//
//===========================END RAM CHUNKS MANIPULATION ==========================//
//=================================================================================//

/*******************************/
/*[2] USER CHUNKS MANIPULATION */
/*******************************/

//======================================================
/// functions used for USER HEAP (malloc, free, ...)
//======================================================

//=====================================
// 1) ALLOCATE USER MEMORY:
//=====================================
void allocate_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
	// Write your code here, remove the panic and write your code
	panic("allocate_user_mem() is not implemented yet...!!");
}

//=====================================
// 2) FREE USER MEMORY:
//=====================================
void free_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
        //TODO: [PROJECT MS3] [USER HEAP - KERNEL SIDE] free_user_mem
        // Write your code here, remove the panic and write your code
        //panic("free_user_mem() is not implemented yet...!!");
        uint32* table;

        for(int va = virtual_address; va < virtual_address + size; va += PAGE_SIZE)
				pf_remove_env_page(e, va);

                for (int index = 0; index < e->page_WS_max_size; index++)
                {
                	uint32 WS_va = env_page_ws_get_virtual_address(e, index);

                        if(WS_va >= virtual_address && WS_va < virtual_address + size)
                        {
                                unmap_frame(e->env_page_directory, WS_va);
                                env_page_ws_clear_entry(e, index);

                        }
                }

                for(int va = virtual_address; va < virtual_address + size; va += PAGE_SIZE)
                {
                        bool flag = 1;
                        get_page_table(e->env_page_directory, va, &table);

                        if(table != NULL)
                        {
                                for(int index = 0; index < 1024; index++)
                                {
                                        if(table[index] != 0)
                                        {
                                                flag = 0;
                                                break;
                                        }
                                }
                                if(flag)
                                {
                                		e->env_page_directory[PDX(va)] = 0;
                                        kfree((void*) table);
                                }
                        }
                }


        //This function should:
        //1. Free ALL pages of the given range from the Page File
        //2. Free ONLY pages that are resident in the working set from the memory
        //3. Removes ONLY the empty page tables (i.e. not used) (no pages are mapped in the table)
}
//=====================================
// 2) FREE USER MEMORY (BUFFERING):
//=====================================
void __free_user_mem_with_buffering(struct Env* e, uint32 virtual_address, uint32 size)
{
	// your code is here, remove the panic and write your code
	panic("__free_user_mem_with_buffering() is not implemented yet...!!");

	//This function should:
	//1. Free ALL pages of the given range from the Page File
	//2. Free ONLY pages that are resident in the working set from the memory
	//3. Free any BUFFERED pages in the given range
	//4. Removes ONLY the empty page tables (i.e. not used) (no pages are mapped in the table)
}

//=====================================
// 3) MOVE USER MEMORY:
//=====================================
void move_user_mem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
{
	//TODO: [PROJECT MS3 - BONUS] [USER HEAP - KERNEL SIDE] move_user_mem
	//your code is here, remove the panic and write your code
	panic("move_user_mem() is not implemented yet...!!");

	// This function should move all pages from "src_virtual_address" to "dst_virtual_address"
	// with the given size
	// After finished, the src_virtual_address must no longer be accessed/exist in either page file
	// or main memory

	/**/
}

//=================================================================================//
//========================== END USER CHUNKS MANIPULATION =========================//
//=================================================================================//

