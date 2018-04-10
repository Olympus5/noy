/*! \file pagefaultmanager.cc
Routines for the page fault managerPage Fault Manager
*/
//
//  Copyright (c) 1999-2000 INSA de Rennes.
//  All rights reserved.
//  See copyright_insa.h for copyright notice and limitation
//  of liability and disclaimer of warranty provisions.
//

#include "kernel/thread.h"
#include "vm/swapManager.h"
#include "vm/physMem.h"
#include "vm/pagefaultmanager.h"

PageFaultManager::PageFaultManager() {
}

// PageFaultManager::~PageFaultManager()
/*! Nothing for now
*/
PageFaultManager::~PageFaultManager() {
}

// ExceptionType PageFault(uint32_t virtualPage)
/*!
//	This method is called by the Memory Management Unit when there is a
//      page fault. This method loads the page from :
//      - read-only sections (text,rodata) $\Rightarrow$ executive
//        file
//      - read/write sections (data,...) $\Rightarrow$ executive
//        file (1st time only), or swap file
//      - anonymous mappings (stack/bss) $\Rightarrow$ new
//        page from the MemoryManager (1st time only), or swap file
//
//	\param virtualPage the virtual page subject to the page fault
//	  (supposed to be between 0 and the
//        size of the address space, and supposed to correspond to a
//        page mapped to something [code/data/bss/...])
//	\return the exception (generally the NO_EXCEPTION constant)
*/
ExceptionType PageFaultManager::PageFault(uint32_t virtualPage)
{
  Process* proc = g_current_thread->GetProcessOwner();
  AddrSpace* as = proc->addrspace;
  TranslationTable* tt = as->translationTable;
  char page[g_cfg->PageSize];

  //printf("VPN: %d, read: %d, write: %d\n", virtualPage, tt->getBitReadAllowed(virtualPage), tt->getBitWriteAllowed(virtualPage));

  // Etape 1 ---------------------------------------------------------------------------
  int page_number = g_physical_mem_manager->AddPhysicalToVirtualMapping(as, virtualPage);

  if(g_cfg->NumPhysPages <= page_number) {
    return BUSERROR_EXCEPTION;
  }

  if(tt->getBitSwap(virtualPage) == 1) {
    //TPV dans swap
    DEBUG('v', "Value of swap bit: %d\n", tt->getBitSwap(virtualPage));
    // Etape 2 ---------------------------------------------------------------------------
    if(tt->getAddrDisk(virtualPage) == -1) {
      // attendre car il y a déjà un voleur de page.
      g_current_thread->Yield();
      // Surement un P
    }

    g_swap_manager->GetPageSwap(tt->getAddrDisk(virtualPage), page);
  } else {
    DEBUG('v', "Value of swap bit: %d\n", tt->getBitSwap(virtualPage));
    if(tt->getAddrDisk(virtualPage) == -1) {
      // Page anonyme
      DEBUG('v', "Value of disk bits: %d\n", tt->getAddrDisk(virtualPage));
      // Etape 2 ---------------------------------------------------------------------------
      memset(page, 0, g_cfg->PageSize);
    } else {
      // TPV dans l'executable
      DEBUG('v', "Value of disk bits: %d\n", tt->getAddrDisk(virtualPage));
      // Etape 2 ---------------------------------------------------------------------------
      int offset = tt->getAddrDisk(virtualPage);
      proc->exec_file->ReadAt(page, g_cfg->PageSize, offset);
    }
  }

  // Etape 3 ---------------------------------------------------------------------------
  tt->setPhysicalPage(virtualPage, page_number);
  tt->setBitValid(virtualPage);
  memcpy(&(g_machine->mainMemory[tt->getPhysicalPage(virtualPage)*g_cfg->PageSize]), page, g_cfg->PageSize);


  //printf("VPN: %d\n", virtualPage);

  DEBUG('v', "Value of valid bit: %d\n", tt->getBitValid(virtualPage));

  return (NO_EXCEPTION);
}
