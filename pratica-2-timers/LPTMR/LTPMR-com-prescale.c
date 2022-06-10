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

// Variaveis dos leds
int red = 0;
int green = 0;

void LPTMR0_IRQHandler(void)
{
	red++;
	green++;
	if (red == 6)
	{
		GPIO_E->PTOR = (1 << 31);
		red = 0;
	}
	if (green == 2)
	{
		GPIO_D->PTOR = (1 << 5);
		green = 0;
	}
	LPTMR_REG->tcsr = LPTMR_REG->tcsr | (1 << 7);
}
/* 
-------- Implementação para 8s (Item 2) ------
void LPTMR0_IRQHandler(void){
	GPIO_E->PTOR = (1 << 31);
	LPTMR_REG->tcsr = LPTMR_REG->tcsr | (1 << 7);
}
------------------------------------------------
*/
void gpio_init(void){
	SIM->SCGC5 = (1 << 12) | (1 << 13);
	PORT_E->PCR[31] = (1 << 8);
	PORT_D->PCR[5] = (1 << 8);
	GPIO_E->PDDR = (1 << 31);
	GPIO_D->PDDR = (1 << 5);
	
}
void ltpmr_init(void){
	SIM->SCGC5 = (1 << 0);
	//Definindo prescale de 64
	LPTMR_REG->prescale = 0x28 | (1 << 0);
	/* A lógica aplicada é a seguinte, vou configurar o timer para 1s e a cada 
     contagem ele vai incrementar duas variaveis, red e green que simbolizam os leds da placa,
     quando a variavel red chegar em 6 significa que passou 6s então o led muda, o mesmo vale
     para a variavel green */

	//LPO é 1kHz -> T = 1/f = 1/1000/64 = 64/1000 = 64x10^(-3)
	// Vou colocar 1s => 1/64x10^(-3) = 15,625 ~ 16

	// LPTMR_REG->compare = 125; //Item 1: 8s
	LPTMR_REG->compare = 16;

	//Ativar Timer Enable -- 0 LPTMR is disabled and internal logic is reset -- 1 LPTMR is enabled
	//Timer Interrupt Enable -- 0 Timer interrupt disabled -- 1 Timer interrupt enabled
	LPTMR_REG->tcsr |= (1 << 0) | (1 << 6);

	
}
int main(void)
{
	gpio_init();
	ltpmr_init();
	NVIC_REG->iser[0] = (1 << 28);

	while (1)
	{
	}

	return 0;
}
