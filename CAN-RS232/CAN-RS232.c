/*
 * CAN_RS232.c
 *
 * Created: 2012-04-29 08:15:30
 *  Author: Sotarsoft
 */ 

//_____ I N C L U D E S ________________________________________________________
#include <avr/io.h>
#include "uart_drv.h"
#include "uart_lib.h"
#include "rtc_drv.h"
#include "can.h"
#include "utils.h"
#include "led.h"
#include "timer16_drv.h"
#include "OW.h"

//_____ D E C L A R A T I O N S ________________________________________________
long taba[10];
long z;
int kz;
		
// RTC variable 
extern volatile U32 rtc_tics;
extern volatile U16 rtc_milliseconds;
extern volatile U8  rtc_seconds;
extern volatile U8  rtc_minutes;
extern volatile U8  rtc_hours;
extern volatile U8  rtc_days;
extern Bool rtc_running;

// MESSAGES
standard_message message_buffor[MESSAGE_BUF_SIZE];
unsigned int message_pntr = 0x00;
unsigned int actual_message_pntr = 0x00;

// UART SIMPLE BUFFOR
unsigned char uart_input_buffer[UART_MAX_BUF_SIZE];
unsigned char *uart_input_buffer_write;
unsigned char *uart_input_buffer_read;
unsigned char uart_getchar_last;

// UART PROTOCOL variable
unsigned char uart_rx_buffer[UART_MAX_BUF_SIZE];  // UART RX buffer
unsigned char *uart_rx_pntr = &uart_rx_buffer[0];
unsigned char uart_msg_cs = 0x00;    // UART calculated message checksum
unsigned char uart_msg_size = 0x00;   
unsigned short uart_msg_procedure = 0x0000; 
unsigned char uart_msg_procedure_length = 0x00;
unsigned short uart_msg_address = 0x0000;   
unsigned char uart_msg_address_length = 0x00;
unsigned char uart_state = STATE_READY;   // UART actual state
U16 uart_start_timeout = 0x00; // UART timeout saver RTC in 

// CAN PROTOCOL variable
extern CAN_messageType can_rx_buffer[CAN_MAX_BUF_SIZE];
//<MSMS 2015-12-19> //extern U8 can_rx_index = 0x00;
extern U8 can_rx_index;
U8 can_rx_current_index = 0x00;
unsigned char can_sequence_number = 0x00; // UART sequence number from host

// Function declarations
Bool app_init(void);
void message_loop(void);

void uart_int_init(void);
Bool uart_int_getchar(void);
void uart_int_getchar_int(void);
void uart_flush_input_buffer(void);
void uart_flush_output_buffer(void);

Bool uart_timeout_check(U16 time);
void uart_message_int(void);

void handle_message(standard_message message);
void send_message_to_can(standard_message);
void send_message_to_uart(standard_message);

uint8_t channel = 0x00;

//_____ F U N C T I O N S ______________________________________________________

Bool app_init()
{
	asm("cli");
	
	// Clock prescaler Reset
	CLKPR = 0x80; CLKPR = 0x00;
	
	// Init Leds
	led_init();
	
	// Init UART
    Uart_select(UART_0);
    uart_init(CONF_8BIT_NOPAR_1STOP, 115200);
    uart_int_init();
	
	// Init RTC Timer;
	rtc_int_init();

    // Init CAN interface
    if (initCAN(_250KBPS)) {
		configRX();
		sei();
		return TRUE;
	} else {
		//<MSMS2015-12-19> uart_mini_printf('EXIT PROGRAM');
		uart_mini_printf('EXIT');
    	return FALSE;
	}
}

void uart_int_init()
{
	UCSR0B |= _BV(RXCIE0); //| _BV(UDRIE0);	//enable RX i TX
	uart_input_buffer_write = uart_input_buffer;
	uart_input_buffer_read = uart_input_buffer;
}

Bool uart_int_getchar()
{
	if (uart_input_buffer_read != uart_input_buffer_write)
	{
		uart_getchar_last = *uart_input_buffer_read;
		if (uart_input_buffer_read < (&uart_input_buffer[UART_MAX_BUF_SIZE-1])) 
			uart_input_buffer_read++;
		else
			uart_input_buffer_read = uart_input_buffer;
		return 1;		
	} else {		
		return 0;
	}	
}

void uart_int_getchar_int()
{
	cli();
	*uart_input_buffer_write = UDR0;
	if (uart_input_buffer_write<(&uart_input_buffer[UART_MAX_BUF_SIZE-1]))
		uart_input_buffer_write++;
	else
		uart_input_buffer_write=uart_input_buffer;
	sei();	
}

void uart_flush_input_buffer()
{
	uart_input_buffer_read = uart_input_buffer_write;
}

Bool uart_timeout_check(U16 time)
{
    if (uart_start_timeout > time) {
		return uart_start_timeout + 200 < time + 1000;
	} else {
		return uart_start_timeout + 200 < time;
	}	
}

void uart_message_int()
{
	unsigned char rx_data;
	//<MSMS2015-12-19> standard_message tmp;
	unsigned int i;

    if (uart_int_getchar()) {	
		led_uart_on();
		rx_data = uart_getchar_last;
	    
		switch (uart_state) {
			case STATE_READY:
				if (rx_data == MESSAGE_START) {
					uart_start_timeout = rtc_milliseconds;
					uart_msg_cs = 0x00;
					uart_msg_procedure = 0x0000;
					uart_msg_procedure_length = 0x00;
					uart_msg_address = 0x0000;
					uart_msg_address_length = 0x00;
					uart_rx_pntr = &uart_rx_buffer[0];  //reset pointer
					uart_state = STATE_PROCEDURE;
				} else {
					uart_msg_cs = 0x00; // clear old checksum
				}
				break;
			case STATE_PROCEDURE:
				if (uart_timeout_check(rtc_milliseconds)) {
					uart_state = STATE_READY;
				} else { 
					if (uart_msg_procedure_length == 0x00) {
						uart_msg_procedure = rx_data << 8;
						uart_msg_procedure_length++;
					} else if(uart_msg_procedure_length == 0x01) {
						uart_msg_procedure |= rx_data;
						uart_msg_procedure_length++;
						uart_state = STATE_ADDRESS;
					} else if (uart_msg_procedure_length > 0x01) {
						uart_state = STATE_READY;
					}
				}			
				break;
			case STATE_ADDRESS:
				if (uart_timeout_check(rtc_milliseconds)) {
					uart_state = STATE_READY;
				} else {
					if (uart_msg_address_length == 0x00) {
						uart_msg_address = rx_data << 8;
						uart_msg_address_length++;
					} else if(uart_msg_address_length == 0x01) {
						uart_msg_address |= rx_data;
						uart_msg_address_length++;
						uart_state = STATE_TOKEN;
					} else if (uart_msg_address_length > 0x01) {
						uart_state = STATE_READY;
					}
				}
				break;			
			case STATE_TOKEN:
				if (uart_timeout_check(rtc_milliseconds)) {
					uart_state = STATE_READY;
				} else {
					if (rx_data == MESSAGE_TOKEN)  {
						uart_msg_size = MESSAGE_DATA_SIZE;
						uart_state = STATE_DATA;
					} else { 
						uart_state = STATE_READY;
					}						
				}
				break;
			case STATE_DATA:
				if (uart_timeout_check(rtc_milliseconds)) {
					uart_state = STATE_READY;
				} else {
					*uart_rx_pntr++ = rx_data;
					--uart_msg_size;
					if (uart_msg_size == 0)
					{
						uart_state = STATE_CS;
					}				
				}
				break;
			case STATE_CS:
				if (uart_timeout_check(rtc_milliseconds)) {
					uart_state = STATE_READY;
				} else {
					if (rx_data == uart_msg_cs) {
						message_pntr++;
						if (message_pntr>=MESSAGE_BUF_SIZE) {
							message_pntr=0;
						}
						message_buffor[message_pntr].handle = MESSAGE_UART_IN;
						message_buffor[message_pntr].procedure = uart_msg_procedure;
						message_buffor[message_pntr].address = uart_msg_address;
						for (i=0; i<MESSAGE_DATA_SIZE; i++) {
							message_buffor[message_pntr].data[i] = uart_rx_buffer[i]; 
						} 
					}
					uart_state = STATE_READY;
				}
				break;				
		}
		uart_msg_cs ^= rx_data;
	}		
}

// SAMPLE: 1B1030AAAA0E41424344454647483D;
// SAMPLE: 1B0103AAAA0E41424344454647481F;

void send_message_to_can(standard_message message)
{
	CAN_messageType msg;

/*
	uart_mini_printf("CAN TX: %02X %02X %02X %02X %02X%02X%02X%02X%02X%02X%02X%02X\r\n", HIGH_BYTE(message.procedure), LOW_BYTE(message.procedure), HIGH_BYTE(message.address), LOW_BYTE(message.address), message.data[0], message.data[1], message.data[2], message.data[3],
	message.data[4], message.data[5], message.data[6], message.data[7]);
*/
	
	msg.ide = 1;
	msg.id = message.procedure;
	msg.ext = message.address;
	msg.length = MESSAGE_DATA_SIZE;
	for(int i=0; i<msg.length; i++) {
	   msg.data[i] = message.data[i];
  	}
	can_tx(msg);
	
}


void send_message_to_uart(standard_message message) 
{

/*
	uart_mini_printf("CAN RX: %02X %02X %02X %02X %02X%02X%02X%02X%02X%02X%02X%02X\r\n", HIGH_BYTE(message.procedure), LOW_BYTE(message.procedure), HIGH_BYTE(message.address), LOW_BYTE(message.address), message.data[0], message.data[1], message.data[2], message.data[3],
	message.data[4], message.data[5], message.data[6], message.data[7]);
*/

    char cs = 0x00;
    uart_putchar(MESSAGE_START);
	cs ^= MESSAGE_START;
	uart_putchar(HIGH_BYTE(message.procedure));
	cs ^= HIGH_BYTE(message.procedure);	
	uart_putchar(LOW_BYTE(message.procedure));
	cs ^= LOW_BYTE(message.procedure);
	uart_putchar(HIGH_BYTE(message.address));
	cs ^= HIGH_BYTE(message.address);
	uart_putchar(LOW_BYTE(message.address));
	cs ^= LOW_BYTE(message.address);
    uart_putchar(MESSAGE_TOKEN);
    cs ^= MESSAGE_TOKEN;
	for (int i=0; i<MESSAGE_DATA_SIZE; i++) {
		uart_putchar(message.data[i]);
		cs ^= message.data[i];
	}
	uart_putchar(cs);
		
}

void handle_message(standard_message message)
{
	//<MSMS2015-12-19> standard_message mess2;
	
	switch (message.handle) {
		case MESSAGE_UART_IN:
		    message.handle = MESSAGE_CAN_OUT;
			handle_message(message);
			break;
		case MESSAGE_UART_OUT:
			send_message_to_uart(message);
			break;
		case MESSAGE_CAN_IN:
		    message.handle = MESSAGE_UART_OUT;
			handle_message(message);
			break;
		case MESSAGE_CAN_OUT:
			send_message_to_can(message);
			break;
	}
}

void can_message_loop()
{
	U8 i;
	
	if (can_rx_current_index != can_rx_index) {
 		led_can_on();
		can_rx_current_index++;
		if (can_rx_current_index>=CAN_MAX_BUF_SIZE) {
			can_rx_current_index=0;
		}
				
		message_pntr++;
		if (message_pntr>=MESSAGE_BUF_SIZE) {
			message_pntr=0;
		}

/*
		uart_mini_printf("CAN RX: %d %d %c%c%c%c%c%c%c%c", can_rx_buffer[can_rx_index].id, 0x01, can_rx_buffer[can_rx_index].data[0], can_rx_buffer[can_rx_index].data[1], can_rx_buffer[can_rx_index].data[2], can_rx_buffer[can_rx_index].data[3],
 can_rx_buffer[can_rx_index].data[4], can_rx_buffer[can_rx_index].data[5], can_rx_buffer[can_rx_index].data[6], can_rx_buffer[can_rx_index].data[7]);
*/
		message_buffor[message_pntr].handle = MESSAGE_CAN_IN;
		message_buffor[message_pntr].procedure = can_rx_buffer[can_rx_index].id;
		message_buffor[message_pntr].address = can_rx_buffer[can_rx_index].ext;
		for (i=0; i<MESSAGE_DATA_SIZE; i++) {
			message_buffor[message_pntr].data[i] = can_rx_buffer[can_rx_index].data[i];
		}
	}
}

void message_loop()
{
	unsigned int next_message = 0x00;
    Bool is_next_message = 0;
	
	if (actual_message_pntr != message_pntr) {
		next_message = actual_message_pntr + 1;
		if (next_message >= MESSAGE_BUF_SIZE) {
			next_message = 0;
		}
	
	    if (next_message > message_pntr) {
			if (next_message <= message_pntr + MESSAGE_BUF_SIZE) {
				is_next_message = 1;
			}
		}
		if (next_message <= message_pntr) {
			is_next_message = 1;
		}
		
		if (is_next_message) {
			actual_message_pntr = next_message;
			handle_message(message_buffor[actual_message_pntr]);
		} 
	}	
}

int main(void)
{
	// Inicjalizuj wszystkie biblioteki
	if (app_init()) {
	
		// Start program
		uart_mini_printf ("\r\n===============================================\r\n");
		uart_mini_printf ("=                                             =\r\n");
		uart_mini_printf ("=   DOMOMATIK  CAN-RS232  ver.1.0  (c) 2012   =\r\n");
		uart_mini_printf ("=                                             =\r\n");
		uart_mini_printf ("===============================================\r\n");
		
		//timer
		//<MSMS2015-12-19> uint16_t licznik;

		int i;
		z=0;
		kz=0;
		for (i =0; i<10;i++){
			taba[i]=0;
		}
		/*
		Timer16_set_clock(TIMER16_CLKIO_BY_1);
		//licznik =  Timer16_get_counter_low();
		//licznik |= (Timer16_get_counter_high() << 8 );
		Timer16_set_mode_output_a(TIMER16_COMP_MODE_NORMAL);
		Timer16_set_compare_a(7);
		Timer16_compare_a_it_enable();
		sei(); */
		led_g_on();
		led_y_on();	
		// GLOWNA PETLA PROGRAMU
		//<MSMS2015-12-19>long a=0;
		//<MSMS2015-12-19>long b=0;
		if (OW_reset_pulse()){
			led_y_toogle();
		}
		//OW_byte_wr(OW_SKIP_ROM);
		while(1)
		{
			//uart_message_int();
			//can_message_loop();
			
			//message_loop();
			//led_can_off();
			//led_uart_off();
				   
				 //  for(i=0;i<255;i++){
				//	   a++;
				//	   a=b;
				//   }
				   led_g_toogle();
			/*if(kz<10){
				z++;
				//if(Timer16_get_compare_a_it()){
				//   Timer16_set_compare_a(7);
				//   led_g_toogle();
				//   taba[kz] = z;
				//   kz++;
				//}
			} else if(kz==10) {
				kz++;
				uart_mini_printf("%d, %d\n",taba[0],taba[1]);	
				uart_mini_printf("%d, %d\n",taba[2],taba[3]);
				uart_mini_printf("%d, %d\n",taba[4],taba[5]);
				uart_mini_printf("%d, %d\n",taba[6],taba[7]);
				uart_mini_printf("%d, %d\n",taba[8],taba[9]);
			}
			*/
		}
	}	
}


//_____ M A C R O S ____________________________________________________________

ISR(USART0_RX_vect) {
   uart_int_getchar_int();
}