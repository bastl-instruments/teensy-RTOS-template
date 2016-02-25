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

// defines - configuration
// how long the timer will run until task is woken up - in Hz
#define TIMER_UPDATE_F	2048
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
#define ADC_CHANNEL 8



// tasks
static TaskHandle_t s_xADCTask = NULL;
static TaskHandle_t s_xADCUpdateMuxTask = NULL;

// current channel multiplexed
static uint16_t s_cur_ch = 0;



// calibrate ADC channel
void ADC_CHANNELalibrate() {
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


// as the systick is 1 ms, we need finer resolution to wait when waiting for new ADC mux channel voltage to settle in
//  we use timer1 here to wake up task - adc conversion is done after this wakeup
void pit1_isr()
{
	// disable timer
	PIT_TCTRL1 &=  ~PIT_TCTRL_TEN;
	// reset interrupt flag
	PIT_TFLG1 =  PIT_TFLG_TIF;
	// wakeup adc update task
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(s_xADCUpdateMuxTask, &xHigherPriorityTaskWoken);	
}

// initialize ADC module
static inline void _ADCInit()
{
	ADC0_CFG1 = ADC_CONFIG1;
	ADC0_CFG2 = ADC_CONFIG2;
	// Voltage ref vcc
	ADC0_SC2 = ADC_SC2_REFSEL(0);

	// Enable averaging, 8 samples, single mode
	ADC0_SC3 = ADC_SC3_AVGE | ADC_SC3_AVGS(1);

	ADC_CHANNELalibrate();

	// Enable ADC interrupt, configure pin
//    ADC0_SC1A = ADC_CHANNEL | ADC_SC1_AIEN;
	NVIC_ENABLE_IRQ(IRQ_ADC0);
}

// set up the task wakeup timer
static inline void _Timer1Init()
{
	SIM_SCGC6 |= SIM_SCGC6_PIT;
	PIT_MCR = 0;
	PIT_TCTRL1 = PIT_TCTRL_TIE;
	PIT_LDVAL1 = (F_BUS / TIMER_UPDATE_F) - 1;
	NVIC_SET_PRIORITY(IRQ_PIT_CH1, 200);
	NVIC_ENABLE_IRQ(IRQ_PIT_CH1);
	_VectorsRam[IRQ_PIT_CH1 + 16] = pit1_isr; // set the timer interrupt
}


void adc0_isr()
{	
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	switch(s_cur_ch) {
		case TeensyHW::hw_t::KnobChannel::KC_KNOB1: hw->knob.k1 =  (ADC0_RA < hw->knob_cal_min.k1) ? 0 : ((ADC0_RA > hw->knob_cal_max.k1) ? 0xffff : (ADC0_RA-hw->knob_cal_min.k1) * hw->knob_adjust.k1); break;
		case TeensyHW::hw_t::KnobChannel::KC_KNOB2: hw->knob.k2 =  (ADC0_RA < hw->knob_cal_min.k2) ? 0 : ((ADC0_RA > hw->knob_cal_max.k2) ? 0xffff : (ADC0_RA-hw->knob_cal_min.k2) * hw->knob_adjust.k2); break;
		case TeensyHW::hw_t::KnobChannel::KC_KNOB3: hw->knob.k3 =  (ADC0_RA < hw->knob_cal_min.k3) ? 0 : ((ADC0_RA > hw->knob_cal_max.k3) ? 0xffff : (ADC0_RA-hw->knob_cal_min.k3) * hw->knob_adjust.k3); break;
		case TeensyHW::hw_t::KnobChannel::KC_KNOB4: hw->knob.k4 =  (ADC0_RA < hw->knob_cal_min.k4) ? 0 : ((ADC0_RA > hw->knob_cal_max.k4) ? 0xffff : (ADC0_RA-hw->knob_cal_min.k4) * hw->knob_adjust.k4); break;
		case TeensyHW::hw_t::KnobChannel::KC_CV1: hw->cv.cv1 = ADC0_RA; break;
		case TeensyHW::hw_t::KnobChannel::KC_CV2: hw->cv.cv2 = ADC0_RA; break;
		case TeensyHW::hw_t::KnobChannel::KC_CV3: hw->cv.cv3 = ADC0_RA; break;
		case TeensyHW::hw_t::KnobChannel::KC_CV4: hw->cv.cv4 = ADC0_RA; break;
		default:	break;
	}
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(s_xADCUpdateMuxTask, &xHigherPriorityTaskWoken);	
}



static void ADCLogTask(void *pvParameters)
{
	while(1) {
		vTaskDelay(1000);
		TeensyHW::hw_t *hw = TeensyHW::getHW();
		LOG_PRINT(Log::LOG_DEBUG, "adc: %04x %04x %04x %04x %04x %04x %04x %04x", 
				hw->knob.k1,  hw->knob.k2,hw->knob.k3,hw->knob.k4,
				hw->cv.cv1,
				hw->cv.cv2,
				hw->cv.cv3,
				hw->cv.cv4);
	}
}

static void ADCUpdateMuxTask(void *pvParameters)
{
	while(1) {
		// set mux channel
		s_cur_ch = (s_cur_ch+1) % 8;
		TeensyHW::setMux(s_cur_ch);
		// start the timer and wait until it fires
		PIT_TCTRL1 = PIT_TCTRL_TIE | PIT_TCTRL_TEN;
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
		// start ADC conversion
		ADC0_SC1A = ADC_CHANNEL | ADC_SC1_AIEN;
		// wait for conversion to complete		TODO - distinguish between timer and adc wakeups
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
	}
}


namespace Tasks {
namespace ADC {

int create() {
	_ADCInit();
	_Timer1Init();
	if(s_xADCTask != NULL) return -2;
	if(xTaskCreate( ADCLogTask, "adc", 
					configMINIMAL_STACK_SIZE*2, 
					NULL, tskIDLE_PRIORITY + 2, 
					&s_xADCTask) != pdTRUE) return -1;
	if(xTaskCreate( ADCUpdateMuxTask, "adcMux", 
					configMINIMAL_STACK_SIZE, 
					NULL, tskIDLE_PRIORITY + 2, 
					&s_xADCUpdateMuxTask) != pdTRUE) return -1;
	return 0;
}

}

}
