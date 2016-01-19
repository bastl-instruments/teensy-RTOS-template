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
static xTimerHandle xUpdateTimer = NULL;
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

#include "synth_add.h"
	AudioSynthAdditive sine;
static void prvLEDToggleTask(void *pvParameters)
{
	static int toggle = 0;
	while(1)
		{
				toggle = (toggle) ? 0 : 1;
				TeensyHW::setLed(TeensyHW::hw_t::LED_PCB, toggle);
				vTaskDelay(500);
//                int16_t xx = (sine.x / 40) * 65536;
//                LOG_PRINT(Log::LOG_DEBUG, "lorenz: %d", xx);
		}
}

#include "macros.h"
#include "output_dac.h"
	AudioOutputAnalog dac;
	AudioConnection          patchCord1(sine, dac);

static void buttonEventCB(TeensyHW::hw_t::ButtonState s)
{
	if(s == TeensyHW::hw_t::BUTTON_PRESSED) {
		TeensyHW::setLed(TeensyHW::hw_t::LED_3, 1);
//        sine.trigger();
	} else if (s == TeensyHW::hw_t::BUTTON_RELEASED) {
		TeensyHW::setLed(TeensyHW::hw_t::LED_3, 0);
	}
}
static void prvUpdateCB( xTimerHandle xTimer )
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	static float last = 0;
	float n = hw->knob.k1 / 8;
	int tmp;
	if(fabs(last - n) > 100) {
		LOG_PRINT(Log::LOG_DEBUG, "setting f to %x", hw->knob.k1/8);
//        sine.frequency(n);
		last = n;
	}

	if(hw->cvAct.cv1) {
	} else {
		sine.frequency(0,hw->knob.k1 / 10);
		sine.frequency(1,hw->knob.k1 / 12);
		sine.frequency(2,hw->knob.k1 / 16);
		sine.frequency(3,hw->knob.k1 / 20);
	}
	if(hw->cvAct.cv2) {
	} else {
		sine.frequency(4,hw->knob.k2 / 2);
		sine.frequency(5,hw->knob.k2 / 4);
		sine.frequency(6,hw->knob.k2 / 8);
		sine.frequency(7,hw->knob.k2 / 16);
	}
//    TeensyHW::setLedBlink(TeensyHW::hw_t::LED_1, hw->knob.k2/8192);
	if(hw->cvAct.cv3) {
	} else {
		sine.frequency(8,hw->knob.k3 / 100);
		sine.frequency(9,hw->knob.k3 / 120);
		sine.frequency(10,hw->knob.k3 / 240);
		sine.frequency(11,hw->knob.k3 / 360);
	}
	if(hw->cvAct.cv4) {
	} else {
		sine.frequency(12,hw->knob.k4 / 70);
		sine.frequency(13,hw->knob.k4 / 80);
		sine.frequency(14,hw->knob.k4 / 90);
		sine.frequency(15,hw->knob.k4 / 100);
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
//    sine.frequency(546);
//    sine.amplitude(1);
	TeensyHW::setButtonEventCB(buttonEventCB);



	// enable fault handler for errors
#define SCB_SHCSR_USGFAULTENA (uint32_t)1<<18
#define SCB_SHCSR_BUSFAULTENA (uint32_t)1<<17
#define SCB_SHCSR_MEMFAULTENA (uint32_t)1<<16
	SCB_SHCSR |= SCB_SHCSR_BUSFAULTENA | SCB_SHCSR_USGFAULTENA | SCB_SHCSR_MEMFAULTENA;	




	xTaskCreate( prvLEDToggleTask, "Rx", configMINIMAL_STACK_SIZE*2, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, NULL );
//    
	Tasks::ADC::create();


	xUpdateTimer = xTimerCreate( 	"LEDTimer", 					//|+ A text name, purely to help debugging. +|
			( 1 ),	//|+ The timer period, in this case 5000ms (5s). +|
			pdTRUE,						//|+ This is a one shot timer, so xAutoReload is set to pdFALSE. +|
			( void * ) 0,					//|+ The ID is not used, so can be set to anything. +|
			prvUpdateCB			//	|+ The callback function that switches the LED off. +|
			);


	xTimerStart( xUpdateTimer, mainDONT_BLOCK );
	vTaskStartScheduler();


    return 0;
}


}
