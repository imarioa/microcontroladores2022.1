
/**
 * @file    adc-potenciometro.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL43Z4.h"
#include "fsl_debug_console.h"
/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */

/*
 * @brief   Application entry point.
 */
typedef struct
{
	uint32_t PCR[32];
} PORTRegs_t;

typedef struct
{
	uint32_t PDOR;
	uint32_t PSOR;
	uint32_t PCOR;
	uint32_t PTOR;
	uint32_t PDIR;
	uint32_t PDDR;
} GPIORegs_t;

// Definindo os PORTs
#define PORT_A ((PORTRegs_t *)0x40049000)
#define PORT_B ((PORTRegs_t *)0x4004A000)
#define PORT_C ((PORTRegs_t *)0x4004B000)
#define PORT_D ((PORTRegs_t *)0x4004C000)
#define PORT_E ((PORTRegs_t *)0x4004D000)

// Definindo os GPIOs
#define GPIO_A ((GPIORegs_t *)0x400FF000)
#define GPIO_B ((GPIORegs_t *)0x400FF040)
#define GPIO_C ((GPIORegs_t *)0x400FF080)
#define GPIO_D ((GPIORegs_t *)0x400FF0C0)
#define GPIO_E ((GPIORegs_t *)0x400FF100)

//Funções auxiliares

void delay(unsigned int time)
{
	for (int i = 0; i < time; i++)
		;
}
void gpio_init(void)
{
	SIM->SCGC5 = (1 << 10); // PORT B
	for (int i = 0; i < 4; i++)
	{
		PORT_B->PCR[i] = (1 << 8);
		GPIO_B->PDDR |= (1 << i);
	}
}
char HOR[4] = {0x09, 0x03, 0x06, 0x0C}; // Matriz dos bytes das Fases do Motor - sentido Horário Full Step
char AHO[4] = {0x0C, 0x06, 0x03, 0x09}; // Matriz dos bytes das Fases do Motor - sentido Anti-Horário Full Step

int main(void)
{
	/*Ativar o clock do ADC
	 *
	 * 	System Clock Gating Control Register 6 (SIM_SCGC6)
	 *|   31 | 30 |  29 | 28 |  27  |  26  |  25  |  24  |  23 | 22 -- 2 |    1   |  0  |
	 *| DAC0 |  0 | RTC |  0 | ADC0 | TPM2 | TPM1 | TPM0 | PIT |    0    | DMAMUX | FTF |
	 */
	gpio_init();
	SIM->SCGC6 = (1 << 27);

	/*Ativando o Clock enable for PORTB
	 *
	 * 	System Clock Gating Control Register 5 (SIM_SCGC5)
	 *| 31  --  20| 19 | 18 -- 14 | 13 | 12 | 11 | 10 |  9 | 8  7 | 6 |   5 | 4  3  2 | 1 |   0   |
	 *|     0	  |  0 |     0    | PE | PD | PC | PB | PA |   1  | 0 | TSI	|    0    | 0 | LPTMR |
	 */
	SIM->SCGC5 = (1 << 11) ;

	/* PORT_B_1 como ADC
	 *
	 * 	Pin Control Register n (PORTx_PCRn)
	 *| 31  --  25|  24 | 23 -- 20 | 19 -- 16 | 15 -- 11 | 10  9  8 | 7 |  6  | 5 |  4  | 3 |  2  | 1  |  0 |
	 *|     0	  | ISF |     0    |   IRQC   |     0    |    MUX   | 0 | DSE | 0 | PFE | 0 | SRE | PE | PS |      | 0 | LPTMR |
	 *|           | w1c |		   |
	 */
	PORTC->PCR[1] = (0b000 << 8);

	/* Configurar o ADC0 CFG1 Bus Clock / 2, 16 bit conversion, divide ration is 8
	*
	 * 	ADC Configuration Register 1 (ADCx_CFG1)
	 *| 31  --  8 |    7  | 6 -- 5 |    4   | 3 -- 2 |  1   0  |
	 *|     0	  | ADLPC |  ADIV  | ADLSMP |  MODE  |  ADICLK |
	 *
	 * 11 The divide ratio is 8 and the clock rate is (input clock)/8.
	 * 11 When DIFF=0:It is single-ended 16-bit conversion; when DIFF=1,
	 *    it is differential 16-bit conversion with 2's complement output.
	 * 01 (Bus clock)/2
	 */
	ADC0->CFG1 = (3 << 5) | (3 << 2) | 0x01;

	/* ADC0 SC3
	 *
	 * Status and Control Register 3 (ADCx_SC3)
	 *| 31  --  8 |   7  |   6  | 5  4 |  3   |   2  | 1  0 |
	 *|     0	  |  CAL | CALF |  0   | ADC0 | AVGE | AVGS |
	 */
	ADC0->SC3 = 0b00; //00 4 samples averaged.

	while (1)
	{
		uint32_t data;
		uint32_t voltage;

		/* No canal de configuração, o ADC iniciará a conversão
		 *
		 * ADC Status and Control Registers 1 (ADCx_SC1n)
		 *| 31  --  8 |   7   |  6   |  5   |  4 --  0 |
		 *|     0	  |  COCO | AIEN | DIFF |   ADCH   |
		 *
		 * 01001  When DIFF=0, AD9 is selected as input; when DIFF=1, it is reserved.
		 *
		 * Pagina 79 olhar na tabela
		 * 3.7.1.3.1 ADC0 Channel Assignment
		 */
		ADC0->SC1[0] = 0xF;
		/* A verificação se a conversão está completa ou não
		 *
		 * COCO (Conversion Complete Flag)
		 *
		 * 1 Conversion is completed.
		 */
		while ((ADC0->SC1[0] & (1 << 7)) == 0)
			;

		/* Ler dados ADC do resultado do registro */
		data = ADC0->R[0];
		/* 0xFFFF => 3.3V, data = que é volt. volt = (data * 3300) / 65535 */
		voltage = (data * 3300) / 65535;
		printf("data: %d voltage: %d\n", data, voltage);
		/* Ativando os LEDs */

		for (int i = 0; i < 256; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				GPIO_B->PDOR = HOR[j];
				for (int k = 0; k < voltage + 300; k++)
					;
			}
		}
		for (int k = 0; k < 0xFFFF; k++)
			;
		for (int i = 0; i < 256; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				GPIO_B->PDOR = AHO[j];
				for (int k = 0; k < voltage + 300; k++)
					;
			}
		}
		for (int k = 0; k < 0xFFFF; k++)
			;

	}
	return 0;
}
