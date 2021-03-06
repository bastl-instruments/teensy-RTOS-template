/**
* @brief 
* @file TaskCalibrate.cpp
* @author J.H. 
* @date 2016-01-20
*/

/* module header */
#include "TaskCalibrate.h"

/* system includes C */
#include "FreeRTOS.h"
#include "task.h"

/* system includes C++ */


/* local includes */
#include "hw.h"


namespace Tasks {
namespace Calibrate {
	// this is used to calibrate the lo-hi values of the knobs. Note this is different from calibrate() routine of the ADC task

	static TaskHandle_t s_xCalTask = NULL;
	static TaskHandle_t s_xNotifyTask = NULL;

	enum CalState {
		CAL_INIT	 	= 	0,
		CAL_SET_MIN	=	1,
		CAL_SET_MAX	=	2,
		CAL_ADJUST	=	3,
		CAL_END			=	100
	};

	static void buttonEventCB(TeensyHW::hw_t::ButtonState s)
	{
		if((s_xCalTask != NULL) && (s == TeensyHW::hw_t::BUTTON_RELEASED)) xTaskNotifyGive(s_xCalTask);
	}

	static inline void __cal_set_min(TeensyHW::hw_t *hw, enum CalState &cal_state) {
		// wait for another button event that signals us to read the ADC values
		if(ulTaskNotifyTake(pdTRUE, 250) == 0) { 
			static int xi = 0;
			TeensyHW::setLed((TeensyHW::hw_t::Led)(xi+1), 0);
			xi = (xi+1) % 4;
			TeensyHW::setLed((TeensyHW::hw_t::Led)(xi+1), 1);
		} else {
			// update min value
			hw->knob_cal_min.k1 = hw->knob.k1;
			hw->knob_cal_min.k2 = hw->knob.k2;
			hw->knob_cal_min.k3 = hw->knob.k3;
			hw->knob_cal_min.k4 = hw->knob.k4;
			cal_state = CAL_SET_MAX;
		}
	}

	static inline void __cal_set_max(TeensyHW::hw_t *hw, enum CalState &cal_state) {
		// wait for another button event that signals us to read the ADC values
		if(ulTaskNotifyTake(pdTRUE, 250) == 0) { 
			static int xi = 0;
			TeensyHW::setLed((TeensyHW::hw_t::Led)(4-xi), 0);
			xi = (xi+1) % 4;
			TeensyHW::setLed((TeensyHW::hw_t::Led)(4-xi), 1);
		} else {
			// update min value
			hw->knob_cal_max.k1 = hw->knob.k1;
			hw->knob_cal_max.k2 = hw->knob.k2;
			hw->knob_cal_max.k3 = hw->knob.k3;
			hw->knob_cal_max.k4 = hw->knob.k4;
			cal_state = CAL_ADJUST;
		}
	}

	static void CalTask(void *param) {
		TeensyHW::hw_t *hw = TeensyHW::getHW();
		enum CalState cal_state = CalState::CAL_INIT;
//        TeensyHW::buttonEventCB_ft old_button_cb = TeensyHW::getButtonEventCB();

		TeensyHW::setButtonEventCB(buttonEventCB);

		while(1) {
			switch(cal_state)	{
				case CalState::CAL_INIT: 
				// wait 500 ms for button event
				if(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(500)) == 0)  { cal_state = CAL_END;  } // timeout 
				else cal_state = CAL_SET_MIN;
				break;
				case CalState::CAL_SET_MIN:
				// -> update MIN value of the pots
					__cal_set_min(hw, cal_state); break;
				case CalState::CAL_SET_MAX:
				// -> update MAX value
					__cal_set_max(hw, cal_state); break;
				case CalState::CAL_ADJUST:
				// write to EEPROM, set adjust value
					TeensyHW::EEWriteCal();
					TeensyHW::adjustKnobs();
					cal_state = CAL_END;
				break;
				case CalState::CAL_END: 
					// signal we are done
					TeensyHW::setLed(TeensyHW::hw_t::LED_1, 0);
					TeensyHW::setLed(TeensyHW::hw_t::LED_2, 0);
					TeensyHW::setLed(TeensyHW::hw_t::LED_3, 0);
					TeensyHW::setLed(TeensyHW::hw_t::LED_4, 0);
					xTaskNotifyGive(s_xNotifyTask);
					vTaskDelete(xTaskGetCurrentTaskHandle());
				break;
			}
		}
	}

	int create(TaskHandle_t notifyTask) {
		if(s_xCalTask != NULL) return -2;
		s_xNotifyTask = notifyTask;
		if(xTaskCreate( CalTask, "cal", 
						configMINIMAL_STACK_SIZE, 
						NULL, tskIDLE_PRIORITY + 2, 
						&s_xCalTask) != pdTRUE) return -1;
	}

}
}
