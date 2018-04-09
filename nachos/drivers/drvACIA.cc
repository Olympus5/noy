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
		/*
		 * Si le driver est configuré en Busy Waiting alors on initialise le mode de fonctionnement à la construction.
		 * Pour les interruptions cela sera fait, respectivement, pour SEND_INTERRUPT dans la fonction d'envoie et pour REC_INTERRUPT dans la fonction de lecture
		 * Cependant on initialise les variables membres propre au fonctionnement par interruptions
		 */
		if(g_cfg->ACIA == ACIA_BUSY_WAITING) {
			g_machine->acia->SetWorkingMode(BUSY_WAITING);
		} else {
			this->send_sema = new Semaphore((char*)"send_sema", 1);
			this->receive_sema = new Semaphore((char*)"receive_sema", 0);
		}
}

//-------------------------------------------------------------------------
// DriverACIA::TtySend(char* buff)
/*! Routine to send a message through the ACIA (Busy Waiting or Interrupt mode)
  */
//-------------------------------------------------------------------------

int DriverACIA::TtySend(char* buff)
{

	if(g_cfg->ACIA == ACIA_BUSY_WAITING) {// Busy waiting mode
		unsigned int i = 0;

	  for(i = 0; i < strlen(buff); i++) {
	      // Attente active
	      while(g_machine->acia->GetOutputStateReg() == FULL) {};
				// On envoie le caractère sur le bus
	      g_machine->acia->PutChar(buff[i]);
	      DEBUG('d', "-Valeur de l'ACIA (it %d) = %c\n", i, buff[i]);
	  }

		// On oublie pas de placer le caractère de fin de chaine '\0' (pour que le receiver puisse savoir quand l'envoie est fini)
	  while(g_machine->acia->GetOutputStateReg() == FULL) {};
	  g_machine->acia->PutChar('\0');
	} else { // Interrupt mode
		this->send_sema->P();
		this->ind_send = 1;
		int i;

		// On remplie le tampon d'émission, pour que la routine d'it puisse accéder aux caractères à envoyer
		for(i = 0; i < BUFFER_SIZE-1 && buff[i] != 0; i++) {
			this->send_buffer[i] = buff[i];
		}

		this->send_buffer[i] = '\0';

		g_machine->acia->SetWorkingMode(SEND_INTERRUPT);
		// On envoie le premier caractère pour lancer la routine d'IT
		g_machine->acia->PutChar(buff[0]);

		DEBUG('d', "state of output ACIA: %d and allowed mode: %d\n", g_machine->acia->GetOutputStateReg() == EMPTY, g_machine->acia->GetWorkingMode());
	}

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
	int length = 0;

	if(g_cfg->ACIA == ACIA_BUSY_WAITING) { // Busy waiting mode
		int i = 0;
	  bool end = false;

	  for(i = 0; i < lg && !end; i++) {
			//Attente active
	    while(g_machine->acia->GetInputStateReg() == EMPTY) {};
			// On reçois le caractère sur le bus
	    buff[i] = g_machine->acia->GetChar();
	    DEBUG('d', "-Valeur du buff[%d] = %c\n", i, buff[i]);

	    if(buff[i] == 0) {// Une fois qu'on tombe sur le caractère de fin on stop la boucle de lecture du registre de donnée du périphérique
	        end = true;
	    } else {
			length++;
		}
	  }
	} else { // Interrupt mode
		this->ind_rec = 0;
		g_machine->acia->SetWorkingMode(REC_INTERRUPT);
		this->receive_sema->P();

		for(int i = 0; i < BUFFER_SIZE && this->receive_buffer[i] != 0; i++) {
			buff[i] = this->receive_buffer[i];
			length++;
		}
	}

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
	//IntStatus int_status = g_machine->interrupt->SetStatus(INTERRUPTS_OFF);

	if(this->send_buffer[this->ind_send] != 0) {
		//Car debug d affiche trop de message !!!
		//printf("Char: %c\n", this->send_buffer[this->ind_send]);
		g_machine->acia->PutChar(this->send_buffer[this->ind_send]);
		this->ind_send++;
	} else {
		g_machine->acia->PutChar('\0');
		// NE PAS OUBLIER DE COUPER SEND INTERRUPT (sinon vide le buffer en entrée !!!)
		g_machine->acia->SetWorkingMode(~SEND_INTERRUPT);
		this->send_sema->V();
	}

	//g_machine->interrupt->SetStatus(int_status);
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
	//IntStatus int_status = g_machine->interrupt->SetStatus(INTERRUPTS_OFF);
	DEBUG('d', "Interrupt receive\n");

	// Reception des caractères
	this->receive_buffer[this->ind_rec] = g_machine->acia->GetChar();
	//printf("J'ai reçu: %c\n", this->receive_buffer[this->ind_rec]);
	//printf("Working Mode: %d\n", g_machine->acia->GetWorkingMode());
	if(this->receive_buffer[this->ind_rec] != 0) {
		this->ind_rec++;
	} else {
		g_machine->acia->SetWorkingMode(~REC_INTERRUPT);
		this->receive_sema->V();
	}

	//g_machine->interrupt->SetStatus(int_status);
}
