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
	uint8_t led1:1;
	uint8_t led2:1;
	uint8_t led3:1;
	uint8_t led4:1;

	uint8_t ledA:1;
	uint8_t ledPCB:1;
	
	volatile uint8_t button:2;
	// state values for button debouncing
	volatile uint8_t button_s:1;
	volatile uint8_t button_i:4;
	volatile uint8_t switch3:2;


	struct  {
		uint16_t k1;
		uint16_t k2;
		uint16_t k3;
		uint16_t k4;
	} knob;
	struct  {
		uint16_t cv1;
		uint16_t cv2;
		uint16_t cv3;
		uint16_t cv4;
		uint16_t cv5;
	} cv;
	struct {
		uint8_t cv1:1;
		uint8_t cv2:1;
		uint8_t cv3:1;
		uint8_t cv4:1;
	} cvAct;
	enum ButtonState {
		BUTTON_NULL		=	0,
		BUTTON_PRESSED	=	1,
		BUTTON_RELEASED	=	2,
		BUTTON_HOLD		=	3
	};
	enum Led {
		LED_1,
		LED_2,
		LED_3,
		LED_4,
		LED_A,
		LED_PCB
	};

	uint8_t led1_blinko;
	uint8_t led2_blinko;
	uint8_t led3_blinko;
	uint8_t led4_blinko;
	uint8_t ledA_blinko;
	uint8_t ledPCB_blinko;
} hw_t;

typedef void (*buttonEventCB_ft)(hw_t::ButtonState s);

extern int init();
extern void setLed(hw_t::Led led, bool s);
extern void setLedBlink(hw_t::Led led, uint8_t b);
extern hw_t *getHW();
extern void setButtonEventCB(buttonEventCB_ft f);

}

#endif

#endif /* _HW_H_ */

