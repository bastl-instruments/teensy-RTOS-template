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


namespace TeensyHW {

static xTimerHandle xHWTimer = NULL;
static xTimerHandle xLedTimer = NULL;
static hw_t _g_hw;
static buttonEventCB_ft _s_buttonEvent_cb;



// input interrupt is set on either edges
#define ENABLE_INTERRUPT(...) ENABLE_INTERRUPT_(__VA_ARGS__)
#define ENABLE_INTERRUPT_(port, pin)	{ PORT##port##_PCR##pin |= PORT_PCR_IRQC(0xb);	NVIC_ENABLE_IRQ(IRQ_PORT##port); }

#define LED_PCB_CFG	C,5		// teensy pin 	13
#define LED_1_CFG	C,4		//				10
#define LED_2_CFG	C,3		//				9
#define LED_3_CFG	D,3		//				8
#define LED_4_CFG	D,2		//				7
#define LED_A_CFG	D,4		//				6

#define BUTTON_A_PIN C,7 	// pin 12
#define SWITCH_PIN_A C,6 	// pin 11
#define SWITCH_PIN_B D,1 	// pin 14

static void __initLeds()
{
	INIT_OUTPUT(LED_PCB_CFG)
	INIT_OUTPUT(LED_1_CFG)
	INIT_OUTPUT(LED_2_CFG)
	INIT_OUTPUT(LED_3_CFG)
	INIT_OUTPUT(LED_4_CFG)
	INIT_OUTPUT(LED_A_CFG)


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
	_g_hw.button_i = ((GPIOC_PDIR & (1<<7)) == 0) ? MIN(BUTTON_I_MAX,_g_hw.button_i+1) : MAX(0,_g_hw.button_i-1);
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

static void __initButtons()
{
	INIT_INPUT(BUTTON_A_PIN);
	INIT_INPUT(SWITCH_PIN_A);
	INIT_INPUT(SWITCH_PIN_B);
}


int init()
{
	__initLeds();
	__initButtons();
_g_hw.led1_blinko = 16;

	xHWTimer = xTimerCreate("HWTimer", ( 30 ),  pdTRUE,  ( void * ) 0, HWUpdateTimerCB);
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

}
