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

#include "app_cfg.h"
#include "src/macros.h"
#include "src/hw.h"
#include "src/logger.h"
#include "src/defines.h"
#include "osc_classic.h"
#include "osc_quadro.h"

typedef struct {
	int16_t (*update)();
	void (*setup)();
	void (*suspend)();
	void (*resume)();
} osc_CB_t;

static osc_CB_t s_osc_CB[] = {
	{ OscClassic::update, OscClassic::setup, OscClassic::suspend, OscClassic::resume },
	{ OscQuadro::update, OscQuadro::setup, OscQuadro::suspend, OscQuadro::resume }
} ;

static osc_CB_t *s_current_osc = &s_osc_CB[0];

namespace App {


// output timer triggered in a frequency of DDS_SAMPLE_RATE
static void pit0_isr()
{
	if(s_current_osc != NULL) *(int16_t *)&(DAC0_DAT0L) = (s_current_osc->update()+32768) >> 4;
	PIT_TFLG0 =  PIT_TFLG_TIF;
}


//class OscDuo : public OscDef
//{
//    public:
//        void setup() {
//            dds.setAmplitude(0.5);
//            mod.setAmplitude(0.5);
//        }
//        int16_t update() {
//            int16_t ret = dds.next();
//            ret+= mod.next();

//            mod.m_inc = map_value(hw->knob.k1, s_map_freq, 4) * (UINT32_MAX / DDS_SAMPLE_RATE); // base frequency as set by pitch
//            dds.m_inc = map_value(hw->knob.k2, s_map_freq, 4)*2 * (UINT32_MAX / DDS_SAMPLE_RATE); // base frequency as set by pitch

//            return ret;
//        }
//};

void setup()
{
	for(auto&& osc : s_osc_CB) {
		osc.setup();
	}
	SIM_SCGC6 |= SIM_SCGC6_PIT;						// gate clock
	PIT_MCR = 0;
	PIT_TCTRL0 = PIT_TCTRL_TIE | PIT_TCTRL_TEN;		// enable timer + interrupt
	PIT_LDVAL0 = (F_BUS / DDS_SAMPLE_RATE) - 1;				// update rate
	NVIC_SET_PRIORITY(IRQ_PIT_CH0, 128);
	NVIC_ENABLE_IRQ(IRQ_PIT_CH0);
	_VectorsRam[IRQ_PIT_CH0 + 16] = pit0_isr; // set the timer interrupt
	SIM_SCGC2 |= SIM_SCGC2_DAC0;
	DAC0_C0 = DAC_C0_DACRFS | DAC_C0_DACEN;                  
	DAC0_C1 = 0;

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
	if(s_current_osc == NULL) return;
	NVIC_DISABLE_IRQ(IRQ_PIT_CH0);
	s_current_osc->suspend();
	switch(s_led) {
		case 0:	s_current_osc = &s_osc_CB[0]; break; 	// classic osc
		case 1:	break; 	// harmonic
		case 2:	break; 	// triple detune
		case 3:	s_current_osc = &s_osc_CB[1]; break; 	// 4-poly
		default: break;
	}
	s_current_osc->resume();
	NVIC_ENABLE_IRQ(IRQ_PIT_CH0);
}


void run()
{
	TeensyHW::setButtonEventCB(buttonEventCB);
}

}
