#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL43Z4.h"
#include "fsl_debug_console.h"

typedef struct
{
	uint32_t PCR[32];
} PORTRegs_t;

#define PORT_B ((PORTRegs_t *)0x4004A000)
#define PORT_D ((PORTRegs_t *)0x4004C000)
#define PORT_E ((PORTRegs_t *)0x4004D000)

typedef struct
{
	uint32_t PDOR;
	uint32_t PSOR;
	uint32_t PCOR;
	uint32_t PTOR;
	uint32_t PDIR;
	uint32_t PDDR;
} GPIORegs_t;

#define GPIO_B ((GPIORegs_t *)0x400FF040)
#define GPIO_D ((GPIORegs_t *)0x400FF0C0)
#define GPIO_E ((GPIORegs_t *)0x400FF100)

typedef struct
{
	uint32_t iser[1];
	uint32_t rsvd[31];
	uint32_t icer[1];
	uint32_t rsvd1[31];
	uint32_t ispr[1];
	uint32_t rsvd2[31];
	uint32_t icpr[1];
	uint32_t rsvd3[31];
	uint32_t rsvd4[64];
	uint32_t ipr[8];
} NVIC_Regs_t;

#define NVIC_REG ((NVIC_Regs_t *)0xE000E100)

typedef struct
{
	uint32_t tcsr;
	uint32_t prescale;
	uint32_t compare;
	uint32_t count;
} LPTMR_Regs_t;

#define LPTMR_REG ((LPTMR_Regs_t *)0x40040000)

void LPTMR0_IRQHandler(void)
{
	GPIO_E->PTOR = (1 << 31);
	LPTMR_REG->tcsr = LPTMR_REG->tcsr | (1 << 7);
}
void gpio_init(void){
    SIM->SCGC5 = (1 << 13); // Ativando clock para o PORT E
    //PORT_E 31 - Led Green / PORT_D 5 - Led Red
    PORT_E->PCR[31] = (1 << 8); //Ativando o GPIO
    GPIO_E->PDDR = (1 << 31); // Definindo a porta 31 como saída

}
void lptmr_init(void){
    SIM->SCGC5 = (1 << 0);
    LPTMR_REG->prescale = (1 << 2) | (1 << 0);
    LPTMR_REG->compare = 999;
    LPTMR_REG->tcsr = (1 << 0) | (1 << 6);
}
int main(void)
{
	gpio_init();
    lptmr_init();
	NVIC_REG->iser[0] = (1 << 28);
	while (1)
	{
	}

	return 0;
}
