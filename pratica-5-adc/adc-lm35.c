
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"

/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */

/*
 * @brief   Application entry point.
 */

void init_ADC(void){
	int cal_res;

	// Ativar o clock ADC0
	SIM->SCGC6 |= (1 << 27);
	SIM->SCGC5 |= (1 << 10);

	PORTB->PCR[0] = (0b000 << 8);

	do{
		// Calibrate ADC
		cal_res = adc_cal();
	} while (cal_res == -1);

	// Configure ADC
	ADC0->SC1[0] |= (1 << 6);  // interrupção ativada
	ADC0->SC1[0] &= (0 << 5);   // Single-Ended ADC

	ADC0->CFG1 = 0; // Reset register
	ADC0->CFG1 |= 	(0b10 << 2)  |  // 10 bits mode
					(0b01 << 0)  |	// Input Bus Clock/2 (24 Mhz)
					(0b11 << 5)  | 	// Clock divide by 8 (3 Mhz)
					(1 << 7); 		// Low power mode

	ADC0->SC3  &= (0 << 2);
}

int adc_cal(void){
	uint16_t calib;

	ADC0->CFG1 |= (0b11 << 2)  |  	// 16 bits mode
				  (0b01 << 0)  |	// Input Bus Clock divided by 2 (48Mhz / 2)
				  (0b11 << 5);	 	// Clock divide by 8 (3 MHz)

	ADC0->SC3  |= (1 << 2) 		|	// Enable HW average
				  (0b11 << 0)   |	// Set HW average of 32 samples
				  (1 << 7); 	 	// Start calibration process

	while (ADC0->SC3 & (1 << 7)); // Wait for calibration to end

	if (ADC0->SC3 & (1 << 6))	// Check for successful calibration
		return -1;

	calib = 0;
	calib += ADC0->CLPS + ADC0->CLP4 + ADC0->CLP3 +
			     ADC0->CLP2 + ADC0->CLP1 + ADC0->CLP0;
	calib /= 2;
	calib |= 0x8000; 	// Set MSB
	ADC0->PG = calib;

	calib = 0;
	calib += ADC0->CLMS + ADC0->CLM4 + ADC0->CLM3 +
			     ADC0->CLM2 + ADC0->CLM1 + ADC0->CLM0;
	calib /= 2;
	calib |= 0x8000;	// Set MSB
	ADC0->MG = calib;

	return 0;
}

void scan_temperature(void){
	static uint16_t adc_result;
	static uint16_t adc_result_avg;
	static uint16_t temp;
	static uint8_t loopcntr;

	uint32_t data;
	uint32_t voltage;

	/* Incrementa a cada 50 ms */
	loopcntr++;

	ADC0->SC1[0] = ( (8 & (8 << 0)) | (ADC0->SC1[0] & ((1 << 6) | (1 << 5)  )
					) );

	while(ADC0->SC2 & (1 << 7)); 	 // Conversion in progress
	while(!(ADC0->SC1[0] & (1 << 7) )); // Run until the conversion is complete
	//while((ADC0->SC1[0] & (1 << 7)) == 0); // add

	/* Obtém o resultado do ADC para o sensor de temperatura */
	adc_result  = adc_result + ADC0->R[0];

	//data = ADC0->R[0];//ADD

	/* Pegue 16 amostras para calcular a temperatura */
	if (loopcntr >= 16){
		adc_result_avg = adc_result >> 4;       /* obtendo o valor médio do ADC*/
		temp = ((adc_result_avg * 300) >> 10); /* Calcular a temperatura */

		adc_result = 0;
		loopcntr = 0;
	}
	/* Exibe a temperatura */

	printf("Temperatura : %d\n", temp);
	/* Desativa o módulo ADC */
	ADC0->SC1[0] |= (0b11111 << 0);
}


int main(void) {
	init_ADC();
	while(1){
		scan_temperature();
		for(int i =0; i < 0xFFF; i++);
	}
    return 0 ;
}