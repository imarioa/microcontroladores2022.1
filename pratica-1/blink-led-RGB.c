/**
 * @file    blink-led-RGB.c
 * @brief   Application entry point.
 */
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

int main(void) {
	/* Ativando os clocks */
    SIM->SCGC5 = (1 << 9) | (1 << 12);

    /* Ativando GPIO */
    PORT_A->PCR[1] = (1 << 8); //Red
    PORT_A->PCR[2] = (1 << 8); //Green
    PORT_D->PCR[3] = (1 << 8); //Blue
    
    /* Definindo os pinos GPIO como saída */
    GPIO_A->PDDR = (1 << 1) | (1 << 2);
    GPIO_D->PDDR = (1 << 3);

    /* Definindo o nível lógico alto para as saídas pq esse Led acende com 0 */
    GPIO_A->PSOR |= (1 << 1);
    GPIO_A->PSOR |= (1 << 2);
    GPIO_D->PSOR |= (1 << 3);

    /* Ativando os LEDs */
    while (1)
    {
    	//Red
        GPIO_A->PCOR = (1 << 1);
        delay(0xFFFFF);
        GPIO_A->PSOR = (1 << 1);
        delay(0xFFFFF);
        //Green
        GPIO_A->PCOR = (1 << 2);
        delay(0xFFFFF);
        GPIO_A->PSOR = (1 << 2);
        delay(0xFFFFF);
        //Blue
        GPIO_D->PCOR = (1 << 3);
        delay(0xFFFFF);
        GPIO_D->PSOR = (1 << 3);
        delay(0xFFFFF);
        //Red + Green
        GPIO_A->PCOR = (1 << 1);
        GPIO_A->PCOR = (1 << 2);
        delay(0xFFFFF);
        GPIO_A->PSOR = (1 << 1);
        GPIO_A->PSOR = (1 << 2);
        delay(0xFFFFF);
        //Red + Blue
        GPIO_A->PCOR = (1 << 1);
        GPIO_D->PCOR = (1 << 3);
        delay(0xFFFFF);
        GPIO_A->PSOR = (1 << 1);
        GPIO_D->PSOR = (1 << 3);
        delay(0xFFFFF);
        //Blue + Green
        GPIO_A->PCOR = (1 << 2);
        GPIO_D->PCOR = (1 << 3);
        delay(0xFFFFF);
        GPIO_A->PSOR = (1 << 2);
        GPIO_D->PSOR = (1 << 3);
        delay(0xFFFFF);

    }
    return 0 ;
}
