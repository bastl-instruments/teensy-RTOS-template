/**
* @brief 
* @file app.cpp
* @author J.H. 
* @date 2016-02-10
*/

/* module header */
#include "src/app_tmpl.h"

/* system includes C */
#include "kinetis.h"

/* system includes C++ */


/* local includes */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "src/macros.h"
#include "src/hw.h"
#include "src/logger.h"
#include "src/defines.h"

#define DDS_SAMPLE_RATE	96000			// audio rate
#include "audio/utils.h"
#include "audio/utility/dspinst.h"

// 12bit 0x200, 16bit 0x2000
#define CV_UNPLUGGED_VAL	0x200

namespace App {
static xTimerHandle xUpdateTimer = NULL;
static DDS dds;
static DDS mod;

static uint32_t s_dc = UINT32_MAX / 2;	// duty cycle for the output
static volatile uint32_t s_phase = 0;	// phase of the output
static uint16_t s_waveShaper = 2<<7;
static volatile bool s_rst = 0;

// output timer triggered in a frequency of DDS_SAMPLE_RATE
static void pit0_isr()
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	if(s_rst) { dds.m_acc = 0; s_rst = 0; }
	int16_t xs = ~dds.next();					// get next sample
	dds.m_inc = (hw->knob.k1-252) * (UINT32_MAX / DDS_SAMPLE_RATE); // base frequency as set by pitch
//    dds.m_inc = ((((hw->knob.k1-252) << 12) | (hw->knob.k2)) >> 8) * (UINT32_MAX / DDS_SAMPLE_RATE);
	if(mod.m_inc > 0) {
		dds.m_inc = signed_multiply_32x16b(dds.m_inc, mod.next() + 32768); // detune
	}
	static uint8_t ps = 0;
	uint8_t st = (dds.m_acc + s_phase) > s_dc;	// should we trigger the CV output
	if(ps != st) { 
		PIN_TOGGLE(CV_OUT_PIN)
		ps = st; }
	uint32_t s =  signed_saturate_rshift(xs*s_waveShaper, 16, 9);		
    *(int16_t *)&(DAC0_DAT0L) = (s+32768) >> 4;
	PIT_TFLG0 =  PIT_TFLG_TIF;
}

void setup()
{
	SIM_SCGC6 |= SIM_SCGC6_PIT;						// gate clock
	PIT_MCR = 0;
	PIT_TCTRL0 = PIT_TCTRL_TIE | PIT_TCTRL_TEN;		// enable timer + interrupt
	PIT_LDVAL0 = (F_BUS / DDS_SAMPLE_RATE) - 1;				// update rate
	NVIC_SET_PRIORITY(IRQ_PIT_CH0, 128);
	NVIC_ENABLE_IRQ(IRQ_PIT_CH0);
	_VectorsRam[IRQ_PIT_CH0 + 16] = pit0_isr; // set the timer interrupt
	dds.setFrequency((uint16_t)0);
	dds.setType(DDS::SINE_HIRES);
	SIM_SCGC2 |= SIM_SCGC2_DAC0;
	DAC0_C0 = DAC_C0_DACRFS | DAC_C0_DACEN;                  
	DAC0_C1 = 0;

	mod.setFrequency((uint16_t)0);
}

static void updateCB( xTimerHandle xTimer )
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();

	// CV 2 - V/OCT
	if(hw->cv.cv2 > CV_UNPLUGGED_VAL) {
	} else {
		// if NC, knob1 is pitch, knob2 is finetune
	}
	
	// CV 3 - waveshaper
	if(hw->cv.cv3 > CV_UNPLUGGED_VAL) {

	} else {
		s_waveShaper = hw->knob.k4;
	}
	// CV 4 - FM
	if(hw->cv.cv4 > CV_UNPLUGGED_VAL) {
		uint32_t f = (hw->cv.cv4 * hw->knob.k2) >> 16;
		mod.m_inc = f  * (UINT32_MAX  / DDS_SAMPLE_RATE);
	} else {
		mod.m_acc = mod.m_inc = 0;
	}

}

static void cv1gateCB(bool state)
{
	if(state)s_rst = 1;
}

// button press sets current oscillator type
static void buttonEventCB(TeensyHW::hw_t::ButtonState s)
{
	static uint8_t s_led = 0;
	if(s == TeensyHW::hw_t::BUTTON_PRESSED) {
		TeensyHW::setLed((TeensyHW::hw_t::Led)(s_led+1), 0);
		s_led = (s_led+1) % 4;
		TeensyHW::setLed((TeensyHW::hw_t::Led)(s_led+1), 1);
	} 
	switch(s_led) {
		case 0:	break; 	// classic osc
		case 1:	break; 	// harmonic
		case 2:	break; 	// triple detune
		case 3:	break; 	// 4-poly
		default: break;
	}
}

static void switchEventCB(uint8_t sw)
{
	// TODO: change waveform
	switch(sw) {
		case 0: dds.setType(DDS::SINE_HIRES);	break; 	
		case 1: dds.setType(DDS::SINE);	break; 	
		case 2: dds.setType(DDS::SAW);	break; 	
		default: break;
	}
}

void run()
{
	TeensyHW::setButtonEventCB(buttonEventCB);
	TeensyHW::setSwitchEventCB(switchEventCB);
	TeensyHW::setGateCallback(TeensyHW::hw_t::KnobChannel::KC_CV1, cv1gateCB);

	xUpdateTimer = xTimerCreate("UpdateCB", 1, pdTRUE, 	( void * ) 0, updateCB);
	xTimerStart( xUpdateTimer, 0 );
}

}
