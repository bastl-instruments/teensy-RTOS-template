/**
* @brief 
* @file app.cpp
* @author J.H. 
* @date 2016-01-20
*/

/* module header */
#include "src/app_tmpl.h"

/* system includes C */

/* system includes C++ */


/* local includes */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "audio/synth_kdrum.h"
#include "audio/output_dac.h"
#include "src/macros.h"
#include "src/hw.h"
#include "src/logger.h"

static AudioKDrum kick;
static AudioOutputAnalog dac;
static AudioConnection          patchCord1(kick, dac);

namespace App {

static xTimerHandle xUpdateTimer = NULL;

static void buttonEventCB(TeensyHW::hw_t::ButtonState s)
{
	if(s == TeensyHW::hw_t::BUTTON_PRESSED) {
		TeensyHW::setLed(TeensyHW::hw_t::LED_3, 1);
		kick.trigger();
	} else if (s == TeensyHW::hw_t::BUTTON_RELEASED) {
		TeensyHW::setLed(TeensyHW::hw_t::LED_3, 0);
	}
}


static void updateCB( xTimerHandle xTimer )
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();

	if(hw->cvAct.cv1) {
		kick.frequency(hw->cv.cv1 / 20.0);
	} else {
		kick.frequency(hw->knob.k1 / 20.0);
	}
	if(hw->cvAct.cv2) {
		kick.dec(hw->cv.cv2 / 32.0);
	} else {
		kick.dec(hw->knob.k2 /32.0);
	}
	if(hw->cvAct.cv3) {
		kick.dec(hw->cv.cv3 / 6553.0);
	} else {
		kick.finc(hw->knob.k3 / 6553.0);
	}
	if(hw->cvAct.cv4) {
		kick.trigger();
		TeensyHW::setLed(TeensyHW::hw_t::LED_A, 1);
	} else {
		TeensyHW::setLed(TeensyHW::hw_t::LED_A, 0);
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
