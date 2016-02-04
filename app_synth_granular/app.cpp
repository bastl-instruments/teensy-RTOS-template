/**
* @brief 
* @file app.cpp
* @author J.H. 
* @date 2016-01-25
*/

/* module header */
#include "src/app_tmpl.h"

/* system includes C */

/* system includes C++ */


/* local includes */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "audio/synth_granular.h"
#include "audio/output_dac.h"
#include "src/hw.h"
#include "src/macros.h"

#define TRIGGER_DLY_MIN	100
static AudioSynthGranular synth;
static AudioOutputAnalog dac;
static AudioConnection   patchCord1(synth, dac);
namespace App {
static xTimerHandle xUpdateTimer = NULL;
static uint32_t s_last_trigger = 0;
static void updateCB( xTimerHandle xTimer )
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	uint32_t t = millis();

	if(hw->cvAct.cv1) {
		synth.setFrequency(hw->cv.cv1 / 20);
	} else {
		synth.setFrequency(hw->knob.k1 / 20);
	}
	if(hw->cvAct.cv2) {
		synth.setModFrequency(hw->cv.cv2 / 20);
	} else {
		synth.setModFrequency(hw->knob.k2 / 200);
	}
	if(hw->cvAct.cv3) {
	} else {
	}
	if(hw->cvAct.cv4) {
		if((t - s_last_trigger) > TRIGGER_DLY_MIN) {
			synth.trigger();
			s_last_trigger = t;
		}
		TeensyHW::setLed(TeensyHW::hw_t::LED_A, 1);
	} else {
		TeensyHW::setLed(TeensyHW::hw_t::LED_A, 0);
		synth.setRepeat(hw->knob.k4 / 10240);
	}

}


static void prvGranuloTask(void *pvParameters)
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	while(1) {
		vTaskDelay(hw->knob.k3 / 1000);
		synth.trigger();
	}
}

static void buttonEventCB(TeensyHW::hw_t::ButtonState s)
{
	if(s == TeensyHW::hw_t::BUTTON_PRESSED) {
		TeensyHW::setLed(TeensyHW::hw_t::LED_3, 1);
		synth.trigger();
	} else if (s == TeensyHW::hw_t::BUTTON_RELEASED) {
		TeensyHW::setLed(TeensyHW::hw_t::LED_3, 0);
	}
}
void setup()
{
	AudioMemory(12);
	synth.setFrequency(1000);
	synth.setModFrequency(20);
	synth.setActiveLayers(10);
}
void run()
{
	TeensyHW::setButtonEventCB(buttonEventCB);
	xUpdateTimer = xTimerCreate("UpdateCB", 1, pdTRUE, 	( void * ) 0, updateCB);
	xTaskCreate( prvGranuloTask, "granulo", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL );
	xTimerStart( xUpdateTimer, 0 );
}
}
