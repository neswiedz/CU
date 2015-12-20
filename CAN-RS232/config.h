/*
 * config.h
 *
 * Created: 2012-04-29 08:19:05
 *  Author: Sotarsoft
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

#include "compiler.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "at90can_drv.h"

// -------------- MCU LIB CONFIGURATION
#define FOSC           F_CPU/1000 //16000        // 8 MHz External cristal
//#define F_CPU          (FOSC*1000) // Need for AVR GCC

#define USE_TIMER16    TIMER16_1 
#define USE_UART       UART_0
#define UART_BAUDRATE  UBRR_9K6 //0x00CF	//VARIABLE_UART_BAUDRATE

// -------------- ONE-WIRE LIB CONFIGURATION
#define OW_PORT	PORTA 
#define OW_BIT		6
#define OW_DDR	DDRA
#define OW_PIN	PINA

// -------------- RTC LIB CONFIGURATION
#define USE_TIMER8     TIMER8_2  // as RTC
#define RTC_TIMER      2  // RTC Timer number
//#define RTC_CLOCK      32  // RTC frequency in KHz
#define RTC_CLOCK     0  // SYSTEM clock (FOSC defined in "config.h") is used as RTC clock

#define MESSAGE_BUF_SIZE      20
#define MESSAGE_DATA_SIZE     8

// -------------- UART PROTOCOL DEFINTION
#define UART_MAX_BUF_SIZE     200

#define MESSAGE_START         0x1B
#define MESSAGE_TOKEN         0x0E

// -------------- LIST OF STATE UART PROTOCOL
#define STATE_READY           0x00
#define STATE_START           0x01
#define STATE_PROCEDURE       0x02
#define STATE_ADDRESS         0x03
#define STATE_TOKEN           0x04
#define STATE_DATA            0x05
#define STATE_CS              0x06

// -------------- CAN PROTOCOL DEFINITION
#define CAN_MAX_BUF_SIZE       20

typedef enum{
	NONE_MESSAGE,
	MESSAGE_UART_IN,
	MESSAGE_UART_OUT,
	MESSAGE_CAN_IN,
	MESSAGE_CAN_OUT
} message_type;

typedef struct{
  message_type  handle; 
  unsigned short procedure;
  unsigned short address;
  unsigned char data[8];  
} standard_message;


#endif /* CONFIG_H_ */