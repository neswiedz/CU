/*******************************************************************
* CAN Test-APP
* - CAN functions definded for AT90CAN
*
* Compiler : avr-gcc 4.3.0 / avr-libc 1.6.2 / AVR Studio 4.14
* size     : 6,18KB
* by       : Thomas Fuchs, Wolfsburg, Germany
*            linux@cc-robotics.de
*
* License  : Copyright (c) 2009 Thomas Fuchs
*
* Tested with AT90CAN128
*******************************************************************/
#include <stdbool.h>
#include <avr/pgmspace.h>
#include "utils.h"
#include "config.h"
#include "can.h"

/* to enhance the readability:
 * - MOb = Message Object
 */

// returns the mob which has the message
// suchen des MOb in dem die Nachricht ist
uint8_t getmob( uint32_t bitmask) 
{
  uint8_t mob;

  if( bitmask==0) return NOMOB;

  for( mob=0; (bitmask & 0x01)==0; bitmask >>= 1, ++mob);

  //  security check
  if ( mob > 14 ) return NOMOB;
    else return mob;
}     

// initializes CAN interface
// Funktion zum Initialisieren
Bool initCAN( uint8_t bitrate )
{
  CAN_DDR |= _BV(CAN_TX);
  CAN_DDR &= ~_BV(CAN_RX);

  //  security check
  if (bitrate >= 8) return false;
  
  //  number of the MOb
  uint8_t mob;

  CANGCON |= (1<<SWRES);		// reset the CAN controller
  
  CANGCON = 0x00;				// reset the general control register (CONFIG mode)

  // reset the interrupt registers
  CANSIT2 = 0x00;
  CANSIT1 = 0x00;
  CANGIT  = 0x00;
  CANGIE  = 0x00;
  CANEN1  = 0x00;
  CANEN2  = 0x00;
  CANIE1  = 0x00;
  CANIE2  = 0x00;

  // set auto inc to Data Buffer Index (DBI)
  CANPAGE &= ~(1<<AINC);

  // set all MObs to 0
  for (mob = 0; mob < NR_MOBS; mob++)
  {
    CANPAGE  = (mob << 4);
	CANIDT1 = 0x00;  //  reset ID-Tag
	CANIDT2 = 0x00;
	CANIDT3 = 0x00;
	CANIDT4 = 0x00;

	CANIDM1 = 0x00;  //  reset ID-Mask
	CANIDM2 = 0x00;
	CANIDM3 = 0x00;
	CANIDM4 = 0x00;

    CANSTMOB = 0x00;  //  reset MOb status
    CANCDMOB = 0x00;  //  disable MOb
  }

  // HAPCAN
  CANBT1 = 0x0E;
  CANBT2 = 0x2C;
  CANBT3 = 0x37;

  // 125kbits
//  CANBT1 = 0x06;
//  CANBT2 = 0x0C;
//  CANBT3 = 0x37;
	
  // set config to MObs 1 and 2
  // MOb 1 soll empfangen
  // MOb 2 soll senden
  for (mob = 1; mob < 3; mob++)
  {
    CANPAGE  = (mob << 4);
	CANSTMOB = 0x00;  //  reset MOb status
	switch (mob)
	{
      case 1:
	    CANCDMOB = 0x80;  //  RX
	  	CANIDT1  = 0x00;  //  set ID-Tag
	    CANIDT2  = 0x00;

	    CANIDM1  = 0x00;  //  set ID-Mask, receive all
	    CANIDM2  = 0x00;
        break;

	  case 2:
	  	CANIDT1  = 0x00;  //  set ID-Tag
	    CANIDT2  = 0x00;
        break;

	  default:
	    return false;
	}
  }

  //  Enable all required interrupts
  CANGIE  = 0xB0;  // ENIT, ENRX, ENTX
  CANIE2  = 0x06;  // MOb 1, MOb 2 aktivieren
    
  //  switch CAN on   
  CANGCON |= (1<<ENASTB);
  	
  //  wait for EnableFlag
  while (!(CANGSTA & (1<<ENFG))); 
  
  return true;
}

// sends CANmsg
void can_tx (CAN_messageType msg)
{
  uint8_t i, mob;	
  uint32_t identifier;

  //  MOb Sender is Nr 2
  mob=2;

  //  enable MOb number mob, auto increment index, start with index = 0
  CANPAGE = (mob<<4);
	

  //  set IDE bit, length = 8
  //  set ID
  if (msg.ide == 0) { 
     // CAN 2.0A
	 CANCDMOB = (0<<IDE) | (msg.length);	
     CANIDT2 = (unsigned char) ((msg.id<<5)&0xE0);
     CANIDT1 = (unsigned char) (msg.id>>3);
  } else {
	 // CAN 2.0B
	 CANCDMOB = (1<<IDE) | (msg.length);

     identifier = ((uint32_t)msg.id) << 16 | msg.ext;
/*
     CANIDT1 = ((*((U8 *)(&(identifier))+3))<<3)+((*((U8 *)(&(identifier))+2))>>5);
     CANIDT2 = ((*((U8 *)(&(identifier))+2))<<3)+((*((U8 *)(&(identifier))+1))>>5);
	 CANIDT3 = ((*((U8 *)(&(identifier))+1))<<3)+((* (U8 *)(&(identifier))   )>>5);
	 CANIDT4 = (* (U8 *)(&(identifier))   )<<3;
*/
     CANIDT1 = (*((U8 *)(&(identifier))+3));
     CANIDT2 = ((*((U8 *)(&(identifier))+2)) & 0xF0)+((*(((U8 *)(&(identifier))+2)) & 0x01)<<3);
     CANIDT3 = ((*((U8 *)(&(identifier))+1))<<3)+((* (U8 *)(&(identifier))   )>>5);
     CANIDT4 = (* (U8 *)(&(identifier))   )<<3;

  }
  
  //  write data to MOb
  for (i=0; i<8; i++)							
  CANMSG = msg.data[i];
	
  //  wait for txbsy
  while (CANGSTA & (1<<TXBSY));
	
  //  send message
  CANCDMOB |= (1<<CONMOB0);
}


// config the receive mob
void configRX()
{	
  uint8_t mob;

  // receive in this MOb
  mob = 1;

  CANPAGE = (mob<<4);  // rotate 4 bits left	

  // define the IDTags to get only one ID (Filter)
  CANIDT4 = 0;		
  CANIDT3 = 0;	
  CANIDT2 = 0;
  CANIDT1 = 0;

  // define the IDMasks to activate the Tags (Maske)
  // 1 means filter, 0 means uncared

  CANIDM1  = 0x00;  //filter off, don't care
  CANIDM2  = 0x00;

  //  activate receiving messages with 11 bit id´s
  CANCDMOB = (1<<CONMOB1);
}

// interrupt service for CAN
SIGNAL(SIG_CAN_INTERRUPT1)
{
  // CAN_messageType recMsg; this is a global variable
  uint8_t save_canpage = CANPAGE;		//  actual CANPAGE
  uint8_t mob;
  uint8_t i;
  uint8_t readmodwrite;
  uint32_t identifier;
  CAN_messageType *recMsg;
  
  // check in which MOb the INT was initiated
  mob = getmob(CANSIT2 | (CANSIT1 << 8));
	if( mob == NOMOB )   return;

  // select concerned MOb page
  CANPAGE = (mob << 4);	

  // Senden war erfolgreich
  if (CANSTMOB & (1<<TXOK))
  {
    readmodwrite = CANSTMOB;	 	
    readmodwrite &= ~(1<<TXOK);	//  reset INT reason
	CANSTMOB = readmodwrite;			
  }
  // Nachricht erfolgreich empfangen
  else if (CANSTMOB & (1<<RXOK))
  {
	if (can_rx_index < CAN_MAX_BUF_SIZE-1) {
		can_rx_index++;
	} else {
		can_rx_index = 0;
	}		
		  
    recMsg = &can_rx_buffer[can_rx_index];
	
	if (CANCDMOB & (1<<IDE))
	{
/*
		*((U8 *)(&(identifier))+3) =  CANIDT1>>3              ; 
		*((U8 *)(&(identifier))+2) = (CANIDT2>>3)+(CANIDT1<<5); 
		*((U8 *)(&(identifier))+1) = (CANIDT3>>3)+(CANIDT2<<5); 
		*((U8 *)(&(identifier))  ) = (CANIDT4>>3)+(CANIDT3<<5);
*/
		*((U8 *)(&(identifier))+3) =  CANIDT1              ;
		*((U8 *)(&(identifier))+2) = (CANIDT2 & 0xF0)+((CANIDT2 & 0x08)>>3);
		*((U8 *)(&(identifier))+1) = (CANIDT3>>3)+(CANIDT2<<5);
		*((U8 *)(&(identifier))  ) = (CANIDT4>>3)+(CANIDT3<<5);
				
		recMsg->id = HIGH_WORD(identifier);
		recMsg->ext = LOW_WORD(identifier);
	} else {
  		// get id of selected MOb
  		recMsg->id = 0;
  		recMsg->id |= ((uint16_t) CANIDT1<<8);
  		recMsg->id |= (CANIDT2&0xE0);
  		recMsg->id >>= 5;
	}
	

    // get length of selected MOb
    recMsg->length = (CANCDMOB&0x0F);

    // clear memory
	for (i=0; i<8; i++)	
	{	
	  recMsg->data[i] = 0;
	}

    // get data of selected MOb
	for (i=0; i<recMsg->length; i++)	
	{	
	  recMsg->data[i] = CANMSG;
	}

    readmodwrite = CANSTMOB;
    readmodwrite &= ~(1<<RXOK);  //  reset interrupt 	
    CANSTMOB = readmodwrite;
	CANCDMOB = (1<<CONMOB1);  //  stay tuned!
  }

  CANPAGE = save_canpage;  //  restore CANPAGE
}
