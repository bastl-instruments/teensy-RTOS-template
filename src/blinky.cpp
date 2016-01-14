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

#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define UARTLog_Task_Priority				( tskIDLE_PRIORITY + 2 )
static xTimerHandle xLEDTimer = NULL;
#define mainDONT_BLOCK	(0)


extern "C" {
void  fault_isr (void) {
	static uint8_t toggle = 0;

	uint32_t* sp=0;
	// this is from "Definitive Guide to the Cortex M3" pg 423
	asm volatile ( "TST LR, #0x4\n\t"   // Test EXC_RETURN number in LR bit 2
			"ITE EQ\n\t"         // if zero (equal) then
			"MRSEQ %0, MSP\n\t"  //   Main Stack was used, put MSP in sp
			"MRSNE %0, PSP\n\t"  // else Process stack was used, put PSP in sp
			: "=r" (sp) : : "cc");

#define SCB_SHPR1_USGFAULTPRI *(volatile uint8_t *)0xE000ED20
#define SCB_SHPR1_BUSFAULTPRI *(volatile uint8_t *)0xE000ED19
#define SCB_SHPR1_MEMFAULTPRI *(volatile uint8_t *)0xE000ED18

	SCB_SHPR1_BUSFAULTPRI = (uint8_t)255;
	SCB_SHPR1_USGFAULTPRI = (uint8_t)255;
	SCB_SHPR1_MEMFAULTPRI = (uint8_t)255;


	while(1) {
		toggle = !toggle;
		TeensyHW::setLed(TeensyHW::hw_t::LED_PCB, toggle);
		TeensyHW::setLed(TeensyHW::hw_t::LED_1, toggle);
		TeensyHW::setLed(TeensyHW::hw_t::LED_2,  toggle);
		TeensyHW::setLed(TeensyHW::hw_t::LED_3,  toggle);
		TeensyHW::setLed(TeensyHW::hw_t::LED_4,  toggle);
		TeensyHW::setLed(TeensyHW::hw_t::LED_A,  toggle);
		LOG_PRINT(Log::LOG_FATAL, "crash: pc=0x%x lr=0x%x", sp[6], sp[5]);

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
				vTaskDelay(500);
//                LOG_PRINT(Log::LOG_DEBUG, "tick: %d", toggle);
		}
}

#include "macros.h"
#include "output_dac.h"
//#include "synth_hypnotoad.h"
#include "synth_sine.h"
	AudioOutputAnalog dac;
	AudioSynthWaveformSine sine;
	AudioConnection          patchCord1(sine, dac);

static void prvUpdateCB( xTimerHandle xTimer )
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	static float last = 0;
	float n = hw->knob.k1 / 8;
	if(last != n) {
		sine.frequency(n);
		last = n;
	}
	sine.finc(hw->knob.k3 / 6553.0);

	if(hw->cvAct.cv1) {
		sine.frequency(hw->cv.cv1 / 8);
	}
	if(hw->cvAct.cv2) {
		sine.nsines(hw->cv.cv2 / 20448);
	} else {
		sine.nsines(hw->knob.k2 / 2048);
	}
	if(hw->cvAct.cv3) {
		sine.finc(hw->cv.cv3 / 6553.0);
	}


}




extern "C" {
	// must be extern C to link it properly
int blinky()
{
	Log::init();
	TeensyHW::init();
	AudioMemory(12);
//    dac.analogReference(EXTERNAL);
	sine.frequency(546);
	sine.amplitude(1);



	// enable fault handler for errors
#define SCB_SHCSR_USGFAULTENA (uint32_t)1<<18
#define SCB_SHCSR_BUSFAULTENA (uint32_t)1<<17
#define SCB_SHCSR_MEMFAULTENA (uint32_t)1<<16
	SCB_SHCSR |= SCB_SHCSR_BUSFAULTENA | SCB_SHCSR_USGFAULTENA | SCB_SHCSR_MEMFAULTENA;	




	xTaskCreate( prvLEDToggleTask, "Rx", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, NULL );
//    
	Tasks::ADC::create();


	xLEDTimer = xTimerCreate( 	"LEDTimer", 					//|+ A text name, purely to help debugging. +|
			( 1 ),	//|+ The timer period, in this case 5000ms (5s). +|
			pdTRUE,						//|+ This is a one shot timer, so xAutoReload is set to pdFALSE. +|
			( void * ) 0,					//|+ The ID is not used, so can be set to anything. +|
			prvUpdateCB			//	|+ The callback function that switches the LED off. +|
			);


	xTimerStart( xLEDTimer, mainDONT_BLOCK );
	vTaskStartScheduler();


    return 0;
}


}
