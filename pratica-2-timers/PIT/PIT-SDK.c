#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL43Z4.h"
#include "fsl_debug_console.h"
#include "fsl_pit.h"
#include "fsl_gpio.h"
#include "fsl_port.h"

#define GET_SEC_COUNT(x) ((10485760 * x)-1)

void PIT_IRQHandler(void) {
	GPIO_PortToggle(GPIOE, (1<<31));
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
}


int main(void) {
	pit_config_t pitCfg = {0};

	gpio_pin_config_t gpioLed = {
			kGPIO_DigitalOutput,
			1
	};
	CLOCK_EnableClock(kCLOCK_PortE);
	PORT_SetPinMux(PORTE, 31, kPORT_MuxAsGpio);
	GPIO_PinInit(GPIOE, 31, &gpioLed);
	
	PIT_GetDefaultConfig(&pitCfg);
	PIT_Init(PIT, &pitCfg);
	
	PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, GET_SEC_COUNT(1));
	PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
	__NVIC_EnableIRQ(PIT_IRQn);
	PIT_StartTimer(PIT, kPIT_Chnl_0);

	while(1) {

	}
    return 0 ;
}
