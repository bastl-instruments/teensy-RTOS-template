/**
* @brief 
* @file app.cpp
* @author J.H. 
* @date 2016-02-04
*/

/* module header */
#include "src/app_tmpl.h"

/* system includes C */

/* system includes C++ */


/* local includes */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "src/hw.h"
#include "src/logger.h"

#include "synth_braids.h"
#include "audio/output_dac.h"
static AudioSynthBraids synth;
static AudioOutputAnalog dac;
static AudioConnection          patchCord1(synth, dac);

namespace App {

static xTimerHandle xUpdateTimer = NULL;
static uint32_t s_last_trigger = 0;

static void buttonEventCB(TeensyHW::hw_t::ButtonState s)
{
//    static uint8_t shax = braids::MacroOscillatorShape::MACRO_OSC_SHAPE_DIGITAL_FILTER_LP;
	if(s == TeensyHW::hw_t::BUTTON_PRESSED) {
		TeensyHW::setLed(TeensyHW::hw_t::LED_3, 1);
		synth.trigger();
//        synth.setShape(shax);
//        shax++;
	} else if (s == TeensyHW::hw_t::BUTTON_RELEASED) {
		TeensyHW::setLed(TeensyHW::hw_t::LED_3, 0);
	}
}

#define TRIGGER_DLY_MIN	100

static void updateCB( xTimerHandle xTimer )
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	uint32_t t;

	if(hw->cvAct.cv1) {
		synth.setPitch(hw->cv.cv1 / 2);
	} else {
		synth.setPitch(hw->knob.k1 / 2);
	}
	if(hw->cvAct.cv2) {
		synth.setParam1(hw->cv.cv2 - 32768);
	} else {
		synth.setParam1(hw->knob.k2 - 32768);
	}
	if(hw->cvAct.cv3) {
		synth.setParam2(hw->cv.cv3 - 32768);
	} else {
		synth.setParam2(hw->knob.k3 - 32768);
	}
	if(hw->cvAct.cv4) {
		t = millis();
		if((t - s_last_trigger) > TRIGGER_DLY_MIN) {
			synth.trigger();
			s_last_trigger = t;
		}
	} else {
		const int16_t _div = (65536 / braids::MacroOscillatorShape::MACRO_OSC_SHAPE_DIGITAL_MODULATION);
		synth.setShape(hw->knob.k4 / _div);
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
