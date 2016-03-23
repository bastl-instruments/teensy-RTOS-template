/**
* @brief 
* @file osc_triple.cpp
* @author J.H. 
* @date 2016-03-18
*/

/* module header */
#include "osc_triple.h"

/* system includes C */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include <math.h>

/* system includes C++ */


/* local includes */
#include "src/valmap.h"
#include "audio/utils.h"
#include "audio/utility/dspinst.h"
#include "src/hw.h"
#include "src/defines.h"
#include "app_cfg.h"

// knob1 - pitch
// knob2 - fine tune / FM
// knob3 - detune
// knob4 - waveshaper
//
// CV1	 - CV A???
// CV2	 - V/OCT
// CV3	 - CV B???
// CV4	 - fm (att. by knob2)


#define NUM_OSC	3
static DDS s_dds[NUM_OSC];
static xTimerHandle xUpdateTimer = NULL;
static uint16_t s_waveShaper = 2<<7;

static knob_map_t s_map_freq[] = {
	{ ADC_KNOB_FROM,											0		},
	{ ADC_KNOB_TO, 												20000	}

};
static knob_map_t s_map_k2_att[] = {
	{ ADC_KNOB_FROM,	0},
	{ ADC_KNOB_TO,		UINT16_MAX }
};
static knob_map_t s_map_ws[] = {
	{ ADC_KNOB_FROM,											2<<8	},
	{ ADC_KNOB_TO, 												2<<12	}
};

static void switchEventCB(uint8_t sw)
{
	for(auto&& ds : s_dds) {
		switch(sw) {
			case 0: ds.setType(DDS::SINE_HIRES); break; 	
			case 1: ds.setType(DDS::SQUARE);	break; 	
			case 2: ds.setType(DDS::SAW);	break; 	
			default: break;
		}
	}
}
static void updateCB( xTimerHandle xTimer )
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	static int16_t f_cv2 = 0;
	static int16_t f_k1 = 0, f_k2 = 0;
	static int16_t f_fm = 0;
	static int32_t f = 0;
	int32_t cvolts;
	// CV2 - V/OCT
	f_cv2 = 0;
	if(hw->cv.cv2 > CV_UNPLUGGED_VAL) {
		cvolts =  TeensyHW::cv2volts(TeensyHW::hw_t::KC_CV2, hw->cv.cv2);
		f_cv2 = ((cvolts*1000) >> 16); 
	}
	// CV3 - waveshaper
	if(hw->cv.cv3 > CV_UNPLUGGED_VAL) {
		s_waveShaper = map_value(hw->cv.cv3, s_map_ws, 2);
	} else {
		s_waveShaper = map_value(hw->knob.k4, s_map_ws, 2);
	}
	// CV4 - fm
	if(hw->cv.cv4 > CV_UNPLUGGED_VAL) {
//         fm mod
		cvolts =  TeensyHW::cv2volts(TeensyHW::hw_t::KC_CV4, hw->cv.cv4);
		f_fm = ((cvolts >> 8) * map_value(hw->knob.k2, s_map_k2_att, 2)) >> 16;
	} else {
		// if CV4 is not active, knob2 does finetune
		 f_k2 = (map_value(hw->knob.k2, s_map_k2_att, 2) * f_k1) >> 16;
	}

	f_k1 = map_value(hw->knob.k1, s_map_freq, 4); // base frequency as set by pitch
	f = f_k1 + f_k2 + f_cv2 + f_fm;

	s_dds[0].m_inc = min(abs(f), 20000) * (UINT32_MAX / DDS_SAMPLE_RATE); 
	s_dds[0].m_backward = (f<0);

	static int16_t f_diff = f*hw->knob.k3 >> 16;
	s_dds[1].m_inc = (f+f_diff) * (UINT32_MAX / DDS_SAMPLE_RATE); 
	s_dds[1].m_backward = (f+f_diff) < 0;
	s_dds[2].m_inc = (f-f_diff) * (UINT32_MAX / DDS_SAMPLE_RATE); 
	s_dds[2].m_backward = (f-f_diff) < 0;
}
namespace OscTriple {
int16_t update() {
	int16_t ret = 0;
	for(auto&& dds : s_dds) {
		ret += ~dds.next();
	}
	return  signed_saturate_rshift(ret*s_waveShaper, 16, 9);		
}

void setup() {
	for(auto&& dds : s_dds) {
		dds.setFrequency((uint16_t)(0));
		dds.setType(DDS::SINE_HIRES);
		dds.setAmplitude(1.0 / NUM_OSC);
	}
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
