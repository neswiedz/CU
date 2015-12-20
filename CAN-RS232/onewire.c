#include "onewire.h"

#define CRC8INIT	0x00
#define CRC8POLY	0x18              //0X18 = X^8+X^5+X^4+X^0

/* Timing issue when using runtime-bus-selection (!OW_ONE_BUS):
   The master should sample at the end of the 15-slot after initiating
   the read-time-slot. The variable bus-settings need more
   cycles than the constant ones so the delays had to be shortened 
   to achive a 15uS overall delay 
   Setting/clearing a bit in I/O Register needs 1 cyle in OW_ONE_BUS
   but around 14 cyles in configureable bus (us-Delay is 4 cyles per uS) */
uint8_t OW_bit_io ( uint8_t b )
{
  uint8_t sreg;

  sreg=SREG;
  cli();
	
  OW_DIR_OUT(); // drive bus low
	
  delayus(1); // Recovery-Time wuffwuff was 1
  if ( b ) 
    OW_DIR_IN(); // if bit is 1 set bus high (by ext. pull-up)
	
  // wuffwuff delay was 15uS-1 see comment above
  delayus(15-1-OW_CONF_DELAYOFFSET);
	
  if( OW_GET_IN() == 0 ) 
    b = 0;  // sample at end of read-timeslot
	
  delayus(60-15);
  OW_DIR_IN();

  SREG=sreg; // sei();
	
  return b;
}

#ifndef OW_PORT
void OW_set_bus(volatile uint8_t* port, uint8_t bit)
{
  OW_OUT=port;
  OW_DDR=port-1;
  OW_IN=port-2;
  OW_PIN_MASK=_BV(bit);
  OW_reset();
}
#endif

uint8_t OW_rom_search( uint8_t diff, uint8_t *id )
{
  uint8_t i, j, next_diff;
  uint8_t b;
	
  if ( OW_reset() ) 
    return OW_PRESENCE_ERR;			// error, no device found
  
  OW_byte_wr( OW_SEARCH_ROM );			// ROM search command
  
  next_diff = OW_LAST_DEVICE;			// unchanged on last device
  i = OW_ROMCODE_SIZE * 8;			// 8 bytes
  do 
  {
    j = 8;					// 8 bits
    do 
    {
      b = OW_bit_io( 1 );			// read bit
      if( OW_bit_io( 1 ) ) 
      {			// read complement bit
	if( b )					// 11
	  return OW_DATA_ERR;			// data error
      }
      else 
      {
	if( !b ) 
	{					// 00 = 2 devices
	  if( diff > i || ((*id & 1) && diff != i) ) 
	  {
	    b = 1;				// now 1
	    next_diff = i;			// next pass 0
	  }
	}
      }
      OW_bit_io( b );     			// write bit
      *id >>= 1;
      if( b ) *id |= 0x80;			// store bit
      i--;
    } 
    while( --j );
    id++;					// next byte
  } 
  while( i );
  return next_diff;				// to continue search
}

uint8_t OW_reset(void)
{
  uint8_t err;
  uint8_t sreg;

  OW_OUT_LOW(); // disable internal pull-up (maybe on from parasite)
  OW_DIR_OUT(); // pull OW-Pin low for 480us
	
  delayus(480);
	
  sreg=SREG;
  cli();
	
  // set Pin as input - wait for clients to pull low
  OW_DIR_IN(); // input
	
  delayus(66);
  err = OW_GET_IN();		// no presence detect
// nobody pulled to low, still high
	
  SREG=sreg; // sei()
	
// after a delay the clients should release the line
// and input-pin gets back to high due to pull-up-resistor
  delayus(480-66);
  if( OW_GET_IN() == 0 )		// short circuit
    err = 1;
	
  return err;
}

void OW_parasite_enable(void)
{
  OW_OUT_HIGH();
  OW_DIR_OUT();
}

void OW_parasite_disable(void)
{
  OW_OUT_LOW();
  OW_DIR_IN();
}

uint8_t OW_input_pin_state()
{
  return OW_GET_IN();
}

uint8_t	crc8 ( uint8_t *data_in, uint16_t number_of_bytes_to_read )
{
	uint8_t	 crc;
	uint16_t loop_count;
	uint8_t  bit_counter;
	uint8_t  data;
	uint8_t  feedback_bit;
	
	crc = CRC8INIT;

	for (loop_count = 0; loop_count != number_of_bytes_to_read; loop_count++)
	{
		data = data_in[loop_count];
		
		bit_counter = 8;
		do {
			feedback_bit = (crc ^ data) & 0x01;
	
			if ( feedback_bit == 0x01 ) {
				crc = crc ^ CRC8POLY;
			}
			crc = (crc >> 1) & 0x7F;
			if ( feedback_bit == 0x01 ) {
				crc = crc | 0x80;
			}
		
			data = data >> 1;
			bit_counter--;
		
		} while (bit_counter > 0);
	}
	
	return crc;
}

void OW_command( uint8_t command, uint8_t *id )
{
  uint8_t i;

  OW_reset();

  if( id ) 
  {
    OW_byte_wr( OW_MATCH_ROM );			// to a single device
    i = OW_ROMCODE_SIZE;
    do 
    {
      OW_byte_wr( *id );
      id++;
    } 
    while( --i );
  } 
  else 
  {
    OW_byte_wr( OW_SKIP_ROM );			// to all devices
  }

  OW_byte_wr( command );
}

uint8_t OW_byte_wr( uint8_t b )
{
	uint8_t i = 8, j;
	
	do {
		j = OW_bit_io( b & 1 );
		b >>= 1;
		if( j ) b |= 0x80;
	} while( --i );
	
	return b;
}

uint8_t OW_byte_rd( void )
{
  // read by sending 0xff (a dontcare?)
  return OW_byte_wr( 0xFF ); 
}




