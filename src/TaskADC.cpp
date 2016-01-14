/**
* @brief 
* @file TaskADC.cpp
* @author J.H. 
* @date 2015-11-26
*/

/* module header */
#include "TaskADC.h"

/* system includes C */
#include "FreeRTOS.h"
#include "task.h"
#include <kinetis.h>
#include <stdio.h>

/* system includes C++ */


/* local includes */
#include "logger.h"
#include "macros.h"
#include "hw.h"

void adcCalibrate() {
	uint16_t sum;

	// Begin calibration
	ADC0_SC3 = ADC_SC3_CAL;
	// Wait for calibration
	while (ADC0_SC3 & ADC_SC3_CAL);

	// Plus side gain
	sum = ADC0_CLPS + ADC0_CLP4 + ADC0_CLP3 + ADC0_CLP2 + ADC0_CLP1 + ADC0_CLP0;
	sum = (sum / 2) | 0x8000;
	ADC0_PG = sum;

	// Minus side gain (not used in single-ended mode)
	sum = ADC0_CLMS + ADC0_CLM4 + ADC0_CLM3 + ADC0_CLM2 + ADC0_CLM1 + ADC0_CLM0;
	sum = (sum / 2) | 0x8000;
	ADC0_MG = sum;
}

/*
	ADC_CFG1_ADIV(2)         Divide ratio = 4 (F_BUS = 48 MHz => ADCK = 12 MHz)
	ADC_CFG1_MODE(2)         Single ended 10 bit mode
	ADC_CFG1_ADLSMP          Long sample time
*/
#define ADC_CONFIG1 (ADC_CFG1_ADIV(3) |  ADC_CFG1_ADICLK(1) | ADC_CFG1_MODE(3) | ADC_CFG1_ADLSMP)
/*
	ADC_CFG2_MUXSEL          Select channels ADxxb
	ADC_CFG2_ADLSTS(3)       Shortest long sample time
*/
#define ADC_CONFIG2  ADC_CFG2_ADLSTS(0) | ADC_CFG2_MUXSEL
#define ADCC 8

static uint16_t s_cur_ch = 0;

#define MUX_A	C,1
#define MUX_B	D,6
#define MUX_C	D,5
static void _muxit(uint8_t val)
{
	PIN_SET(MUX_A, (val & 1));
	PIN_SET(MUX_B, (val & 2));
	PIN_SET(MUX_C, (val & 4));
}

static void _ADCInit()
{
	 // init muxer - pin 20,, 21, 22
	INIT_OUTPUT(MUX_C)
	INIT_OUTPUT(MUX_B)
	INIT_OUTPUT(MUX_A)


	ADC0_CFG1 = ADC_CONFIG1;
	ADC0_CFG2 = ADC_CONFIG2;
	// Voltage ref vcc, hardware trigger, DMA
	ADC0_SC2 = ADC_SC2_REFSEL(0);

	// Enable averaging, 4 samples
	ADC0_SC3 = ADC_SC3_AVGE | ADC_SC3_AVGS(0);

	adcCalibrate();

	// Enable ADC interrupt, configure pin
	ADC0_SC1A = ADCC | ADC_SC1_AIEN;
	NVIC_ENABLE_IRQ(IRQ_ADC0);
}

enum KnobChannel {
	KC_KNOB1	= 5,
	KC_KNOB2	= 4,
	KC_KNOB3	= 6,
	KC_KNOB4	= 7,
	KC_CV1		= 0,
	KC_CV2		= 1,
	KC_CV3		= 2,
	KC_CV4		= 3
} ;


void adc0_isr()
{	
//    s_ch_vals[s_cur_ch] = ADC0_RA;
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	switch(s_cur_ch) {
		case KnobChannel::KC_KNOB1: hw->knob.k1 = ADC0_RA; break;
		case KnobChannel::KC_KNOB2: hw->knob.k2 = ADC0_RA; break;
		case KnobChannel::KC_KNOB3: hw->knob.k3 = ADC0_RA; break;
		case KnobChannel::KC_KNOB4: hw->knob.k4 = ADC0_RA; break;
		case KnobChannel::KC_CV1: hw->cv.cv1 = ADC0_RA; break;
		case KnobChannel::KC_CV2: hw->cv.cv2 = ADC0_RA; break;
		case KnobChannel::KC_CV3: hw->cv.cv3 = ADC0_RA; break;
		case KnobChannel::KC_CV4: hw->cv.cv4 = ADC0_RA; break;
		default:	break;
	}
	s_cur_ch = (s_cur_ch+1) % 8;
	_muxit(s_cur_ch);
	ADC0_SC1A = ADCC | ADC_SC1_AIEN;
}


static TaskHandle_t s_xADCTask = NULL;

static void ADCTask(void *pvParameters)
{
	while(1) {
		TeensyHW::hw_t *hw = TeensyHW::getHW();
		LOG_PRINT(Log::LOG_DEBUG, "adc: %04x %04x %04x %04x %04x %04x %04x %04x", 
				hw->knob.k1, hw->knob.k2,hw->knob.k3,hw->knob.k4,
				hw->cv.cv1, hw->cv.cv2, hw->cv.cv3, hw->cv.cv4
				);
		vTaskDelay(100);
	}
}
namespace Tasks {
namespace ADC {

int create() {
	_ADCInit();
	if(s_xADCTask != NULL) return -2;
	if(xTaskCreate( ADCTask, "adc", 
					configMINIMAL_STACK_SIZE*2, 
					NULL, tskIDLE_PRIORITY + 2, 
					&s_xADCTask) != pdTRUE) return -1;
	return 0;
}

}

}
