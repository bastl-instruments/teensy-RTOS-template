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

static DDS s_osc;
static DDS s_mod;
static xTimerHandle xUpdateTimer = NULL;

namespace OscHarmonic {
int16_t update() {
	int16_t ret = 0;
	ret = ~s_osc.next();
	s_osc.m_inc = ((s_mod.next() * 40000) >> 16) * (UINT32_MAX / DDS_SAMPLE_RATE); 
	return ret;
}

static void updateCB( xTimerHandle xTimer )
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	int16_t f_k1;
	int32_t f;

	f_k1 = lut_exponential[(hw->knob.k1 - ADC_KNOB_FROM) >> 4];
	f_k1 = (f_k1 * 20000) >> 16;
	
	f = f_k1;
	s_osc.m_inc = min(abs(f), 20000) * (UINT32_MAX / DDS_SAMPLE_RATE); 
	s_osc.m_backward = (f<0);

	s_mod.m_inc = ((lut_exponential[(hw->knob.k2 - ADC_KNOB_FROM) >> 4] * f) >> 16) * (UINT32_MAX / DDS_SAMPLE_RATE);
}
void setup() {
	s_osc.setFrequency((uint16_t)(0));
	s_osc.setType(DDS::SINE_HIRES);
	s_mod.setFrequency((uint16_t)(0));
	s_mod.setType(DDS::SINE_HIRES);
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
//    TeensyHW::setSwitchEventCB(switchEventCB);
}
}
