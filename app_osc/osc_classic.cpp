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

#include "src/conversion.h"
#include "src/fixedpoint.h"


static DDS dds;

static xTimerHandle xUpdateTimer = NULL;
static xTimerHandle xLogTimer = NULL;

static uint32_t s_dc = UINT32_MAX / 2;	// duty cycle for the output
static volatile uint32_t s_phase = 0;	// phase of the output
static uint16_t s_waveShaper = 2<<7;

static volatile bool s_rst = 0;

#define ADC_KNOB_FROM	259
#define ADC_KNOB_TO		3993

#define CV2_FROM		995
#define CV2_TO			3189


static knob_map_t s_map_freqF[] = {
	{ 0,											0		},
	{ UINT16_MAX, 									20000	}

};
static knob_map_t s_map_freq[] = {
	{ ADC_KNOB_FROM,											0		},
	{ ADC_KNOB_TO, 												20000	}

};
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
static knob_map_t s_map_cv2[] = {
	{ CV2_FROM,											0	},
	{ CV2_TO, 											14*12	}
};
static knob_map_t s_map_cv4[] = {
	{ CV2_FROM,											0	},
	{ CV2_TO, 											UINT16_MAX	}
};

static void updateCB( xTimerHandle xTimer );

static void cv1gateCB(bool state)
{
	if(state)s_rst = 1;
}

static void switchEventCB(uint8_t sw)
{
	// TODO: change waveform
	switch(sw) {
		case 0: dds.setType(DDS::SINE_HIRES);	break; 	
		case 1: dds.setType(DDS::SQUARE);	break; 	
		case 2: dds.setType(DDS::SAW);	break; 	
		default: break;
	}
}


static void updateCB( xTimerHandle xTimer )
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	int32_t f_cv = 0;
	uint32_t f_k1 = 0;
	int32_t f_fm = 0;
	int32_t f = 0;
	
	// actual frequency consists of sum of these
	// knob1 - main frequency
	// knob2 - fine frequency
	// cv2	 - v/oct 
	// cv4	 - fm input attentuated by a knob2
	if(hw->cv.cv2 > CV_UNPLUGGED_VAL) {
		int32_t cvolts =  cv2volts(hw->cv.cv2);
		cvolts -= 7.3*(1<<12);
		f_cv = (cvolts * 1000) >> 12;
	}
	f_k1 = map_value(hw->knob.k1, s_map_freq, 4); // base frequency as set by pitch
	

	if(hw->cv.cv4 > CV_UNPLUGGED_VAL) {
//         fm mod
		f_fm = f_fm*((map_value(hw->cv.cv4, s_map_cv4, 2) << 16) * map_value(hw->knob.k2, s_map_k2_att, 2));
		f_fm = f_fm>>16;
		f_fm += INT16_MIN;
	}

	f =  f_k1 + f_cv + f_fm;
	// set up the frequency for next cycle
	dds.m_inc = min(abs(f), 20000) * (UINT32_MAX / DDS_SAMPLE_RATE); 
	dds.m_backward = (f<0);
	// CV 3 - waveshaper
#if 0
	if(hw->cv.cv3 > CV_UNPLUGGED_VAL) {
		s_waveShaper = map_value(hw->cv.cv3, s_map_ws, 2);
		s_dc = hw->cv.cv4 << 20;
	} else {
		s_waveShaper = map_value(hw->knob.k4, s_map_ws, 2);
		s_dc = hw->knob.k4 << 20;
	}
#endif

//    LOG_PRINT(Log::LOG_DEBUG, "%cf=%d", dds.m_backward?'-':'+',dds.m_inc / (UINT32_MAX/ DDS_SAMPLE_RATE));
//        int32_t f_cv = map_value(hw->cv.cv2-9, s_map_cv2, 2); // semitones
}

static void logCB(xTimerHandle xTimer)
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	int32_t cvolts =  cv2volts(hw->cv.cv2);
	cvolts -= 7.3*(1<<12);
	LOG_PRINT(Log::LOG_DEBUG, "%cf=%d", dds.m_backward?'-':'+',dds.m_inc / (UINT32_MAX/ DDS_SAMPLE_RATE));
	LOG_PRINT(Log::LOG_DEBUG, "cv2=%c%d.%d", (cvolts<0) ? '-' : '+', fp2int(cvolts, 12), frac2int(cvolts, 12));
}

namespace OscClassic {
int16_t update() {
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	if(s_rst) { dds.m_acc = 0; s_rst = 0; }
	int16_t xs = ~dds.next();					// get next sample

	static uint8_t ps = 0;
	uint8_t st = (dds.m_acc + s_phase) > s_dc;	// should we trigger the CV output
	if(ps != st) { 
		PIN_TOGGLE(CV_OUT_PIN)
			ps = st; }
	return  signed_saturate_rshift(xs*s_waveShaper, 16, 9);		
}

void setup() {
	dds.setFrequency((uint16_t)1500);
	dds.setType(DDS::SINE_HIRES);
	TeensyHW::setSwitchEventCB(switchEventCB);
	TeensyHW::setGateCallback(TeensyHW::hw_t::KnobChannel::KC_CV1, cv1gateCB);
	xUpdateTimer = xTimerCreate("UpdateCB", 1, pdTRUE, 	( void * ) 0, updateCB);
	xTimerStart( xUpdateTimer, 0 );
	xLogTimer = xTimerCreate("logt", 100, pdTRUE, 	( void * ) 0, logCB);
	xTimerStart( xLogTimer, 0 );
}
}
