#ifndef _1wire_h_
#define _1wire_h_

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"
//#include "delay.h"

/*******************************************/
/* Hardware connection                     */
/*******************************************/

/* Define OW_ONE_BUS if only one 1-Wire-Bus is used
   in the application -> shorter code.
   If not defined make sure to call OW_set_bus() before using 
   a bus. Runtime bus-select increases code size by around 300 
   bytes so use OW_ONE_BUS if possible */
//#define OW_ONE_BUS

#ifdef OW_PORT

#define OW_PIN  OW_BIT
#define OW_IN   PIN(OW_PORT)
#define OW_OUT  OW_PORT
#define OW_DDR  DDR(OW_PORT)

//#define OW_CONF_DELAYOFFSET 0

#else 
#ifdef _DEBUG-MS
#if F_CPU<1843200
 #warning | experimental multi-bus-mode is not tested for 
 #warning | frequencies below 1,84MHz - use OW_ONE_WIRE or
 #warning | faster clock-source (i.e. internal 2MHz R/C-Osc.)
#endif
#endif // _DEBUG-MS

//#define OW_CONF_CYCLESPERACCESS 13
//#define OW_CONF_DELAYOFFSET ( (uint16_t)( ((OW_CONF_CYCLESPERACCESS)*1000000L) / F_CPU  ) )
#endif

#ifdef OW_PORT

#define OW_GET_IN()   ( OW_IN & (1<<OW_PIN))
#define OW_OUT_LOW()  ( OW_OUT &= (~(1 << OW_PIN)) )
#define OW_OUT_HIGH() ( OW_OUT |= (1 << OW_PIN) )
#define OW_DIR_IN()   ( OW_DDR &= (~(1 << OW_PIN )) )
#define OW_DIR_OUT()  ( OW_DDR |= (1 << OW_PIN) )

#else

/* set bus-config with OW_set_bus() */
uint8_t OW_PIN_MASK; 
volatile uint8_t* OW_IN;
volatile uint8_t* OW_OUT;
volatile uint8_t* OW_DDR;

#define OW_GET_IN()   ( *OW_IN & OW_PIN_MASK )
#define OW_OUT_LOW()  ( *OW_OUT &= (uint8_t) ~OW_PIN_MASK )
#define OW_OUT_HIGH() ( *OW_OUT |= (uint8_t)  OW_PIN_MASK )
#define OW_DIR_IN()   ( *OW_DDR &= (uint8_t) ~OW_PIN_MASK )
#define OW_DIR_OUT()  ( *OW_DDR |= (uint8_t)  OW_PIN_MASK )

#endif

/*******************************************/

// #define OW_SHORT_CIRCUIT  0x01

#define OW_MATCH_ROM	0x55
#define OW_SKIP_ROM	    0xCC
#define	OW_SEARCH_ROM	0xF0

#define	OW_SEARCH_FIRST	0xFF		// start new search
#define	OW_PRESENCE_ERR	0xFF
#define	OW_DATA_ERR	    0xFE
#define OW_LAST_DEVICE	0x00		// last device found
//			0x01 ... 0x40: continue searching

// rom-code size including CRC
#define OW_ROMCODE_SIZE 8

extern uint8_t OW_reset(void);

extern uint8_t OW_bit_io( uint8_t b );
extern uint8_t OW_byte_wr( uint8_t b );
extern uint8_t OW_byte_rd( void );

extern uint8_t OW_rom_search( uint8_t diff, uint8_t *id );

extern void OW_command( uint8_t command, uint8_t *id );

extern void OW_parasite_enable(void);
extern void OW_parasite_disable(void);
extern uint8_t OW_input_pin_state(void);

#ifndef OW_PORT
void OW_set_bus(volatile uint8_t* port, uint8_t bit);
#endif

uint8_t	crc8 (uint8_t* data_in, uint16_t number_of_bytes_to_read);

#endif

