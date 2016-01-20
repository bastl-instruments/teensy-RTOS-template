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


#include "src/compat.h"

#include "src/TaskADC.h"
#include "src/logger.h"
#include "src/fault_isr.h"
#include "src/app_tmpl.h"

#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define UARTLog_Task_Priority				( tskIDLE_PRIORITY + 2 )
#define mainDONT_BLOCK	(0)

// this simply toggles PCB led every 500 ms, to show the OS is alive
static void prvLEDToggleTask(void *pvParameters)
{
	static int toggle = 0;
	while(1)
		{
				toggle = (toggle) ? 0 : 1;
				TeensyHW::setLed(TeensyHW::hw_t::LED_PCB, toggle);
				vTaskDelay(500);
		}
}

extern "C" {
	// must be extern C to link it properly
int blinky()
{
	// init hardware 
	Log::init();
	TeensyHW::init();
	init_fault_isr();

	// create basic tasks
	xTaskCreate( prvLEDToggleTask, "Rx", configMINIMAL_STACK_SIZE*2, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, NULL );
	Tasks::ADC::create();

	// this is the app-specific code that links controls to audio object parameters
	App::run();

	// run FreeRTOS
	vTaskStartScheduler();


    return 0;
}


}
