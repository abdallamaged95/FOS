/*
 * paging_helpers.c
 *
 *  Created on: Sep 30, 2022
 *      Author: HP
 */
#include "memory_manager.h"

/*[2.1] PAGE TABLE ENTRIES MANIPULATION */
inline void pt_set_page_permissions(uint32* page_directory, uint32 virtual_address, uint32 permissions_to_set, uint32 permissions_to_clear)
{
	//TODO: [PROJECT MS2] [PAGING HELPERS] pt_set_page_permissions
//	cprintf("set : %d\n",permissions_to_set);
//	cprintf("clear : %d\n",permissions_to_clear);
	uint32 *page_table_entry;
	uint32 exist = get_page_table(page_directory , virtual_address , &page_table_entry);
	if(!exist){
		page_table_entry[PTX(virtual_address)] =
				page_table_entry[PTX(virtual_address)] | permissions_to_set;
		page_table_entry[PTX(virtual_address)] =
				page_table_entry[PTX(virtual_address)] & (~permissions_to_clear);
		tlb_invalidate((void*)NULL , (void *)virtual_address);
	}
	else{
		panic("\nDEVELOPER PANIC : this address is invalid");
	}

}

inline int pt_get_page_permissions(uint32* page_directory, uint32 virtual_address )
{
	//TODO: [PROJECT MS2] [PAGING HELPERS] pt_get_page_permissions
	uint32 *page_table_entry;
		uint32 exist = get_page_table(page_directory , virtual_address , &page_table_entry);
		if(!exist){
			uint32 perms = page_table_entry[PTX(virtual_address)] & 0xFFF;
			return perms;
		}
		else
			return -1;
}

inline void pt_clear_page_table_entry(uint32* page_directory, uint32 virtual_address)
{
	//TODO: [PROJECT MS2] [PAGING HELPERS] pt_clear_page_table_entry

	uint32 *page_table_entry;
	uint32 exist = get_page_table(page_directory , virtual_address , &page_table_entry);
	if(!exist){
		page_table_entry[PTX(virtual_address)] = 0;
		tlb_invalidate((void*)NULL , (void *)virtual_address);
	}
	else{
		panic("\nDEVELOPER PANIC : this address is invalid");
	}
}

/***********************************************************************************************/

/*[2.2] ADDRESS CONVERTION*/
inline int virtual_to_physical(uint32* page_directory, uint32 virtual_address)
{
	//TODO: [PROJECT MS2] [PAGING HELPERS] virtual_to_physical

	uint32 *page_table_entry;
	uint32 exist = get_page_table(page_directory , virtual_address , &page_table_entry);
	if(!exist){
		uint32 tmp = page_table_entry[PTX(virtual_address)] >> 12;
		uint32 pa = tmp << 12;
		return pa;
	}
	else
		return -1;
}

/***********************************************************************************************/

/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/

///============================================================================================
/// Dealing with page directory entry flags

inline uint32 pd_is_table_used(uint32* page_directory, uint32 virtual_address)
{
	return ( (page_directory[PDX(virtual_address)] & PERM_USED) == PERM_USED ? 1 : 0);
}

inline void pd_set_table_unused(uint32* page_directory, uint32 virtual_address)
{
	page_directory[PDX(virtual_address)] &= (~PERM_USED);
	tlb_invalidate((void *)NULL, (void *)virtual_address);
}

inline void pd_clear_page_dir_entry(uint32* page_directory, uint32 virtual_address)
{
	page_directory[PDX(virtual_address)] = 0 ;
	tlbflush();
}
