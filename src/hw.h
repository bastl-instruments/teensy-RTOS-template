#ifndef _HW_H_
#define _HW_H_
/* system includes */
#include <stdint.h>
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

namespace TeensyHW {


typedef struct hw_ {
	hw_();
	uint8_t led1:1;
	uint8_t led2:1;
	uint8_t led3:1;
	uint8_t led4:1;

	uint8_t ledA:1;
	uint8_t ledPCB:1;
	
	// button
	volatile uint8_t button:2;
	// state values for button debouncing
	volatile uint8_t button_s:1;
	volatile uint8_t button_i:4;

	// switch A
	volatile uint8_t switchA:2;
	volatile uint8_t switchA_s:1;
	volatile uint8_t switchA_i:4;
	// switch B
	volatile uint8_t switchB:2;
	volatile uint8_t switchB_s:1;
	volatile uint8_t switchB_i:4;


	struct  {
		uint16_t k1;
		uint16_t k2;
		uint16_t k3;
		uint16_t k4;
	} knob;
	struct  {
		uint16_t k1;
		uint16_t k2;
		uint16_t k3;
		uint16_t k4;
	} knob_cal_min;
	struct  {
		uint16_t k1;
		uint16_t k2;
		uint16_t k3;
		uint16_t k4;
	} knob_cal_max;
	struct {
		float k1;
		float k2;
		float k3;
		float k4;
	} knob_adjust;
	// CV inputs. if value is < 0x2000, the input is disconnected
	struct  {
		uint16_t cv1;
		uint16_t cv2;
		uint16_t cv3;
		uint16_t cv4;
	} cv_cal_0v;
	struct  {
		uint16_t cv1;
		uint16_t cv2;
		uint16_t cv3;
		uint16_t cv4;
	} cv_cal_1v;
	struct  {
		uint16_t cv1;
		uint16_t cv2;
		uint16_t cv3;
		uint16_t cv4;
		uint16_t cv5;
	} cv;
	enum ButtonState {
		BUTTON_NULL		=	0,
		BUTTON_PRESSED	=	1,
		BUTTON_RELEASED	=	2,
		BUTTON_HOLD		=	3
	};
	enum Led {
		LED_1	=	1,
		LED_2	=	2,
		LED_3	=	3,
		LED_4	=	4,
		LED_A	=	10,
		LED_PCB =	20
	};
	// mux channel
	enum KnobChannel {
		KC_KNOB1	= 5,
		KC_KNOB2	= 4,
		KC_KNOB3	= 6,
		KC_KNOB4	= 7,
		KC_CV1		= 0,
		KC_CV2		= 1,
		KC_CV3		= 2,
		KC_CV4		= 3,

		KC_CHANNELS	= 8
	} ;

	uint8_t led1_blinko;
	uint8_t led2_blinko;
	uint8_t led3_blinko;
	uint8_t led4_blinko;
	uint8_t ledA_blinko;
	uint8_t ledPCB_blinko;

	uint16_t dac_0v;
	uint16_t dac_1v;
} hw_t;

typedef void (*buttonEventCB_ft)(hw_t::ButtonState s);
typedef void (*switchEventCB_ft)(uint8_t sw);
typedef void (*gateEventCB_ft)(bool state);

// call init() to set pin i/o properly
extern int init();
extern hw_t *getHW();

// led manipulation
extern void setLed(hw_t::Led led, bool s);
extern void setLedBlink(hw_t::Led led, uint8_t b);

// callbacks called whenever button state changes
extern void setButtonEventCB(buttonEventCB_ft f);
extern void setSwitchEventCB(switchEventCB_ft f);
extern buttonEventCB_ft getButtonEventCB();

// setting of a knob/cv value for the global hw_t structure
// only this function will call gate callbacks set by setGateCallback()!
extern void setCV(hw_t::KnobChannel mux, uint16_t val);
extern void setCVcal(hw_t::KnobChannel mux, uint16_t v0, uint16_t v1);
extern void setGateCallback(hw_t::KnobChannel mux, gateEventCB_ft);

// calibration
//extern void EEWriteCal(hw_t::KnobChannel ch, uint16_t min, uint16_t max);
extern void EEWriteCal();
extern void EEReadCal();
extern void adjustKnobs();

extern int32_t cv2volts(hw_t::KnobChannel ch, uint16_t val);

//  convert voltage (s4:11 integer) to a 12bit DAC value
extern uint16_t DAC_volts2dac(int16_t volts);
extern uint16_t DAC_dac2volts(uint16_t dac);
extern void setDACCalibration(uint16_t dac_0v, uint16_t dac_1v);

extern void setMux(uint8_t channel);

}

#endif

#endif /* _HW_H_ */

