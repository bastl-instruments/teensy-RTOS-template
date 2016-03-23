/**
* @brief 
* @file osc_harmonic.cpp
* @author J.H. 
* @date 2016-03-22
*/

/* module header */
#include "osc_harmonic.h"

/* system includes C */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* system includes C++ */


/* local includes */
#include "app_cfg.h"
#include "src/defines.h"
#include "src/hw.h"
#include "audio/utils.h"
#include "audio/utility/dspinst.h"
#include "src/valmap.h"
#include "src/macros.h"
#include "src/logger.h"

#include "src/fixedpoint.h"

#include "src/resources.h"

static DDS s_sub;
static DDS s_osc;
static DDS s_mod1;
static DDS s_mod2;
static xTimerHandle xUpdateTimer = NULL;

static knob_map_t s_map_k2_att[] = {
	{ ADC_KNOB_FROM,	0},
	{ ADC_KNOB_TO,		UINT16_MAX }
};

static void switchEventCB(uint8_t sw)
{
	switch(sw) {
		case 0: s_osc.setType(DDS::SINE_HIRES);	break; 	
		case 1: s_osc.setType(DDS::SQUARE);	break; 	
		case 2: s_osc.setType(DDS::SAW);	break; 	
		default: break;
	}
}

namespace OscHarmonic {
int16_t update() {
	int16_t ret = 0;
	ret = ~s_osc.next() + ~s_sub.next();
	s_osc.m_inc = (((s_mod1.next()+s_mod2.next()) * 20000) >> 16) * (UINT32_MAX / DDS_SAMPLE_RATE); 
	s_sub.m_inc = (((s_mod1.next()+s_mod2.next()) * 200) >> 16) * (UINT32_MAX / DDS_SAMPLE_RATE); 
	return ret;
}

static void updateCB( xTimerHandle xTimer )
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	int16_t f_k1, f_k2 = 0, f_fm = 0;
	int32_t f, cvolts;

	f_k1 = lut_exponential[max(0,hw->knob.k1 - ADC_KNOB_FROM) >> 4];
	f_k1 = (f_k1 * 1000) >> 16;


	// CV4 - fm modulation, attentuated by knob2
	if(hw->cv.cv4 > CV_UNPLUGGED_VAL) {
		cvolts =  TeensyHW::cv2volts(TeensyHW::hw_t::KC_CV4, hw->cv.cv4);
		f_fm = ((cvolts >> 8) * (lut_exponential[(hw->knob.k2 - ADC_KNOB_FROM) >> 4])) >> 16;
	} else {
		// if CV4 is not active, knob2 does finetune
		 f_k2 = (map_value(hw->knob.k2, s_map_k2_att, 2) * f_k1) >> 12;
	}
	f = min(abs(f_k1 + f_k2 + f_fm), 20000);
//    f= 500;
	
	if(hw->cv.cv1 > CV_UNPLUGGED_VAL) {
		cvolts =  TeensyHW::cv2volts(TeensyHW::hw_t::KC_CV1, hw->cv.cv1);
		s_mod1.m_mag = (cvolts * 1000) >> 16;
	} else {
		s_mod1.m_mag = (lut_exponential[(hw->knob.k3 - ADC_KNOB_FROM) >> 4]) >> 3;
	}
	if(hw->cv.cv3 > CV_UNPLUGGED_VAL) {
		cvolts =  TeensyHW::cv2volts(TeensyHW::hw_t::KC_CV2, hw->cv.cv2);
		s_mod2.m_mag = (cvolts * 1000) >> 16;
	} else {
		s_mod2.m_mag = (lut_exponential[(hw->knob.k4 - ADC_KNOB_FROM) >> 4]) >> 3;
	}
	s_mod1.m_inc = (f * f)  * (UINT32_MAX / DDS_SAMPLE_RATE);
	s_mod2.m_inc = (f)  * (UINT32_MAX / DDS_SAMPLE_RATE);
}
void setup() {
	s_osc.setFrequency((uint16_t)(0));
	s_osc.setType(DDS::SINE_HIRES);
	s_mod1.setFrequency((uint16_t)(0));
	s_mod1.setType(DDS::SINE_HIRES);
	s_mod2.setFrequency((uint16_t)(0));
	s_mod2.setType(DDS::SINE_HIRES);
	s_sub.m_mag = 1<<12;
	s_osc.m_mag = (1<<16) - s_sub.m_mag;
	xUpdateTimer = xTimerCreate("UpdateCB", 1, pdTRUE, 	( void * ) 0, updateCB);
	resume();
}

void suspend()
{
	xTimerStop(xUpdateTimer, portMAX_DELAY);
}
void resume()
{
	xTimerStart(xUpdateTimer, portMAX_DELAY);
	TeensyHW::setSwitchEventCB(switchEventCB);
}
}
