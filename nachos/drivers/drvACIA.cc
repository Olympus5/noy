/* \file drvACIA.cc
   \brief Routines of the ACIA device driver
//
//      The ACIA is an asynchronous device (requests return
//      immediately, and an interrupt happens later on).
//      This is a layer on top of the ACIA.
//      Two working modes are to be implemented in assignment 2:
//      a Busy Waiting mode and an Interrupt mode. The Busy Waiting
//      mode implements a synchronous IO whereas IOs are asynchronous
//      IOs are implemented in the Interrupt mode (see the Nachos
//      roadmap for further details).
//
//  Copyright (c) 1999-2000 INSA de Rennes.
//  All rights reserved.
//  See copyright_insa.h for copyright notice and limitation
//  of liability and disclaimer of warranty provisions.
//
*/

/* Includes */

#include "kernel/system.h"         // for the ACIA object
#include "kernel/synch.h"
#include "machine/ACIA.h"
#include "drivers/drvACIA.h"
#include <string.h>

//-------------------------------------------------------------------------
// DriverACIA::DriverACIA()
/*! Constructor.
  Initialize the ACIA driver. In the ACIA Interrupt mode,
  initialize the reception index and semaphores and allow
  reception and emission interrupts.
  In the ACIA Busy Waiting mode, simply inittialize the ACIA
  working mode and create the semaphore.
  */
//-------------------------------------------------------------------------

DriverACIA::DriverACIA()
{
    g_machine->acia->SetWorkingMode(BUSY_WAITING);
    this->send_sema = new Semaphore((char*)"send_sema", 0);
    this->receive_sema = new Semaphore((char*)"receive_sema", 0);
}

//-------------------------------------------------------------------------
// DriverACIA::TtySend(char* buff)
/*! Routine to send a message through the ACIA (Busy Waiting or Interrupt mode)
  */
//-------------------------------------------------------------------------

int DriverACIA::TtySend(char* buff)
{
  unsigned int i = 0;

  for(i = 0; i < strlen(buff); i++) {
      // Attente active
      while(g_machine->acia->GetOutputStateReg() == FULL) {};
      this->send_buffer[i] = buff[i];
      g_machine->acia->PutChar(this->send_buffer[i]);
      DEBUG('d', "-Valeur de l'ACIA (it %d) = %c\n", i, buff[i]);
  }

  while(g_machine->acia->GetOutputStateReg() == FULL) {};
  this->send_buffer[i] = '\0';
  g_machine->acia->PutChar('\0');

  return strlen(buff);
}

//-------------------------------------------------------------------------
// DriverACIA::TtyReceive(char* buff,int length)
/*! Routine to reveive a message through the ACIA
//  (Busy Waiting and Interrupt mode).
  */
//-------------------------------------------------------------------------

int DriverACIA::TtyReceive(char* buff,int lg)
{
  int i = 0, length = 0;
  bool end = false;

  for(i = 0; i < lg && !end; i++, length++) {
    while(g_machine->acia->GetInputStateReg() == EMPTY) {};

    this->receive_buffer[i] = g_machine->acia->GetChar();
    buff[i] = this->receive_buffer[i];
    DEBUG('d', "-Valeur du buff[%d] = %c\n", i, buff[i]);

    if(buff[i] == 0) {
        end = true;
    }
  }

  printf("Coucou\n");

  return length;
}


//-------------------------------------------------------------------------
// DriverACIA::InterruptSend()
/*! Emission interrupt handler.
  Used in the ACIA Interrupt mode only.
  Detects when it's the end of the message (if so, releases the send_sema semaphore), else sends the next character according to index ind_send.
  */
//-------------------------------------------------------------------------

void DriverACIA::InterruptSend()
{
  printf("**** Warning: send interrupt handler not implemented yet\n");
  exit(-1);
}

//-------------------------------------------------------------------------
// DriverACIA::Interrupt_receive()
/*! Reception interrupt handler.
  Used in the ACIA Interrupt mode only. Reveices a character through the ACIA.
  Releases the receive_sema semaphore and disables reception
  interrupts when the last character of the message is received
  (character '\0').
  */
//-------------------------------------------------------------------------

void DriverACIA::InterruptReceive()
{
  printf("**** Warning: receive interrupt handler not implemented yet\n");
  exit(-1);
}
