#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL43Z4.h"
#include "fsl_debug_console.h"

typedef struct{
	uint32_t PCR[32];
}PORTRegs_t;

#define PORT_B ((PORTRegs_t *)0x4004A000)
#define PORT_D ((PORTRegs_t *) 0x4004C000)
#define PORT_E ((PORTRegs_t *) 0x4004D000)

typedef struct{
	uint32_t PDOR;
	uint32_t PSOR;
	uint32_t PCOR;
	uint32_t PTOR;
	uint32_t PDIR;
	uint32_t PDDR;
}GPIORegs_t;

#define GPIO_B ((GPIORegs_t *)0x400FF040)
#define GPIO_D ((GPIORegs_t *) 0x400FF0C0)
#define GPIO_E ((GPIORegs_t *) 0x400FF100)

typedef struct {
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
}NVIC_Regs_t;

#define NVIC_REG ((NVIC_Regs_t *)0xE000E100)

typedef struct{
	uint32_t sc;
	uint32_t cnt;
	uint32_t mod;
}TPMR_Regs_t;

#define TPM_REG ((TPMR_Regs_t *) 0x40038000)

typedef struct{
	uint32_t status;
	uint32_t conf;
}TPM_CONF_Regs_t;

#define TPM_CONF_REG ((TPM_CONF_Regs_t *) 0x40038050)

typedef struct{
	uint32_t sopt2;
}SOPT2R_Regs_t;

#define SOPT2_REG ((SOPT2R_Regs_t *) 0x40048004)

int red = 0;
int green = 0;

void TPM0_IRQHandler(void){
	red++;
	green++;
	if(green == 2){
		GPIO_D->PTOR = (1 << 5);
		green = 0;
		TPM_REG->sc = TPM_REG->sc | (1 << 7);
		TPM_REG->cnt = 0x0000; // resetar
	}
	if(red == 6){
		GPIO_E->PTOR = (1 << 31);
		red = 0;
		TPM_REG->sc = TPM_REG->sc | (1 << 7);
		TPM_REG->cnt = 0x0000; // resetar
	}

}

/*
 * @brief   Application entry point.
 */
int main(void) {

		SOPT2_REG->sopt2 = (1 << 24) | (1 << 25);// MCGIRCLK clock é de 8Mhz
		
		SIM->SCGC6 = (1 << 24) | (1 << 25);
		MCG->SC |= (1 << 1) | (1 << 2);
		
        TPM_REG->sc |= (1 << 6);//ativando a interrupção para TOF
		
		TPM_REG->mod = 0x1E85; //valor para 1s;

		TPM_REG->cnt = 0x0000; // resetar

		TPM_REG->sc |= 0x7;//ativado o prescaler para 128
		// mod = (t/(1/f)) - 1
		// mod = (8/(1/(1Mhz/128))) - 1
		// t = 1/(8Mhz/128) = 16us
		TPM_REG->sc |= (1 << 3);//incrementa a cada pulso do LPTPM

	/*****************************************************************************/

	SIM->SCGC5 = (1 << 12) | (1 << 13); //ativar clock porta E

	/*****************************************************************************/

	//Ativando as portas
	PORT_E->PCR[31] = (1 << 8);
	PORT_D->PCR[5] = (1 << 8);
	//Configurando os GPIOs como saída
	GPIO_E->PDDR = (1 << 31);
	GPIO_D->PDDR = (1 << 5);

	/*****************************************************************************/

	//NVIC_EnableIRQ(TPM0_IRQn);
	NVIC_REG->iser[0] = (1 << 17);

	while(1){
	}
	
	
    return 0 ;
}

