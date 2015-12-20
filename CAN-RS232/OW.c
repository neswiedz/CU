/*
 * CFile1.c
 *
 * Created: 2014-02-23 13:32:03
 *  Author: Jan
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include "OW.h"

#define OW_IN   OW_PIN
#define OW_OUT  OW_PORT

#define OW_PORT	PORTA
#define OW_BIT		6
#define OW_DDR	DDRA
#define OW_PIN	PINA


char OW_get_state(){
	return (OW_IN & (1<<OW_BIT));
}

void OW_out_low(){	
	OW_DDR |= (1 << OW_BIT);
	OW_OUT &= (~(1 << OW_BIT));
}

void OW_out_high(){
	OW_DDR |= (1 << OW_BIT);
	OW_DDR |= (1 << OW_BIT);
}

void OW_dir_in(){
	OW_DDR &= (~(1 << OW_BIT ));
	OW_OUT |= (1 << OW_BIT); //pull-up
} 

/*
void OW_dir_out(){
	OW_DDR |= (1 << OW_BIT);
}
*/

void OW_write (char bit){
	OW_out_low();
	_delay_us(5);//delay(5);
	
	if(bit==1){ OW_out_high();	}
	_delay_us(80);//delay(80);
	OW_out_high();
}

unsigned char OW_read (void){
	OW_out_low();
	_delay_us(2);//delay(2);
	OW_out_high();
	_delay_us(14); //delay(14);
	return OW_get_state();
}

unsigned char OW_reset_pulse(void){
	unsigned char status;
	OW_out_low();
	_delay_us(500);//delay(500);
	OW_out_high();
	_delay_us(30);//delay(30);
	status = OW_get_state();
	_delay_us(470);//delay(470);
	status = OW_get_state();
	return status;
}

void OW_write_byte (uint8_t message){
	unsigned char i;
	unsigned char temp;
	
	for(i=0; i<8; i++){
		temp = message>>i;
		temp &= 0x01;
		OW_write(temp);
	}
	_delay_us(100);//delay(100);	//czy potrzebne?
}

unsigned char OW_read_byte (void){
	unsigned char i;
	unsigned char message = 0;
	
	for (i=0; i<8; i++){
		if(OW_read()) message |= 0x01<<i;
		_delay_us(15);//delay(15);	
	}
	return message;
}  