/**
 * @file    switch-led.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL43Z4.h"
#include "fsl_debug_console.h"

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

int main(void)
{
    /* Ativando os clocks */
    SIM->SCGC5 = (1 << 9) | (1 << 12);

    /* Ativando GPIO */
    PORT_A->PCR[13] = (1 << 8); //Led 1
    PORT_A->PCR[1] = (1 << 8); //Led 2

    GPIO_A->PDDR = (1 << 13);
	GPIO_A->PDDR = (1 << 1);

    PORT_D->PCR[4] = (1 << 8) | 0x2; // Sw1
    PORT_A->PCR[12] = (1 << 8) | 0x2; // Sw2

    while (1)
    {
		if (GPIO_D->PDIR & (1 << 4)){
	        GPIO_A->PSOR = (1 << 1);
	    }else{
			GPIO_A->PCOR = (1 << 1);
		}
	    if (GPIO_A->PDIR & (1 << 12)){
	        GPIO_A->PSOR = (1 << 13);
	    }else{
	        GPIO_A->PCOR = (1 << 13);
	    }
    }

    return 0;
}
