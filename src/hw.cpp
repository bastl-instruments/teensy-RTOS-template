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

#include "LCDTask.h"

namespace TeensyHW {

static xTimerHandle xHWTimer = NULL;
static hw_t _g_hw;



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
	if(_g_hw.button == hw_t::BUTTON_PRESSED) {
		static bool s = 0;
		s = !s;
		setLed(hw_t::LED_1, s);
	}

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

 xHWTimer = xTimerCreate( 	"HWTimer", 					//|+ A text name, purely to help debugging. +|
							 ( 30 ),	//|+ The timer period, in this case 5000ms (5s). +|
							 pdTRUE,						//|+ This is a one shot timer, so xAutoReload is set to pdFALSE. +|
							 ( void * ) 0,					//|+ The ID is not used, so can be set to anything. +|
							 HWUpdateTimerCB			//	|+ The callback function that switches the LED off. +|
							 );
	xTimerStart( xHWTimer, pdFALSE );
	return 0;
}


void setLed(hw_t::Led led, bool s)
{
	switch(led) {
		case hw_t::LED_1: PIN_SET(LED_1_CFG, s); break;
		case hw_t::LED_2: PIN_SET(LED_2_CFG, s); break;
		case hw_t::LED_3: PIN_SET(LED_3_CFG, s); break;
		case hw_t::LED_4: PIN_SET(LED_4_CFG, s); break;
		case hw_t::LED_A: PIN_SET(LED_A_CFG, s); break;
		case hw_t::LED_PCB: PIN_SET(LED_PCB_CFG, s); break;
	}

}

hw_t *getHW() { return &_g_hw; }



}
