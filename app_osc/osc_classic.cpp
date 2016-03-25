/**
* @brief 
* @file osc_classic.cpp
* @author J.H. 
* @date 2016-03-07
*/

/* module header */
#include "osc_classic.h"

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
static DDS dds;

static xTimerHandle xUpdateTimer = NULL;
static xTimerHandle xLogTimer = NULL;

static uint32_t s_dc = UINT32_MAX / 2;	// duty cycle for the output
static volatile uint32_t s_phase = 0;	// phase of the output
static uint16_t s_waveShaper = 2<<7;

static volatile bool s_rst = 0;

static knob_map_t s_map_k1_att[] = {
	{ ADC_KNOB_FROM,	0},
	{ ADC_KNOB_TO,		1500 }
};
static knob_map_t s_map_k2_att[] = {
	{ ADC_KNOB_FROM,	0},
	{ ADC_KNOB_TO,		UINT16_MAX }
};
static knob_map_t s_map_ws[] = {
	{ ADC_KNOB_FROM,											2<<8	},
	{ ADC_KNOB_TO, 												2<<12	}
};

static void updateCB( xTimerHandle xTimer );

static void cv1gateCB(bool state)
{
	if(state)s_rst = 1;
}

static void switchEventCB(uint8_t sw)
{
	switch(sw) {
		case 0: dds.setType(DDS::SINE);	break; 	
		case 1: dds.setType(DDS::SQUARE);	break; 	
		case 2: dds.setType(DDS::SAW);	break; 	
		default: break;
	}
}


static int32_t f = 0;
static void updateCB( xTimerHandle xTimer )
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	
	// CV 3 - waveshaper

	if(hw->cv.cv3 > CV_UNPLUGGED_VAL) {
		s_waveShaper = map_value(hw->cv.cv3, s_map_ws, 2);
		s_dc = hw->cv.cv4 << 20;
	} else {
		s_waveShaper = map_value(hw->knob.k4, s_map_ws, 2);
		s_dc = hw->knob.k4 << 20;
	}


	// actual frequency consists of sum of these
	// knob1 - main frequency
	// knob2 - fine frequency
	// cv2	 - v/oct 
	// cv4	 - fm input attentuated by a knob2
	int16_t f_fm = 0;
	int16_t f_cv = 0;
	if(hw->cv.cv2 > CV_UNPLUGGED_VAL) {
		int32_t cvolts =  TeensyHW::cv2volts(TeensyHW::hw_t::KC_CV2, hw->cv.cv2);
		f_cv = (cvolts * 1000) >> 16;
	}

	// Knob1 - pitch of the synth	
	int32_t f_k1, f_k2 =0; 
	f_k1 = map_value_exp(hw->knob.k1 << 4);
	f_k1 = (f_k1 * 12000) >> 16;
//    f_k1 = map_value_exp(hw->knob.k1 << 4);

	// CV4 - fm modulation, attentuated by knob2
	if(hw->cv.cv4 > CV_UNPLUGGED_VAL) {
		int32_t cvolts =  TeensyHW::cv2volts(TeensyHW::hw_t::KC_CV4, hw->cv.cv4);
		f_fm = ((cvolts >> 8) * (lut_exponential[(hw->knob.k2 - ADC_KNOB_FROM) >> 4])) >> 16;
	} else {
		// if CV4 is not active, knob2 does finetune
		 f_k2 = (map_value(hw->knob.k2, s_map_k2_att, 2) * f_k1) >> 16;
	}

	f =  f_k1 + f_k2 + f_cv + f_fm;
//    LOG_PRINT(Log::LOG_DEBUG, "f_cv=%d f_fm=%d f_k1=%d f=%d",f_cv, f_fm, f_k1, f);
//     set up the frequency for next cycle
	dds.m_inc = min(abs(f), 20000) * (UINT32_MAX / DDS_SAMPLE_RATE); 
	dds.m_backward = (f<0);
}

static void logCB(xTimerHandle xTimer)
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	int32_t cvolts =  TeensyHW::cv2volts(TeensyHW::hw_t::KC_CV2, hw->cv.cv2);
//    int16_t f=(cvolts*1000) >> 16;
//    LOG_PRINT(Log::LOG_DEBUG, "%cf=%d", dds.m_backward?'-':'+',dds.m_inc / (UINT32_MAX/ DDS_SAMPLE_RATE));
//    LOG_PRINT(Log::LOG_DEBUG, "k1=%d -> %d", (hw->knob.k1) << 4, map_value_exp((hw->knob.k1) << 4));
//    LOG_PRINT(Log::LOG_DEBUG, "k2=%c%d.%d f=%d", (cvolts<0) ? '-' : '+', fp2int(cvolts, 16), frac2int(cvolts, 16), f);
}

namespace OscClassic {
int16_t update() {
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	if(s_rst) { dds.m_acc = 0; s_rst = 0; }
	int16_t xs = ~dds.next();					// get next sample

	static uint8_t ps = 0;
	static uint8_t st = (dds.m_acc + s_phase) > s_dc;	// should we trigger the CV output
	if(ps != st) { 
		PIN_TOGGLE(CV_OUT_PIN)
			ps = st; }
	return  signed_saturate_rshift(xs*s_waveShaper, 16, 9);		
}

void setup() {
	dds.setFrequency((uint16_t)1500);
	dds.setType(DDS::SINE);
	xUpdateTimer = xTimerCreate("UpdateCB", 1, pdTRUE, 	( void * ) 0, updateCB);
	xLogTimer = xTimerCreate("logt", 100, pdTRUE, 	( void * ) 0, logCB);
}

void suspend()
{
	xTimerStop(xUpdateTimer, portMAX_DELAY);
	xTimerStop(xLogTimer, portMAX_DELAY);
}
void resume()
{
	xTimerStart(xUpdateTimer, portMAX_DELAY);
	xTimerStart(xLogTimer, portMAX_DELAY);
	TeensyHW::setSwitchEventCB(switchEventCB);
	TeensyHW::setGateCallback(TeensyHW::hw_t::KnobChannel::KC_CV1, cv1gateCB);
}
}
