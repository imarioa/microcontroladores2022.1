#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL43Z4.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fsl_lptmr.h"

void LPTMR0_IRQHandler(void) {
	GPIO_PortToggle(GPIOE, (1 << 31));
	LPTMR_ClearStatusFlags(LPTMR0, kLPTMR_TimerCompareFlag);
}


int main(void) {
	lptmr_config_t lptmrCfg = {0};

	gpio_pin_config_t gpioLed = {
				kGPIO_DigitalOutput,
				1
	};

	CLOCK_EnableClock(kCLOCK_PortE);
	PORT_SetPinMux(PORTE, 31, kPORT_MuxAsGpio);
	GPIO_PinInit(GPIOE, 31, &gpioLed);

	CLOCK_EnableClock(kCLOCK_Lptmr0);
	LPTMR_GetDefaultConfig(&lptmrCfg);
	lptmrCfg.bypassPrescaler = false;
	lptmrCfg.value = kLPTMR_Prescale_Glitch_5;
	LPTMR_Init(LPTMR0, &lptmrCfg);

	LPTMR_SetTimerPeriod(LPTMR0, 124);
	LPTMR_EnableInterrupts(LPTMR0, kLPTMR_TimerInterruptEnable);
	__NVIC_EnableIRQ(LPTMR0_IRQn);
	LPTMR_StartTimer(LPTMR0);

	while(1){
	}

    return 0 ;
}
