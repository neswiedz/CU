#include "led.h"

void led_init(){
	LED_Y_DDR |= (1<<LED_Y);
	LED_G_DDR |= (1<<LED_G);
	led_y_off();
	led_g_off();
}

void led_can_off(void){
	led_y_off();
}

void led_can_on(void){
	led_y_on();
}
	
void led_can_toogle(void){
	led_y_toogle();
}

void led_uart_off(void){
	led_g_off();
}

void led_uart_on(void){
	led_g_on();
}

void led_uart_toogle(void){
	led_g_toogle();
}
	
void led_on(uint8_t port, uint8_t led){
	if (port == LED_Y_PORT) {
		LED_Y_PORT |= (1<<led);
	}
	if (port == LED_G_PORT){
		LED_G_PORT |= (1<<led);
	}
}

void led_off(uint8_t port, uint8_t led){
	if (port == LED_Y_PORT){
		LED_Y_PORT &= ~(1<<led);
	}
	if (port == LED_G_PORT){
		LED_G_PORT &= ~(1<<led);
	}
}

void led_y_on(void){
	led_on(LED_Y_PORT, LED_Y);		
}

void led_y_off(void){
	led_off(LED_Y_PORT, LED_Y);
}
void led_y_toogle(void){
	if(bit_is_set(LED_Y_PORT, LED_Y)){
		led_off(LED_Y_PORT, LED_Y);
	}
	else{
		led_on(LED_Y_PORT, LED_Y);
	}
}

void led_g_on(void){
	led_on(LED_G_PORT, LED_G);
}
void led_g_off(void){
	led_off(LED_G_PORT, LED_G);
}
void led_g_toogle(void){
	if(bit_is_set(LED_G_PORT, LED_G)){
		led_off(LED_G_PORT, LED_G);
	}
	else{
		led_on(LED_G_PORT, LED_G);
	}
}
