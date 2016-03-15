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

#include "median.h"
#include "defines.h"

// defines - configuration
/*
	ADC_CFG1_ADIV(3)         Divide ratio = 4 (F_BUS = 48 MHz => ADCK = 12 MHz)
	ADC_CFG1_MODE(3)         Single ended 16 bit mode
	ADC_CFG1_ADLSMP          Long sample time
*/
#define ADC_CONFIG1 (ADC_CFG1_ADIV(0) |  ADC_CFG1_ADICLK(0) | ADC_CFG1_MODE(1) | ADC_CFG1_ADLSMP)
/*
	ADC_CFG2_MUXSEL          Select channels ADxxb
	ADC_CFG2_ADLSTS(3)       Shortest long sample time
*/
#define ADC_CONFIG2  ADC_CFG2_ADLSTS(0) | ADC_CFG2_MUXSEL

// tasks
static TaskHandle_t s_xADCTask = NULL;
static TaskHandle_t s_xADCUpdateMuxTask = NULL;

#include <list>
class AvgFilter
{
	std::list<uint16_t> m_q;
	public:
		AvgFilter(uint16_t nitems) : m_q(nitems, 0) { }
		uint16_t push(uint16_t val) { 
			uint32_t ret = 0;
			m_q.pop_front(); m_q.push_back(val); 
			for(auto x : m_q) {
				ret += x;
			}
			ret /= m_q.size();
			return ret;
		}
};

uint16_t middle_of_3(uint16_t a, uint16_t b, uint16_t c)
{
	uint16_t middle;

	if ((a <= b) && (a <= c)) 			middle = (b <= c) ? b : c;
	else if ((b <= a) && (b <= c))		middle = (a <= c) ? a : c;
	else								middle = (a <= b) ? a : b;
	return middle;
}

static uint8_t s_med_idx = 0;
static uint16_t s_med[3] = {0,0,0};

//static MedianFilter<uint16_t> s_mf(5);

// current channel multiplexed
static uint16_t s_cur_ch = 0;
static uint16_t s_xval = 0;
static const uint16_t S_CHANNELS[] = {7,5,6,4,0,1,2,3}; //{0,1,2,3,4,5,6,7};

static const uint16_t *s_channels_p = S_CHANNELS;
static uint8_t	s_nchannels = (sizeof(S_CHANNELS) / sizeof(S_CHANNELS[0]));


// calibrate ADC channel
void ADC_CHANNELalibrate() {
	uint16_t sum;

	__disable_irq();
	// Begin calibration
	ADC0_SC3 = ADC_SC3_CAL;
	// Wait for calibration
	while (ADC0_SC3 & ADC_SC3_CAL) { asm("nop"); }

	// Plus side gain
	sum = ADC0_CLPS + ADC0_CLP4 + ADC0_CLP3 + ADC0_CLP2 + ADC0_CLP1 + ADC0_CLP0;
	sum = (sum / 2) | 0x8000;
	ADC0_PG = sum;

	// Minus side gain (not used in single-ended mode)
	sum = ADC0_CLMS + ADC0_CLM4 + ADC0_CLM3 + ADC0_CLM2 + ADC0_CLM1 + ADC0_CLM0;
	sum = (sum / 2) | 0x8000;
	ADC0_MG = sum;
	__enable_irq();
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
//    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// initialize ADC module
static inline void _ADCInit()
{
	ADC0_CFG1 = ADC_CONFIG1;
	ADC0_CFG2 = ADC_CONFIG2;
	// Voltage ref vcc
	ADC0_SC2 = ADC_SC2_REFSEL(0);

	// Enable averaging, 8 samples, single mode
	ADC0_SC3 = ADC_SC3_AVGE | ADC_SC3_AVGS(3);

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
	PIT_LDVAL1 = (F_BUS / ADC_TIMER_UPDATE_F) - 1;
	NVIC_SET_PRIORITY(IRQ_PIT_CH1, 200);
	NVIC_ENABLE_IRQ(IRQ_PIT_CH1);
	_VectorsRam[IRQ_PIT_CH1 + 16] = pit1_isr; // set the timer interrupt
}

void adc0_isr()
{	
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	switch(s_channels_p[s_cur_ch]) {
		case TeensyHW::hw_t::KnobChannel::KC_KNOB1: hw->knob.k1 = ADC0_RA ; break;
		case TeensyHW::hw_t::KnobChannel::KC_KNOB2: hw->knob.k2 = ADC0_RA; break;
		case TeensyHW::hw_t::KnobChannel::KC_KNOB3: hw->knob.k3 = ADC0_RA; break;
		case TeensyHW::hw_t::KnobChannel::KC_KNOB4: hw->knob.k4 = ADC0_RA; break;
//        case TeensyHW::hw_t::KnobChannel::KC_KNOB1: hw->knob.k1 =  (ADC0_RA < hw->knob_cal_min.k1) ? 0 : ((ADC0_RA > hw->knob_cal_max.k1) ? 0xffff : (ADC0_RA-hw->knob_cal_min.k1) * hw->knob_adjust.k1); break;
//        case TeensyHW::hw_t::KnobChannel::KC_KNOB2: hw->knob.k2 =  (ADC0_RA < hw->knob_cal_min.k2) ? 0 : ((ADC0_RA > hw->knob_cal_max.k2) ? 0xffff : (ADC0_RA-hw->knob_cal_min.k2) * hw->knob_adjust.k2); break;
//        case TeensyHW::hw_t::KnobChannel::KC_KNOB3: hw->knob.k3 =  (ADC0_RA < hw->knob_cal_min.k3) ? 0 : ((ADC0_RA > hw->knob_cal_max.k3) ? 0xffff : (ADC0_RA-hw->knob_cal_min.k3) * hw->knob_adjust.k3); break;
//        case TeensyHW::hw_t::KnobChannel::KC_KNOB4: hw->knob.k4 =  (ADC0_RA < hw->knob_cal_min.k4) ? 0 : ((ADC0_RA > hw->knob_cal_max.k4) ? 0xffff : (ADC0_RA-hw->knob_cal_min.k4) * hw->knob_adjust.k4); break;
		case TeensyHW::hw_t::KnobChannel::KC_CV1: 
		case TeensyHW::hw_t::KnobChannel::KC_CV2:
		case TeensyHW::hw_t::KnobChannel::KC_CV3: 
		case TeensyHW::hw_t::KnobChannel::KC_CV4: TeensyHW::setCV((TeensyHW::hw_t::KnobChannel)s_channels_p[s_cur_ch], ADC0_RA); break;
		default:	break;
	}
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(s_xADCUpdateMuxTask, &xHigherPriorityTaskWoken);	
//    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}



static void ADCLogTask(void *pvParameters)
{
	while(1) {
		
		vTaskDelay(ADC_LOG_INTERVAL);
		TeensyHW::hw_t *hw = TeensyHW::getHW();
#if ADC_LOG_HEX == 1
#define ADC_LOG_MSG "adc: %04x %04x %04x %04x %04x %04x %04x %04x"
#else
#define ADC_LOG_MSG	"adc: %04d %04d %04d %04d %04d %04d %04d %04d"
#endif
		LOG_PRINT(Log::LOG_DEBUG, ADC_LOG_MSG, 
				hw->knob.k1, hw->knob.k2,hw->knob.k3,hw->knob.k4,
				hw->cv.cv1,
				hw->cv.cv2,
				hw->cv.cv3,
				hw->cv.cv4);
	}
}

static void ADCUpdateMuxTask(void *pvParameters)
{
	TeensyHW::hw_t *hw = TeensyHW::getHW();
	while(1) {
		// set mux channel
		s_cur_ch = (s_cur_ch+1) % s_nchannels;
		TeensyHW::setMux(s_channels_p[s_cur_ch]);
		// start the timer and wait until it fires
		PIT_TCTRL1 = PIT_TCTRL_TIE | PIT_TCTRL_TEN;
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
//        for(uint16_t i=0;i<3000;i++) asm("nop");
		// start ADC conversion
		ADC0_SC1A = MUX_ADC_CHANNEL | ADC_SC1_AIEN;
		// wait for conversion to complete		TODO - distinguish between timer and adc wakeups
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
//        if(s_channels_p[s_cur_ch] == TeensyHW::hw_t::KnobChannel::KC_KNOB1) {
//            s_med_idx = (s_med_idx +1 ) % 3;
//            s_med[s_med_idx] = s_xval;
//            hw->knob.k1 = middle_of_3(s_med[0], s_med[1], s_med[2]);
//        }
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

void setChannels(const uint16_t chans[], size_t nchans)
{
	NVIC_DISABLE_IRQ(IRQ_ADC0);
	if(chans == NULL) {
		s_channels_p = S_CHANNELS;
		s_nchannels = (sizeof(S_CHANNELS) / sizeof(S_CHANNELS[0]));
	} else {
		s_cur_ch = 0;
		s_channels_p = chans;
		s_nchannels = nchans;
	}
	NVIC_ENABLE_IRQ(IRQ_ADC0);
}

}

}
