/*****************************************************************************
*
* CAN Test APP
* - header file of at90can.c
*
* Compiler : avr-gcc 4.3.0 / avr-libc 1.6.2 / AVR Studio 4.14
* size     : 2,02KB
* by       : Thomas Fuchs, Wolfsburg, Germany
*            linux@cc-robotics.de
*
* License  : Copyright (c) 2009 Thomas Fuchs
*
* Tested with AT90CAN128
****************************************************************************/
//  CPU speed
#define CAN_DDR DDRD
#define CAN_RX 6
#define CAN_TX 5

//  message objects
#define NR_MOBS 15		// number of message objects
#define NOMOB   0xff

#define	_10KBPS		0	// untested
#define	_20KBPS		1	// untested
#define	_50KBPS		2	// untested
#define	_100KBPS    3
#define	_125KBPS    4
#define	_250KBPS    5
#define	_500KBPS    6
#define	_1MBPS		7	// untested

//  global structs and variables
typedef struct
	{	
		uint8_t ide;            //  0 STANDARD   1 EXTENDED
		uint16_t id;			//  MSG ID                   
		uint16_t ext;           //  
		uint8_t length;   		//  DLC length
		uint8_t data[8];  		//  Data field 8 Byte
} CAN_messageType;           //  CAN 2.0 A message 11 Bit

CAN_messageType can_rx_buffer[CAN_MAX_BUF_SIZE];
void can_tx (CAN_messageType msg);
Bool initCAN( uint8_t bitrate );
void configRX();

U8 can_rx_index = 0x00;