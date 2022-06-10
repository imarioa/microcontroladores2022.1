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
void PIT_IRQHandler(){

	//Determine which channel triggered interrupt
	if (PIT->CHANNEL[0].TFLG & 0x1) {
		GPIO_E->PTOR |= (1 << 31);	//toggle RED
		//Clear interrupt request flag for channel
		PIT->CHANNEL[0].TFLG = 0x1;
	}
	if (PIT->CHANNEL[1].TFLG & 0x1) {
			GPIO_D->PTOR |= (1 << 5);	//toggle RED
			//Clear interrupt request flag for channel
			PIT->CHANNEL[1].TFLG = 0x1;
	}
}
//953us
// V = (T/s * 10485760)–1
int main(void) {
	/* Ativando os clocks */
    SIM->SCGC5 = (1 << 13) | (1 << 12); // Clock GPIO
    SIM->SCGC6 |= (1 << 23); //Clock PIT

    /*Ativando GPIO*/
    PORT_E->PCR[31] = (1 << 8);
    PORT_D->PCR[5] = (1 << 8);
    GPIO_E->PDDR = (1 << 31); // Definindo como saída
    GPIO_D->PDDR |= (1 << 5);
    GPIO_D->PSOR |= (1 << 5);
    GPIO_E->PSOR |= (1 << 31); //Apagando led

    /* Obtendo o valor de LDVAL por meio da formula LDVAL = ((tempo desejado/s) * freq_clock) - 1 */
    uint32_t clock = CLOCK_GetBusClkFreq();
    uint32_t ldval = (5 * clock) - 1;

    /*Configuração do PIT*/
	//LED Red
    PIT->MCR &= ~(1 << 1);
    PIT->CHANNEL[0].LDVAL = ldval; //5s
    PIT->CHANNEL[0].TCTRL |= 0x3;
	// LED Green
    ldval = (2 * clock) - 1;
    PIT->CHANNEL[1].LDVAL = ldval; //2s
    PIT->CHANNEL[1].TCTRL |= 0x3;

    NVIC_EnableIRQ(PIT_IRQn);

    while(1){

    }
    return 0 ;
}
