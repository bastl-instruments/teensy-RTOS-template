/**
* @brief 
* @file app.cpp
* @author J.H. 
* @date 2016-01-20
*/

/* module header */
#include "app.h"

/* system includes C */

/* system includes C++ */


/* local includes */

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "audio/synth_add.h"
#include "audio/output_dac.h"
#include "src/macros.h"
#include "src/hw.h"
#include "src/logger.h"

static AudioSynthAdditive sine;
static AudioOutputAnalog dac;
static AudioConnection          patchCord1(sine, dac);

namespace App {

static xTimerHandle xUpdateTimer = NULL;

static void buttonEventCB(TeensyHW::hw_t::ButtonState s)
{
	if(s == TeensyHW::hw_t::BUTTON_PRESSED) {
		TeensyHW::setLed(TeensyHW::hw_t::LED_3, 1);
	} else if (s == TeensyHW::hw_t::BUTTON_RELEASED) {
		TeensyHW::setLed(TeensyHW::hw_t::LED_3, 0);
	}
}


static void updateCB( xTimerHandle xTimer )
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	static float last = 0;
	float n = hw->knob.k1 / 8;
	int tmp;
	if(fabs(last - n) > 100) {
		LOG_PRINT(Log::LOG_DEBUG, "setting f to %x", hw->knob.k1/8);
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

void setup()
{
	AudioMemory(12);
}

void run()
{
	TeensyHW::setButtonEventCB(buttonEventCB);

	xUpdateTimer = xTimerCreate("UpdateCB", 1, pdTRUE, 	( void * ) 0, updateCB);
	xTimerStart( xUpdateTimer, 0 );
}

}
