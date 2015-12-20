/*
 * OW.h
 *
 * Created: 2014-02-23 13:33:34
 *  Author: Maciek
 */ 

#ifndef OW_H_
#define OW_H_

#include "config.h"

//OW_BIT oraz OW_PORT zdefiniowane w pliku config.h
#define OW_IN   OW_PIN
#define OW_OUT  OW_PORT 

#define OW_SEARCH_ROM		0xF0
#define OW_READ_ROM			0x33
#define OW_MATCH_ROM		0x55
#define OW_SKIP_ROM			0xCC
#define OW_ALARM_SEARCH		0xEC
#define OW_CONVERT_T		0x44
#define OW_READ_SCRATCHPAD	0xBE
#define OW_WRITE_SCRATCHPAD	0x4E
#define OW_COPY_SCRATCHPAD	0x48
#define OW_RECALL_E2		0xB8
#define OW_READ_POWER_SUP	0xB4

char OW_get_in();
void OW_out_low();
void OW_out_high();
void OW_dir_in();
//void OW_dir_out();
	
void OW_write (char bit);
unsigned char OW_read (void);

unsigned char  OW_reset_pulse(void);
void OW_write_byte (uint8_t message);
unsigned char OW_read_byte (void);


#endif /* OW_H_ */