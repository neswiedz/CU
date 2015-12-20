#ifndef LED_H_
#define LED_H_

#include <avr/io.h>

#define LED_Y_PORT 	PORTA
#define LED_Y_DDR 	DDRA
#define LED_Y		PA7
#define LED_G_PORT 	PORTG
#define LED_G_DDR	DDRG
#define LED_G		PG2

void led_init();

void led_on(uint8_t port, uint8_t led);
void led_off(uint8_t port, uint8_t led);

void led_y_on(void);
void led_y_off(void);
void led_y_toogle(void);

void led_g_on(void);
void led_g_off(void);
void led_g_toogle(void);
void led_uart_on(void);void led_uart_off(void);void led_uart_toogle(void);void led_can_on(void);void led_can_off(void);void led_can_toogle(void);
#endif