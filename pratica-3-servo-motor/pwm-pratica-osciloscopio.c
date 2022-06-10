
/**
 * @file    PWM_Osciloscopio.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"
/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */
typedef struct{
	uint32_t PCR[32];
}PORTRegs_t;

#define PORT_A ((PORTRegs_t *)0x40049000)
#define PORT_B ((PORTRegs_t *)0x4004A000)

typedef struct{
	uint32_t sc;
	uint32_t cnt;
	uint32_t mod;
	struct {
	    uint32_t CnSC;
	    uint32_t CnV;
	} CONTROLS[6];
	uint8_t RESERVED_0[20];
	uint32_t status;
	uint8_t RESERVED_1[48];
	uint32_t conf;
}TPMR_Regs_t;

#define TPM0_REG ((TPMR_Regs_t *) 0x40038000)
#define TPM1_REG ((TPMR_Regs_t *) 0x40039000)
#define TPM2_REG ((TPMR_Regs_t *) 0x4003A000)

typedef struct{
	uint32_t sopt2;
}SOPT2R_Regs_t;

#define SOPT2_REG ((SOPT2R_Regs_t *) 0x40048004)



#define PTB2_PIN 2
#define PTA12_PIN 12
//#define FREQ_2_MOD(x) (375000/x)

/*
 * Configuração do canal 1 do módulo TPM1 com a função "PWM".
 *
 */
void InitTPMPWM(void){
	/*
	 * 	System Clock Gating Control Register 5 (SIM_SCGC5)
	 *| 31  --  20| 19 | 18 -- 14 | 13 | 12 | 11 | 10 |  9 | 8  7 | 6 |   5 | 4  3  2 | 1 |   0   |
	 *|     0	  |  0 |     0    | PE | PD | PC | PB | PA |   1  | 0 | TSI	|    0    | 0 | LPTMR |
	 */
	SIM->SCGC5 = (1 << 9) | (1 << 10); //ativar clock porta B
	/*
	 * 	Pin Control Register n (PORTx_PCRn)
	 *| 31  --  25|  24 | 23 -- 20 | 19 -- 16 | 15 -- 11 | 10  9  8 | 7 |  6  | 5 |  4  | 3 |  2  | 1  |  0 |
	 *|     0	  | ISF |     0    |   IRQC   |     0    |    MUX   | 0 | DSE | 0 | PFE | 0 | SRE | PE | PS |      | 0 | LPTMR |
	 *|           | w1c |		   |
	 */
	PORT_B->PCR[PTB2_PIN] = 	(1 << 24) |		// ISF=PORTB_PCR18[24]: w1c (limpa a pendência)
							(0b011 << 8);   // MUX=PORTB_PCR18[10:8]=0b011 (TPM2_CH0)

	PORT_A->PCR[PTA12_PIN] = 	(1 << 24) |		// ISF=PORTB_PCR18[24]: w1c (limpa a pendência)
								(0b011 << 8);   // MUX=PORTB_PCR18[10:8]=0b011 (TPM2_CH0)
	/*
	 * 	System Clock Gating Control Register 6 (SIM_SCGC6)
	 *|   31 | 30 |  29 | 28 |  27  |  26  |  25  |  24  |  23 | 22 -- 2 |    1   |  0  |
	 *| DAC0 |  0 | RTC |  0 | ADC0 | TPM2 | TPM1 | TPM0 | PIT |    0    | DMAMUX | FTF |
	 */
	SIM->SCGC6 = (1 << 25) | (1 << 26);

	/*
	 * 	System Options Register 2 (SIM_SOPT2)
	 *|   31 -- 28 |   27  26 | 25  24 | 23 -- 19 |   18   | 17 |     16    | 15 -- 8 |   7 -- 5  |     4      | 3 -- 0 |
	 *|      0     | UART0SRC | TPMSRC |    0     | USBSRC |  0 | PLLFLLSEL |     0   | CLKOUTSEL | RTCCLKOUTS |    0   |
	 */
	//PLL/FLL clock select
	SOPT2_REG->sopt2 = (0 << 16);	//0 MCGFLLCLK clock
									//1 MCGPLLCLK clock with fixed divide by two
	/*TPMSRC	 *
	 * TPM clock source select
	 * 	Selects the clock source for the TPM counter clock
	 * 	00 Clock disabled
	 * 	01 MCGFLLCLK clock or MCGPLLCLK/2
	 * 	10 OSCERCLK clock
	 * 	11 MCGIRCLK clock
	*/
	SOPT2_REG->sopt2 = (0b01 << 24);	//(TPMSRC) clock source select
									//Selects the clock source for the TPM counter clock
									// 01  MCGFLLCLK clock or MCGPLLCLK/2

	//valor do modulo = 20971520 / 128 = 163840 / 3276 = 50 Hz
	TPM1_REG->mod = 3276;    		// MOD=TPM2_MOD[15:0]=3276
	TPM2_REG->mod = 3276;
	
	//valor do modulo = 48000000 / 128 = 375000 / 7500 = 50 Hz
	//TPM1_REG->mod = 7500;    		// MOD=TPM2_MOD[15:0]=7500

	/*
	 * 	Status and Control (TPMx_SC)
	 *|   31 -- 9 |  8  |  7  |   6  |   5   | 4  3 | 2  1  0 |
	 *|      0    | DMA | TOF | TOIE | CPWMS | CMOD |   PS    |
	 *|			  |     | w1c |
	 */
	TPM1_REG->sc = 	(0 << 5)	|			// CPWMS=TPM2_SC[5]=0 (modo de contagem crescente)
					(0b01 << 3)	|           // CMOD=TPM2_SC[4:3]=0b01 (incrementa a cada pulso do LPTPM)
					(0b111 << 0);           		// PS=TPM2_SC[2:0]=0b101 (Fator Prescale = 32)

	TPM2_REG->sc = 	(0 << 5)	|			// CPWMS=TPM2_SC[5]=0 (modo de contagem crescente)
					(0b01 << 3)	|           // CMOD=TPM2_SC[4:3]=0b01 (incrementa a cada pulso do LPTPM)
					(0b111 << 0);           		// PS=TPM2_SC[2:0]=0b101 (Fator Prescale = 32)

	/* Desativar o PWM no modulo TPM1 canal 0 --> PTB0 */
	TPM1_REG->CONTROLS[0].CnSC = (0 << 5) | // MSB =TPM2_C0SC[5]=0
								 (0 << 4) | // MSA =TPM2_C0SC[4]=0
								 (0 << 3) | // ELSB=TPM2_C0SC[3]=0
								 (0 << 2);  // ELSA=TPM2_C0SC[2]=0

	/* Ativar o PWM no modulo TPM1 canal 0 --> PTB0 */
	TPM1_REG->CONTROLS[0].CnSC = (1 << 5) | // MSB =TPM2_C0SC[5]=0
								 (0 << 4) | // MSA =TPM2_C0SC[4]=0
								 (1 << 3) | // ELSB=TPM2_C0SC[3]=0
								 (0 << 2);  // ELSA=TPM2_C0SC[2]=0

	/* Desativar o PWM no modulo TPM1 canal 1 --> PTB1 */
	TPM2_REG->CONTROLS[0].CnSC = (0 << 5) | // MSB =TPM2_C0SC[5]=0
								 (0 << 4) | // MSA =TPM2_C0SC[4]=0
								 (0 << 3) | // ELSB=TPM2_C0SC[3]=0
								 (0 << 2);  // ELSA=TPM2_C0SC[2]=0

	/* Ativar o PWM no modulo TPM1 canal 1 --> PTB1 */
	TPM2_REG->CONTROLS[0].CnSC = (1 << 5) | // MSB =TPM2_C0SC[5]=0
								 (0 << 4) | // MSA =TPM2_C0SC[4]=0
								 (1 << 3) | // ELSB=TPM2_C0SC[3]=0
								 (0 << 2);  // ELSA=TPM2_C0SC[2]=0
	

}
/*
 * @brief   Application entry point.
 */
int main(void) {
	//SystemCoreClockUpdate();
	InitTPMPWM();
	//TPM1_REG->CONTROLS[0].CnV = 0x0000;  	// 0% duty cycle

	TPM1_REG->CONTROLS[0].CnV = 0x0148;
	TPM2_REG->CONTROLS[0].CnV = 0x0A3D;
		// 0x0666 = 1638 (metade de 3276) --> 50% duty cycle
	//TPM1_REG->CONTROLS[0].CnV = 0x0EA6;  	// 0x0EA6 = 3750 (metade de 7500) --> 50% duty cycle
	//TPM1_REG->CONTROLS[0].CnV = 0x0753;  	// 0x0EA6

	while(1){

	}


    return 0 ;
}

