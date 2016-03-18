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
static void updateCB( xTimerHandle xTimer )
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	
	// CV 3 - waveshaper

	if(hw->cv.cv1 > CV_UNPLUGGED_VAL) {
//        s_dds[0].m_inc = hw->.k1 * (UINT32_MAX / DDS_SAMPLE_RATE); 
	} else {
		s_dds[0].m_inc = hw->knob.k1 * (UINT32_MAX / DDS_SAMPLE_RATE); 
	}
	if(hw->cv.cv2 > CV_UNPLUGGED_VAL) {
//        s_dds[0].m_inc = hw->.k1 * (UINT32_MAX / DDS_SAMPLE_RATE); 
	} else {
		s_dds[1].m_inc = hw->knob.k2 * (UINT32_MAX / DDS_SAMPLE_RATE); 
	}
	if(hw->cv.cv3 > CV_UNPLUGGED_VAL) {
//        s_dds[0].m_inc = hw->.k1 * (UINT32_MAX / DDS_SAMPLE_RATE); 
	} else {
		s_dds[2].m_inc = hw->knob.k3 * (UINT32_MAX / DDS_SAMPLE_RATE); 
	}
	if(hw->cv.cv4 > CV_UNPLUGGED_VAL) {
//        s_dds[0].m_inc = hw->.k1 * (UINT32_MAX / DDS_SAMPLE_RATE); 
	} else {
		s_dds[3].m_inc = hw->knob.k4 * (UINT32_MAX / DDS_SAMPLE_RATE); 
	}
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
	xTimerStart( xUpdateTimer, 0 );
}

void suspend()
{
	xTimerStop(xUpdateTimer, portMAX_DELAY);
}

void resume()
{
	xTimerStart(xUpdateTimer, portMAX_DELAY);
}
}
