/**
* @brief 
* @file osc_quadro.cpp
* @author J.H. 
* @date 2016-03-18
*/

/* module header */
#include "osc_quadro.h"

/* system includes C */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* system includes C++ */


/* local includes */
#include "audio/utils.h"
#include "audio/utility/dspinst.h"
#include "src/hw.h"
#include "src/defines.h"

#define NUM_OSC	4
static DDS s_dds[NUM_OSC];
static xTimerHandle xUpdateTimer = NULL;
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
	
	int16_t f_cv;
	if(hw->cv.cv1 > CV_UNPLUGGED_VAL) {
		f_cv = (TeensyHW::cv2volts(TeensyHW::hw_t::KC_CV1, hw->cv.cv1) * 1000) >> 16;
	} else f_cv = 0;
	s_dds[0].m_inc = (hw->knob.k1 + f_cv) * (UINT32_MAX / DDS_SAMPLE_RATE); 
	s_dds[0].m_backward = (hw->knob.k1 + f_cv) < 0;

	if(hw->cv.cv2 > CV_UNPLUGGED_VAL) {
		f_cv = (TeensyHW::cv2volts(TeensyHW::hw_t::KC_CV2, hw->cv.cv2) * 1000) >> 16;
	} else f_cv = 0;
	s_dds[1].m_inc = (hw->knob.k2 + f_cv) * (UINT32_MAX / DDS_SAMPLE_RATE); 
	s_dds[1].m_backward = (hw->knob.k2 + f_cv) < 0;

	if(hw->cv.cv3 > CV_UNPLUGGED_VAL) {
		f_cv = (TeensyHW::cv2volts(TeensyHW::hw_t::KC_CV3, hw->cv.cv3) * 1000) >> 16;
	} else f_cv = 0;
	s_dds[2].m_inc = (hw->knob.k3 + f_cv) * (UINT32_MAX / DDS_SAMPLE_RATE); 
	s_dds[3].m_backward = (hw->knob.k3 + f_cv) < 0;

	if(hw->cv.cv4 > CV_UNPLUGGED_VAL) {
		f_cv = (TeensyHW::cv2volts(TeensyHW::hw_t::KC_CV4, hw->cv.cv4) * 1000) >> 16;
	} else f_cv = 0;
	s_dds[3].m_inc = (hw->knob.k4 + f_cv) * (UINT32_MAX / DDS_SAMPLE_RATE); 
	s_dds[3].m_backward = (hw->knob.k4 + f_cv) < 0;
}

namespace OscQuadro {
int16_t update() {
	int16_t ret = 0;
	for(auto&& dds : s_dds) {
		ret += ~dds.next();
	}
	return  ret;
}

void setup() {
	uint8_t i = 0;
	for(auto&& dds : s_dds) {
		dds.setFrequency((uint16_t)(1500 + i*131));
		dds.setType(DDS::SINE_HIRES);
		dds.setAmplitude(1.0 / NUM_OSC);
	}
	xUpdateTimer = xTimerCreate("UpdateCB", 1, pdTRUE, 	( void * ) 0, updateCB);
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
