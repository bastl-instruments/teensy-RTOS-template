#ifndef _DEFINES_H_
#define _DEFINES_H_
/* system includes */
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif


// ----------- pins configuration --------------------
#define LED_PCB_CFG	C,5		// teensy pin 	13, led on teensy board
#define LED_1_CFG	C,4		//				10	4 vertical LEDs of the module
#define LED_2_CFG	C,3		//				9
#define LED_3_CFG	D,3		//				8
#define LED_4_CFG	D,2		//				7
#define LED_A_CFG	D,4		//				6	single LED in the bottom left corner

#define BUTTON_A_PIN C,7 	// pin 12		toggle button
#define SWITCH_PIN_A C,6 	// pin 11		3 state switch
#define SWITCH_PIN_B D,1 	// pin 14

#define CV_OUT_PIN	D,0		// pin 2

// multiplexer pins	- output of the mux is connected to analog input pin (MUX_ADC_CHANNEL) of the teensy
#define MUX_A	C,1			// pin 22
#define MUX_B	D,6			// pin 21
#define MUX_C	D,5			// pin 20

#define MUX_ADC_CHANNEL 8


// ----------- some config variables --------------------
// for how long the PIT1 will run until ADC conversion task is woken up - in Hz
// this is used to let the mux settle a bit - basically it is a update frequency of the ADC
#define ADC_TIMER_UPDATE_F	8000

// logging interval of the ADC task - in ms
// if the interval is too low, it may cause instability on the ADC input!
#define ADC_LOG_INTERVAL	1000
#define ADC_LOG_HEX			1

// treshold value for CV unplugged 
// for 12bit ~0x200, for 16bit 0x2000
#ifndef CV_UNPLUGGED_VAL
#define CV_UNPLUGGED_VAL	0x200
#endif

// measured voltage values on the DAC output
#define	DAC_VOUT_MIN		-7.3
#define DAC_VOUT_MAX		7.5
#define DAC_VOUT_DELTA		(DAC_VOUT_MAX-DAC_VOUT_MIN)
#define DAC_V_PER_BIT		DAC_VOUT_DELTA / 0xfff

#ifdef __cplusplus
}
#endif

#endif /* _DEFINES_H_ */

