
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
int main(void) {
	/*Ativar o clock do ADC
	 *
	 * 	System Clock Gating Control Register 6 (SIM_SCGC6)
	 *|   31 | 30 |  29 | 28 |  27  |  26  |  25  |  24  |  23 | 22 -- 2 |    1   |  0  |
	 *| DAC0 |  0 | RTC |  0 | ADC0 | TPM2 | TPM1 | TPM0 | PIT |    0    | DMAMUX | FTF |
	 */
	SIM->SCGC6 = (1 << 27);

	/*Ativando o Clock enable for PORTB
	 *
	 * 	System Clock Gating Control Register 5 (SIM_SCGC5)
	 *| 31  --  20| 19 | 18 -- 14 | 13 | 12 | 11 | 10 |  9 | 8  7 | 6 |   5 | 4  3  2 | 1 |   0   |
	 *|     0	  |  0 |     0    | PE | PD | PC | PB | PA |   1  | 0 | TSI	|    0    | 0 | LPTMR |
	 */
	SIM->SCGC5 = (1 << 10);

	/* PORT_B_1 como ADC
	 *
	 * 	Pin Control Register n (PORTx_PCRn)
	 *| 31  --  25|  24 | 23 -- 20 | 19 -- 16 | 15 -- 11 | 10  9  8 | 7 |  6  | 5 |  4  | 3 |  2  | 1  |  0 |
	 *|     0	  | ISF |     0    |   IRQC   |     0    |    MUX   | 0 | DSE | 0 | PFE | 0 | SRE | PE | PS |      | 0 | LPTMR |
	 *|           | w1c |		   |
	 */
	PORTB->PCR[1] = (0b000 << 8);

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
	ADC0->SC3 = 0b00;//00 4 samples averaged.

	while(1){
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
		ADC0->SC1[0] = (9 << 0);
		/* A verificação se a conversão está completa ou não
		 *
		 * COCO (Conversion Complete Flag)
		 *
		 * 1 Conversion is completed.
		 */
		while((ADC0->SC1[0] & (1 << 7)) == 0);

		/* Ler dados ADC do resultado do registro */
		data = ADC0->R[0];
		/* 0xFFFF => 3.3V, data = que é volt. volt = (data * 3300) / 65535 */
		voltage = (data * 3300) / 65535;

		printf("data: %d voltage: %d\n", data, voltage);
	}
    return 0 ;
}
