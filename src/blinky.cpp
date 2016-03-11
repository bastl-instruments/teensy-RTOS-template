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
#include "src/TaskCalibrate.h"
#include "src/logger.h"
#include "src/fault_isr.h"
#include "src/app_tmpl.h"

#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define mainDONT_BLOCK	(0)

// this simply toggles PCB led every 500 ms, to show the OS is alive
static void prvLEDToggleTask(void *pvParameters)
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	static int toggle = 0;
	while(1)
		{
				toggle = (toggle) ? 0 : 1;
				TeensyHW::setLed(TeensyHW::hw_t::LED_PCB, toggle);
				vTaskDelay(1000);
		}
}

static void prvInitTask(void *params)
{
	// hang until calibration task is done
	uint32_t val = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	//
	// run main app code

	App::run();

	while(1) vTaskDelete(xTaskGetCurrentTaskHandle());
}
#include "usb_dev.h"
extern "C" {
	// must be extern C to link it properly
int blinky()
{
	TaskHandle_t initTask_handle;
	// init hardware 
	Log::init();
	TeensyHW::init();
	init_fault_isr();

	// --  create basic tasks --
	xTaskCreate( prvLEDToggleTask, "Rx", configMINIMAL_STACK_SIZE*2, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, NULL ); // alive LED
	Tasks::ADC::create();	// continuous ADC conversion
	// initapp - but only after calibration is done
	xTaskCreate( prvInitTask, "init", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, &initTask_handle);	
	// create calibration task
	Tasks::Calibrate::create(initTask_handle);
	App::setup();

	// run FreeRTOS
	vTaskStartScheduler();


    return 0;
}


}
