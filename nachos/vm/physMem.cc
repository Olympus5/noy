//-----------------------------------------------------------------
/*! \file mem.cc
//  \brief Routines for the physical page management
*/
//
//  Copyright (c) 1999-2000 INSA de Rennes.
//  All rights reserved.
//  See copyright_insa.h for copyright notice and limitation
//  of liability and disclaimer of warranty provisions.
//-----------------------------------------------------------------

#include <unistd.h>
#include "vm/physMem.h"

//-----------------------------------------------------------------
// PhysicalMemManager::PhysicalMemManager
//
/*! Constructor. It simply clears all the page flags and inserts them in the
// free_page_list to indicate that the physical pages are free
*/
//-----------------------------------------------------------------
PhysicalMemManager::PhysicalMemManager() {

	long i;

	tpr = new struct tpr_c[g_cfg->NumPhysPages];

	for (i=0;i<g_cfg->NumPhysPages;i++) {
		tpr[i].free=true;
		tpr[i].locked=false;
		tpr[i].owner=NULL;
		free_page_list.Append((void*)i);
	}
	i_clock=-1;
}

PhysicalMemManager::~PhysicalMemManager() {
	// Empty free page list
	int64_t page;
	while (!free_page_list.IsEmpty()) page =  (int64_t)free_page_list.Remove();

	// Delete physical page table
	delete[] tpr;
}

//-----------------------------------------------------------------
// PhysicalMemManager::RemovePhysicalToVitualMapping
//
/*! This method releases an unused physical page by clearing the
//  corresponding bit in the page_flags bitmap structure, and adding
//  it in the free_page_list.
//
//  \param num_page is the number of the real page to free
*/
//-----------------------------------------------------------------
void PhysicalMemManager::RemovePhysicalToVirtualMapping(long num_page) {

	// Check that the page is not already free
	printf("free ? %d\n", tpr[num_page].free);
	printf("swap ? %d\n", tpr[num_page].owner->translationTable->getBitSwap(this->tpr[num_page].virtualPage));
	ASSERT(!tpr[num_page].free);

	// Update the physical page table entry
	tpr[num_page].free=true;
	tpr[num_page].locked=false;
	if (tpr[num_page].owner->translationTable!=NULL)
	tpr[num_page].owner->translationTable->clearBitValid(tpr[num_page].virtualPage);

	// Insert the page in the free list
	free_page_list.Prepend((void*)num_page);
}

//-----------------------------------------------------------------
// PhysicalMemManager::UnlockPage
//
/*! This method unlocks the page numPage, after
//  checking the page is in the locked state. Used
//  by the page fault manager to unlock at the
//  end of a page fault (the page cannot be evicted until
//  the page fault handler terminates).
//
//  \param num_page is the number of the real page to unlock
*/
//-----------------------------------------------------------------
void PhysicalMemManager::UnlockPage(long num_page) {
	ASSERT(num_page<g_cfg->NumPhysPages);
	ASSERT(tpr[num_page].locked==true);
	ASSERT(tpr[num_page].free==false);
	tpr[num_page].locked = false;
}

//-----------------------------------------------------------------
// PhysicalMemManager::ChangeOwner
//
/*! Change the owner of a page
//
//  \param owner is a pointer on new owner (Thread *)
//  \param numPage is the concerned page
*/
//-----------------------------------------------------------------
void PhysicalMemManager::ChangeOwner(long numPage, Thread* owner) {
	// Update statistics
	g_current_thread->GetProcessOwner()->stat->incrMemoryAccess();
	// Change the page owner
	tpr[numPage].owner = owner->GetProcessOwner()->addrspace;
}

//-----------------------------------------------------------------
// PhysicalMemManager::AddPhysicalToVirtualMapping
//
/*! This method returns a new physical page number. If there is no
//  page available, it evicts one page (page replacement algorithm).
//
//  NB: this method locks the newly allocated physical page such that
//      it is not stolen during the page fault resolution. Don't forget
//      to unlock it
//
//  \param owner address space (for backlink)
//  \param virtualPage is the number of virtualPage to link with physical page
//  \return A new physical page number.
*/
//-----------------------------------------------------------------
int PhysicalMemManager::AddPhysicalToVirtualMapping(AddrSpace* owner,int virtualPage)
{
	int page_number;

	page_number = FindFreePage();

	if(page_number == -1) {
		int swap_sector;
		char page[g_cfg->PageSize];

		DEBUG('v', "Page number: %d\n", page_number);
		page_number = EvictPage();

		this->tpr[page_number].owner->translationTable->clearBitValid(this->tpr[page_number].virtualPage);
		this->tpr[page_number].owner->translationTable->setBitSwap(this->tpr[page_number].virtualPage);
		// Pour dire que la page est dans un défaut de page et qu'elle va être mise en swap
		this->tpr[page_number].owner->translationTable->setAddrDisk(this->tpr[page_number].virtualPage, -1);

		//Mise de la page evict en swap
		memcpy(page, &(g_machine->mainMemory[this->tpr[page_number].owner->translationTable->getPhysicalPage(this->tpr[page_number].virtualPage)*g_cfg->PageSize]), g_cfg->PageSize);
		swap_sector = g_swap_manager->PutPageSwap(-1, page);
		this->tpr[page_number].owner->translationTable->setAddrDisk(this->tpr[page_number].virtualPage, swap_sector);

	}

	this->tpr[page_number].free = false;
	this->tpr[page_number].locked = true;
	this->tpr[page_number].virtualPage = virtualPage;
	this->tpr[page_number].owner = owner;

	return page_number;
}

//-----------------------------------------------------------------
// PhysicalMemManager::FindFreePage
//
/*! This method returns a new physical page number, if it finds one
//  free. If not, return -1. Does not run the clock algorithm.
//
//  \return A new free physical page number.
*/
//-----------------------------------------------------------------
int PhysicalMemManager::FindFreePage() {
	int64_t page;

	// Check that the free list is not empty
	if (free_page_list.IsEmpty())
	return -1;

	// Update statistics
	g_current_thread->GetProcessOwner()->stat->incrMemoryAccess();

	// Get a page from the free list
	page = (int64_t)free_page_list.Remove();

	// Check that the page is really free
	ASSERT(tpr[page].free);

	// Update the physical page table
	tpr[page].free = false;

	return page;
}

//-----------------------------------------------------------------
// PhysicalMemManager::EvictPage
//
/*! This method implements page replacement, using the well-known
//  clock algorithm.
//
//  \return A new free physical page number.
*/
//-----------------------------------------------------------------
int PhysicalMemManager::EvictPage() {
	bool evict = false;
	int page_number;
	int local_i_clock = this->i_clock;
	int nb_lock = 0;

	//TODO: Ne pas oublier d'implémenter les contraintes
	while(!evict) {
		//Incrément au début car
		local_i_clock = this->i_clock = (local_i_clock+1) % g_cfg->NumPhysPages;//On se déplace de manière circulaire donc faut pouvoir remettre à 0 une fois arrivé au bout

		if(!this->tpr[local_i_clock].owner->translationTable->getBitU(this->tpr[local_i_clock].virtualPage)) {
			page_number = local_i_clock;
			evict = true;
		} else {
			if(!this->tpr[local_i_clock].locked) {
				this->tpr[local_i_clock].owner->translationTable->clearBitU(this->tpr[local_i_clock].virtualPage);
			} else {
				nb_lock++;

				if(nb_lock == g_cfg->NumPhysPages) {
					// On remet à zéro tout simplement car sinon au prochain lock rencontré nb_lock = NumPhysPages + 1 et on rentre donc dans ce bloc
					nb_lock = 0;
					g_current_thread->Yield();
				}
			}
		}
	}

	this->i_clock = local_i_clock;

	return page_number;
}

//-----------------------------------------------------------------
// PhysicalMemManager::Print
//
/*! print the current status of the table of physical pages
//
//  \param rpage number of real page
*/
//-----------------------------------------------------------------

void PhysicalMemManager::Print(void) {
	int i;

	printf("Contents of TPR (%d pages)\n",g_cfg->NumPhysPages);
	for (i=0;i<g_cfg->NumPhysPages;i++) {
		printf("Page %d free=%d locked=%d virtpage=%d owner=%lx U=%d M=%d\n",
		i,
		tpr[i].free,
		tpr[i].locked,
		tpr[i].virtualPage,
		(long int)tpr[i].owner,
		(tpr[i].owner!=NULL) ? tpr[i].owner->translationTable->getBitU(tpr[i].virtualPage) : 0,
		(tpr[i].owner!=NULL) ? tpr[i].owner->translationTable->getBitM(tpr[i].virtualPage) : 0);
	}
}
