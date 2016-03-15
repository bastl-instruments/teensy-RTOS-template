/**
* @brief 
* @file hw.cpp
* @author J.H. 
* @date 2015-11-24
*/

/* module header */
#include "hw.h"

/* system includes C */
#include <kinetis.h>
#include "portManipulations.h"
#include "FreeRTOS.h"
#include "timers.h"

/* system includes C++ */


/* local includes */
#include "macros.h"
#include "avr_functions.h"
#include "src/eeprom_addr.h"
#include "logger.h"
#include "src/defines.h"


namespace TeensyHW {

static xTimerHandle xHWTimer = NULL;
static xTimerHandle xLedTimer = NULL;
static hw_t _g_hw;
static buttonEventCB_ft _s_buttonEvent_cb;
static switchEventCB_ft _s_switchAEvent_cb;

typedef struct {
	gateEventCB_ft cb;
	bool state;
} gate_state_t;

hw_t::hw_()
{}

static gate_state_t	_s_gateEvent_cb[hw_t::KnobChannel::KC_CHANNELS];


// input interrupt is set on either edges
#define ENABLE_INTERRUPT(...) ENABLE_INTERRUPT_(__VA_ARGS__)
#define ENABLE_INTERRUPT_(port, pin)	{ PORT##port##_PCR##pin |= PORT_PCR_IRQC(0xb);	NVIC_ENABLE_IRQ(IRQ_PORT##port); }

#define CV_GATE_VAL	0x9c4


static void __initLeds()
{
	INIT_OUTPUT(LED_PCB_CFG)
	INIT_OUTPUT(LED_1_CFG)
	INIT_OUTPUT(LED_2_CFG)
	INIT_OUTPUT(LED_3_CFG)
	INIT_OUTPUT(LED_4_CFG)
	INIT_OUTPUT(LED_A_CFG)

	INIT_OUTPUT(CV_OUT_PIN)

}

static inline void __initMux()
{
	INIT_OUTPUT(MUX_C)
	INIT_OUTPUT(MUX_B)
	INIT_OUTPUT(MUX_A)

	for(auto & e : _s_gateEvent_cb)  e.cb = NULL;
}


static inline void __initButtons()
{
	INIT_INPUT(BUTTON_A_PIN);
	INIT_INPUT(SWITCH_PIN_A);
	INIT_INPUT(SWITCH_PIN_B);
}

#if 0
void portc_isr(void)
{
	uint32_t isfr = PORTC_ISFR;
	PORTC_ISFR = isfr;
	_g_hw.isfr = isfr;
	if(isfr & (1<<7)) {
		_g_hw.but_fired++;
		_g_hw.button = (_g_hw.button << 1) | (!(GPIOC_PDIR & (1<<7)));
		TeensyHW::setLed(TeensyHW::hw_t::LED_1, _g_hw.button & 1);
	}
	if(isfr & (1<<6)) {
		_g_hw.switch3 = (_g_hw.switch3 << 2) | (!((GPIOC_PDIR & (1<<6))) << 1) | (!(GPIOD_PDIR & (1<<1)));
	}
}

void portd_isr(void)
{
	uint32_t isfr = PORTD_ISFR;
	PORTD_ISFR = isfr;
	if(isfr & (1<<1)) {
		_g_hw.switch3 = (_g_hw.switch3 << 2) | (!((GPIOC_PDIR & (1<<6))) << 1) | (!(GPIOD_PDIR & (1<<1)));
	}
}
	ENABLE_INTERRUPT(BUTTON_A_PIN)
	ENABLE_INTERRUPT(SWITCH_PIN_A)
	ENABLE_INTERRUPT(SWITCH_PIN_B)
	_VectorsRam[IRQ_PORTC+16] = portc_isr;
	_VectorsRam[IRQ_PORTD+16] = portd_isr;
#endif

#define BUTTON_I_MAX	3

void HWUpdateTimerCB(xTimerHandle xT)
{
	// -- update button state --
	_g_hw.button_i = ((PIN_STATE(BUTTON_A_PIN)) == 0) ? MIN(BUTTON_I_MAX,_g_hw.button_i+1) : MAX(0,_g_hw.button_i-1);
	if(_g_hw.button_i == 0) {
		_g_hw.button_s = 0;
	} else if(_g_hw.button_i >= BUTTON_I_MAX) {
		_g_hw.button_s = 1;
		_g_hw.button_i = BUTTON_I_MAX;
	}

	_g_hw.button = (_g_hw.button << 1) | _g_hw.button_s;
	if((_s_buttonEvent_cb != NULL) && 
			((_g_hw.button == hw_t::BUTTON_PRESSED) || 
			 (_g_hw.button == hw_t::BUTTON_HOLD) || 
			 (_g_hw.button == hw_t::BUTTON_RELEASED))) _s_buttonEvent_cb((TeensyHW::hw_t::ButtonState)_g_hw.button);


	// -- update switch state --
	_g_hw.switchA_i = ((PIN_STATE(SWITCH_PIN_A)) == 0) ? MIN(BUTTON_I_MAX,_g_hw.switchA_i+1) : MAX(0,_g_hw.switchA_i-1);
	if(_g_hw.switchA_i == 0) {
		_g_hw.switchA_s = 0;
	} else if(_g_hw.switchA_i >= BUTTON_I_MAX) {
		_g_hw.switchA_s = 1;
		_g_hw.switchA_i = BUTTON_I_MAX;
	}
	_g_hw.switchA = (_g_hw.switchA << 1) | _g_hw.switchA_s;

	_g_hw.switchB_i = ((PIN_STATE(SWITCH_PIN_B)) == 0) ? MIN(BUTTON_I_MAX,_g_hw.switchB_i+1) : MAX(0,_g_hw.switchB_i-1);
	if(_g_hw.switchB_i == 0) {
		_g_hw.switchB_s = 0;
	} else if(_g_hw.switchB_i >= BUTTON_I_MAX) {
		_g_hw.switchB_s = 1;
		_g_hw.switchB_i = BUTTON_I_MAX;
	}
	_g_hw.switchB = (_g_hw.switchB << 1) | _g_hw.switchB_s;

	// callback if state of a switch changed
	if((_s_switchAEvent_cb != NULL) && 	((_g_hw.switchA == hw_t::BUTTON_PRESSED))) _s_switchAEvent_cb(0);
	if((_s_switchAEvent_cb != NULL) && 	((_g_hw.switchB == hw_t::BUTTON_PRESSED))) _s_switchAEvent_cb(2);
	if((_s_switchAEvent_cb != NULL) && 	((_g_hw.switchA == hw_t::BUTTON_RELEASED))) _s_switchAEvent_cb(1);
	if((_s_switchAEvent_cb != NULL) && 	((_g_hw.switchB == hw_t::BUTTON_RELEASED))) _s_switchAEvent_cb(1);

}

void HWBlinkyCB(xTimerHandle xT)
{
	static uint8_t counter;
	counter++;
	if(counter & _g_hw.led1_blinko) setLed(hw_t::LED_1, !_g_hw.led1);
	if(counter & _g_hw.led2_blinko) setLed(hw_t::LED_2, !_g_hw.led2);
	if(counter & _g_hw.led3_blinko) setLed(hw_t::LED_3, !_g_hw.led3);
	if(counter & _g_hw.led4_blinko) setLed(hw_t::LED_4, !_g_hw.led4);
	if(counter & _g_hw.ledA_blinko) setLed(hw_t::LED_A, !_g_hw.ledA);
	if(counter & _g_hw.ledPCB_blinko) setLed(hw_t::LED_PCB, !_g_hw.ledPCB);
}

void adjustKnobs()
{
	_g_hw.knob_adjust.k1 = 65535.0 / (_g_hw.knob_cal_max.k1 - _g_hw.knob_cal_min.k1);
	_g_hw.knob_adjust.k2 = 65535.0 / (_g_hw.knob_cal_max.k2 - _g_hw.knob_cal_min.k2);
	_g_hw.knob_adjust.k3 = 65535.0 / (_g_hw.knob_cal_max.k3 - _g_hw.knob_cal_min.k3);
	_g_hw.knob_adjust.k4 = 65535.0 / (_g_hw.knob_cal_max.k4 - _g_hw.knob_cal_min.k4);
}

int init()
{
	__initLeds();
	__initButtons();
	__initMux();
	eeprom_initialize();
	EEReadCal();
	adjustKnobs();


	xHWTimer = xTimerCreate("HWTimer", ( 20 ),  pdTRUE,  ( void * ) 0, HWUpdateTimerCB);
	xLedTimer = xTimerCreate("LEDTimer", 100, pdTRUE, (void*) 0, HWBlinkyCB);

	xTimerStart( xHWTimer, pdFALSE );
	xTimerStart( xLedTimer, pdFALSE );
	return 0;
}


void setLed(hw_t::Led led, bool s)
{
	switch(led) {
		case hw_t::LED_1: _g_hw.led1 = s; PIN_SET(LED_1_CFG, s); break;
		case hw_t::LED_2: _g_hw.led2 = s; PIN_SET(LED_2_CFG, s); break;
		case hw_t::LED_3: _g_hw.led3 = s; PIN_SET(LED_3_CFG, s); break;
		case hw_t::LED_4: _g_hw.led4 = s; PIN_SET(LED_4_CFG, s); break;
		case hw_t::LED_A: _g_hw.ledA = s; PIN_SET(LED_A_CFG, s); break;
		case hw_t::LED_PCB: _g_hw.ledPCB = s; PIN_SET(LED_PCB_CFG, s); break;
	}

}


void setLedBlink(hw_t::Led led, uint8_t b)
{
	switch(led) {
		case hw_t::LED_1: _g_hw.led1_blinko = b; break;
		case hw_t::LED_2: _g_hw.led2_blinko = b; break;
		case hw_t::LED_3: _g_hw.led3_blinko = b; break;
		case hw_t::LED_4: _g_hw.led4_blinko = b; break;
		case hw_t::LED_A: _g_hw.ledA_blinko = b; break;
		case hw_t::LED_PCB: _g_hw.ledPCB_blinko = b; break;
	}
}

hw_t *getHW() { return &_g_hw; }


void setButtonEventCB(buttonEventCB_ft f)
{
	_s_buttonEvent_cb = f;
}

void setSwitchEventCB(switchEventCB_ft f)
{
	_s_switchAEvent_cb = f;
	_g_hw.switchA = _g_hw.switchB = 0;
}

void setGateCallback(TeensyHW::hw_t::KnobChannel mux, gateEventCB_ft f)
{
	if((mux != hw_t::KnobChannel::KC_CV1) && (mux != hw_t::KnobChannel::KC_CV2) && (mux != hw_t::KnobChannel::KC_CV3) && (mux != hw_t::KnobChannel::KC_CV4)) return;
	_s_gateEvent_cb[mux].cb = f;
}

buttonEventCB_ft getButtonEventCB()
{
	return _s_buttonEvent_cb;
}

void setCV(hw_t::KnobChannel mux, uint16_t val)
{
	if((mux != hw_t::KnobChannel::KC_CV1) && (mux != hw_t::KnobChannel::KC_CV2) && (mux != hw_t::KnobChannel::KC_CV3) && (mux != hw_t::KnobChannel::KC_CV4)) return;
	if(_s_gateEvent_cb[mux].cb != NULL) {
		if((_s_gateEvent_cb[mux].state == 0) && (val > CV_GATE_VAL)) { _s_gateEvent_cb[mux].cb(_s_gateEvent_cb[mux].state = 1); }
		else if((_s_gateEvent_cb[mux].state == 1) && (val < CV_GATE_VAL)) { _s_gateEvent_cb[mux].cb(_s_gateEvent_cb[mux].state = 0); }
	}
	switch(mux) {
		case TeensyHW::hw_t::KnobChannel::KC_CV1: _g_hw.cv.cv1 = val; break;
		case TeensyHW::hw_t::KnobChannel::KC_CV2: _g_hw.cv.cv2 = val; break;
		case TeensyHW::hw_t::KnobChannel::KC_CV3: _g_hw.cv.cv3 = val; break;
		case TeensyHW::hw_t::KnobChannel::KC_CV4: _g_hw.cv.cv4 = val; break;
		default: break;
	}
}

void EEWriteCal()
{
	eeprom_write_block(&_g_hw.knob_cal_min, EEPROM_ADDR_KNOB_CALMIN, sizeof(_g_hw.knob_cal_min));
	eeprom_write_block(&_g_hw.knob_cal_max, EEPROM_ADDR_KNOB_CALMAX, sizeof(_g_hw.knob_cal_max));
	eeprom_write_block(&_g_hw.cv_cal_0v, EEPROM_ADDR_CV_CALMIN, sizeof(_g_hw.cv_cal_0v));
	eeprom_write_block(&_g_hw.cv_cal_1v, EEPROM_ADDR_CV_CALMAX, sizeof(_g_hw.cv_cal_1v));
	eeprom_write_block(&_g_hw.dac_0v, EEPROM_ADDR_DAC_0V, sizeof(_g_hw.dac_0v));
	eeprom_write_block(&_g_hw.dac_1v, EEPROM_ADDR_DAC_1V, sizeof(_g_hw.dac_1v));
}

void EEReadCal()
{
	eeprom_read_block(&_g_hw.knob_cal_min, EEPROM_ADDR_KNOB_CALMIN, sizeof(_g_hw.knob_cal_min));
	eeprom_read_block(&_g_hw.knob_cal_max, EEPROM_ADDR_KNOB_CALMAX, sizeof(_g_hw.knob_cal_max));
	eeprom_read_block(&_g_hw.cv_cal_0v, EEPROM_ADDR_CV_CALMIN, sizeof(_g_hw.cv_cal_0v));
	eeprom_read_block(&_g_hw.cv_cal_1v, EEPROM_ADDR_CV_CALMAX, sizeof(_g_hw.cv_cal_1v));
	eeprom_read_block(&_g_hw.dac_0v, EEPROM_ADDR_DAC_0V, sizeof(_g_hw.dac_0v));
	eeprom_read_block(&_g_hw.dac_1v, EEPROM_ADDR_DAC_1V, sizeof(_g_hw.dac_1v));
}

void setCVcal(hw_t::KnobChannel mux, uint16_t v0, uint16_t v1)
{
	switch(mux) {
		case TeensyHW::hw_t::KnobChannel::KC_CV1: _g_hw.cv_cal_0v.cv1 = v0;  _g_hw.cv_cal_1v.cv1 = v1;break;
		case TeensyHW::hw_t::KnobChannel::KC_CV2: _g_hw.cv_cal_0v.cv2 = v0;  _g_hw.cv_cal_1v.cv2 = v1;break;
		case TeensyHW::hw_t::KnobChannel::KC_CV3: _g_hw.cv_cal_0v.cv3 = v0;  _g_hw.cv_cal_1v.cv3 = v1;break;
		case TeensyHW::hw_t::KnobChannel::KC_CV4: _g_hw.cv_cal_0v.cv4 = v0;  _g_hw.cv_cal_1v.cv4 = v1;break;
		default: return;
	}
	eeprom_write_block(&_g_hw.cv_cal_0v, EEPROM_ADDR_CV_CALMIN, sizeof(_g_hw.cv_cal_0v));
	eeprom_write_block(&_g_hw.cv_cal_1v, EEPROM_ADDR_CV_CALMAX, sizeof(_g_hw.cv_cal_1v));
}

int32_t cv2volts(hw_t::KnobChannel ch, uint16_t val)
{
	uint16_t *v0_p = NULL;
	uint16_t *v1_p= NULL;
	int32_t tmp;
	switch(ch) {
		case TeensyHW::hw_t::KnobChannel::KC_CV1: v0_p = &_g_hw.cv_cal_0v.cv1; v1_p = &_g_hw.cv_cal_1v.cv1;break;
		case TeensyHW::hw_t::KnobChannel::KC_CV2: v0_p = &_g_hw.cv_cal_0v.cv2; v1_p = &_g_hw.cv_cal_1v.cv2;break;
		case TeensyHW::hw_t::KnobChannel::KC_CV3: v0_p = &_g_hw.cv_cal_0v.cv3; v1_p = &_g_hw.cv_cal_1v.cv3;break;
		case TeensyHW::hw_t::KnobChannel::KC_CV4: v0_p = &_g_hw.cv_cal_0v.cv4; v1_p = &_g_hw.cv_cal_1v.cv4;break;
		default: return -1;
	}
//    LOG_PRINT(Log::LOG_DEBUG, "v=%x 0v=%x 1v=%x", val, *v0_p, *v1_p);

	tmp = ((val-*v0_p)<<16);
	return tmp/(*v1_p-*v0_p);
}

void setMux(uint8_t channel)
{
	PIN_SET(MUX_A, (channel & 1));
	PIN_SET(MUX_B, (channel & 2));
	PIN_SET(MUX_C, (channel & 4));
}

//
// @param zero is a value at which there is (almost) 0V output on the DAC
// @param volt_diff is a difference to one volt
//
void setDACCalibration(uint16_t zero, uint16_t volt_diff)
{
	_g_hw.dac_0v = zero; _g_hw.dac_1v = volt_diff;
	eeprom_write_block(&zero, 		EEPROM_ADDR_DAC_0V, sizeof(zero));
	eeprom_write_block(&volt_diff, 	EEPROM_ADDR_DAC_1V, sizeof(volt_diff));
}

uint16_t DAC_volts2dac(int16_t volts)
{
	int32_t bpv = (_g_hw.dac_1v - _g_hw.dac_0v);
	bpv = (volts*bpv) >> 12;
//    LOG_PRINT(Log::LOG_DEBUG, "v=%x 0v=%x 1v=%x bpv=%x", volts, _g_hw.dac_0v, _g_hw.dac_1v, bpv);
	return bpv + _g_hw.dac_0v;
}
uint16_t DAC_dac2volts(uint16_t dac)
{
//    LOG_PRINT(Log::LOG_DEBUG, "dac=%x 0v=%x 1v=%x", dac, _g_hw.dac_0v, _g_hw.dac_1v);
	int32_t tmp = ((dac-_g_hw.dac_0v)<<12);
	return (tmp/(_g_hw.dac_1v - _g_hw.dac_0v));
}

}
