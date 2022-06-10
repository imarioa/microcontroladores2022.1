#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL43Z4.h"
#include "fsl_debug_console.h"

//Definindo estruturas básicas

typedef struct{
    uint32_t PCR[32];
}PORTRegs_t;

typedef struct{
	 uint32_t PDOR;
	 uint32_t PSOR;
	 uint32_t PCOR;
	 uint32_t PTOR;
	 uint32_t PDIR;
	 uint32_t PDDR;
}GPIORegs_t;

// Definindo os PORTs
#define PORT_A ((PORTRegs_t *) 0x40049000)
#define PORT_B ((PORTRegs_t *) 0x4004A000)
#define PORT_C ((PORTRegs_t *) 0x4004B000)
#define PORT_D ((PORTRegs_t *) 0x4004C000)
#define PORT_E ((PORTRegs_t *) 0x4004D000)

// Definindo os GPIOs
#define GPIO_A ((GPIORegs_t* ) 0x400FF000)
#define GPIO_B ((GPIORegs_t *) 0x400FF040)
#define GPIO_C ((GPIORegs_t *) 0x400FF080)
#define GPIO_D ((GPIORegs_t *) 0x400FF0C0)
#define GPIO_E ((GPIORegs_t *) 0x400FF100)

//Funções auxiliares

void delay(unsigned int time){
    for(int i = 0; i < time; i++);
}
void gpio_init(void){
	SIM->SCGC5 = (1 << 10); // PORT B
	for(int i = 0; i < 4; i++){
		PORT_B->PCR[i] = (1 << 8);
		GPIO_B->PDDR |= (1 << i);
	}
}
char HOR[4] = {0x09,0x03,0x06,0x0C};    // Matriz dos bytes das Fases do Motor - sentido Horário Full Step
char AHO[4] = {0x0C,0x06,0x03,0x09};    // Matriz dos bytes das Fases do Motor - sentido Anti-Horário Full Step
int main(void) {
	gpio_init();
    /* Ativando os LEDs */
    while (1)
    {
    	for(int i = 0; i < 256; i++){
    		for(int j = 0; j < 4; j++){
    			GPIO_B->PDOR = HOR[j];
    			for(int k = 0; k < 0xAFF; k++);
    		}
    	}
    	for(int k = 0; k < 0xFFFFF; k++);
    	for(int i = 0; i < 256; i++){
    	    for(int j = 0; j < 4; j++){
    	    	GPIO_B->PDOR = AHO[j];
    	    	for(int k = 0; k < 0xAFF; k++);
    	    }
    	}
    	for(int k = 0; k < 0xFFFFF; k++);

    }
    return 0 ;
}
