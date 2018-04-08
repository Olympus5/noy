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
  // Etape 1 ---------------------------------------------------------------------------
  int page_number = g_physical_mem_manager->AddPhysicalToVirtualMapping(as, virtualPage);

  if(g_cfg->NumPhysPages <= page_number) {
    return BUSERROR_EXCEPTION;
  }

  if(tt->getBitSwap(virtualPage) == 1) {
    //TPV dans swap
    // Etape 2 ---------------------------------------------------------------------------
    if(tt->getAddrDisk(virtualPage) == -1) {
      // attendre car il y a déjà un voleur de page.
      // g_current_thread->;
    }

    g_swap_manager->GetPageSwap(tt->getAddrDisk(virtualPage), page);
  } else {
    if(tt->getAddrDisk(virtualPage) == -1) {
      // Page anonyme
      // Etape 2 ---------------------------------------------------------------------------
      memset(page, 0, g_cfg->PageSize);
    } else {
      // TPV dans l'executable
      // Etape 2 ---------------------------------------------------------------------------
      int offset = tt->getAddrDisk(virtualPage);
      proc->exec_file->ReadAt(page, g_cfg->PageSize, offset);
    }
  }

  // Etape 2 ---------------------------------------------------------------------------
  memcpy(g_machine->mainMemory, page, g_cfg->PageSize);

  // Etape 3 ---------------------------------------------------------------------------
  tt->setPhysicalPage(virtualPage, page_number);
  tt->setBitValid(1);

  return (NO_EXCEPTION);
}
