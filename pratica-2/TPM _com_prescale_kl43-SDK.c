#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL43Z4.h"
#include "fsl_debug_console.h"
#include "fsl_tpm.h"
#include "fsl_port.h"

void TPM0_IRQHandler(void){
	if((TPM_GetStatusFlags(TPM0) & kTPM_TimeOverflowFlag) != 0)
		GPIO_PortToggle(GPIOE, 1 << 31);
	TPM_ClearStatusFlags(TPM0, kTPM_TimeOverflowFlag);
}

volatile uint8_t updatedDutycycle = 10U;

int main(void) {
	tpm_config_t tpm0_config = {0};
	gpio_pin_config_t gpioLed = {
					kGPIO_DigitalOutput,
					1
	};

	CLOCK_EnableClock(kCLOCK_PortE);
	PORT_SetPinMux(PORTE, 31, kPORT_MuxAsGpio);
	GPIO_PinInit(GPIOE, 31, &gpioLed);

	TPM_GetDefaultConfig(&tpm0_config);
	tpm0_config.prescale = kTPM_Prescale_Divide_128;
	TPM_Init(TPM0, &tpm0_config);
	
	CLOCK_SetTpmClock(3);
    	TPM_SetTimerPeriod(TPM0, 2047);
    	
	TPM_EnableInterrupts(TPM0, kTPM_TimeOverflowInterruptEnable);
	__NVIC_EnableIRQ(TPM0_IRQn);
	TPM_StartTimer(TPM0, kTPM_SystemClock);


	while(1){

	}

	return 0 ;
}
