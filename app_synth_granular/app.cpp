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

static AudioSynthGranular synth;
static AudioOutputAnalog dac;
static AudioConnection   patchCord1(synth, dac);
namespace App {
static xTimerHandle xUpdateTimer = NULL;
static void updateCB( xTimerHandle xTimer )
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();

	if(hw->cvAct.cv1) {
	} else {
		synth.setFrequency(hw->knob.k1 / 20.0);
	}
	if(hw->cvAct.cv2) {
	} else {
		synth.setModFrequency(hw->knob.k2 / 200.0);
	}
	if(hw->cvAct.cv3) {
	} else {
	}
	if(hw->cvAct.cv4) {
	} else {
	}

}


static void prvGranuloTask(void *pvParameters)
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	while(1) {
		TeensyHW::setLed(TeensyHW::hw_t::LED_4, 1);
		vTaskDelay(hw->knob.k3 / 1000);
		synth.trigger();
		TeensyHW::setLed(TeensyHW::hw_t::LED_4, 0);
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
}
void run()
{
	TeensyHW::setButtonEventCB(buttonEventCB);
	xUpdateTimer = xTimerCreate("UpdateCB", 1, pdTRUE, 	( void * ) 0, updateCB);
	xTaskCreate( prvGranuloTask, "granulo", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL );
	xTimerStart( xUpdateTimer, 0 );
}
}