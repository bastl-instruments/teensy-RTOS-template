/**
 * Binky test
 */

//this file gives us the pinMode and digitalWrite defines that are part of the
//ardiuno compatibiltiy stuff. If we really wanted to, we could go totally
//hardcore and write our own.
#include <avr_emulation.h>

//this is found in our local include directory
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "random.h"
#include "portManipulations.h"

#include "hw.h"
#include "ui.h"
#include <stdio.h>

#include "lcd1602.h"

#include "src/compat.h"

#include "src/LCDTask.h"
#include "src/TaskADC.h"
#include "src/logger.h"
#include "src/TaskError.h"

#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define UARTLog_Task_Priority				( tskIDLE_PRIORITY + 2 )
static xTimerHandle xLEDTimer = NULL;
static xTimerHandle xButtonTimer = NULL;
static xTimerHandle xUITimer = NULL;
#define mainDONT_BLOCK	(0)

#include "dac.h"

extern "C" {
void  fault_isr (void) {
	static uint8_t toggle = 0;
	while(1) {
		toggle = !toggle;
		TeensyHW::setLed(TeensyHW::hw_t::LED_PCB, toggle);
		TeensyHW::setLed(TeensyHW::hw_t::LED_1, toggle);
		TeensyHW::setLed(TeensyHW::hw_t::LED_2,  toggle);
		TeensyHW::setLed(TeensyHW::hw_t::LED_3,  toggle);
		TeensyHW::setLed(TeensyHW::hw_t::LED_4,  toggle);
		TeensyHW::setLed(TeensyHW::hw_t::LED_A,  toggle);
		for (int n=0; n<1000000; n++)  ;
	}
}


}

static void prvLEDToggleTask(void *pvParameters)
{
	static int toggle = 0;
	while(1)
		{
				toggle = (toggle) ? 0 : 1;
				TeensyHW::setLed(TeensyHW::hw_t::LED_PCB, toggle);
				vTaskDelay(1000);
		}
}


static void prvLEDTimerCB( xTimerHandle xTimer )
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
//    Tasks::LCDTask::updatef("%02x %d%d %02x", GPIOC_PDIR, hw->button >> 1, hw->button & 1, hw->button_i);
//    Tasks::LCDTask::notify();

//    Log::print(Log::LOG_ERROR, "[%08ld] %02x: %d%d %02x", millis()/1000, GPIOC_PDIR, hw->button >> 1, hw->button & 1, hw->button_i);
	LOG_PRINT(Log::LOG_ERROR, "%02x: %d%d %02x", GPIOC_PDIR, hw->button >> 1, hw->button & 1, hw->button_i);

}


#include "macros.h"



extern "C" {
	// must be extern C to link it properly
int blinky()
{

	TeensyHW::init();

	// enable fault handler for errors
#define SCB_SHCSR_USGFAULTENA (uint32_t)1<<18
#define SCB_SHCSR_BUSFAULTENA (uint32_t)1<<17
#define SCB_SHCSR_MEMFAULTENA (uint32_t)1<<16
	SCB_SHCSR |= SCB_SHCSR_BUSFAULTENA | SCB_SHCSR_USGFAULTENA | SCB_SHCSR_MEMFAULTENA;	

DAC::run();

	xTaskCreate( prvLEDToggleTask, "Rx", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, NULL );
	
	Tasks::LCDTask::create();
//    Tasks::Error::create();
	Tasks::ADC::create();


 xLEDTimer = xTimerCreate( 	"LEDTimer", 					//|+ A text name, purely to help debugging. +|
							 ( 1000 ),	//|+ The timer period, in this case 5000ms (5s). +|
							 pdTRUE,						//|+ This is a one shot timer, so xAutoReload is set to pdFALSE. +|
							 ( void * ) 0,					//|+ The ID is not used, so can be set to anything. +|
							 prvLEDTimerCB			//	|+ The callback function that switches the LED off. +|
							 );

// xButtonTimer = xTimerCreate( 	"updateButtons", 					//|+ A text name, purely to help debugging. +|
//                             ( 5 ),	//|+ The timer period, in this case 5000ms (5s). +|
//                             pdTRUE,						//|+ This is a one shot timer, so xAutoReload is set to pdFALSE. +|
//                             ( void * ) 0,					//|+ The ID is not used, so can be set to anything. +|
//                             prvButtonTimerCB			//	|+ The callback function that switches the LED off. +|
//                             );

// xUITimer = xTimerCreate( 	"UITimer", 					//|+ A text name, purely to help debugging. +|
//                             ( 5 ),	//|+ The timer period, in this case 5000ms (5s). +|
//                             pdTRUE,						//|+ This is a one shot timer, so xAutoReload is set to pdFALSE. +|
//                             ( void * ) 0,					//|+ The ID is not used, so can be set to anything. +|
//                             prvUITimerCB			//	|+ The callback function that switches the LED off. +|
//                             );
	xTimerStart( xLEDTimer, mainDONT_BLOCK );
//    xTimerStart( xButtonTimer, mainDONT_BLOCK );
//    xTimerStart( xUITimer, mainDONT_BLOCK );
	vTaskStartScheduler();



    return 0;
}


}
