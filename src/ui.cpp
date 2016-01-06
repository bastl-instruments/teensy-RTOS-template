/**
* @brief 
* @file ui.cpp
* @author J.H. 
* @date 2015-11-24
*/

/* module header */
#include "ui.h"

/* system includes C */

/* system includes C++ */


/* local includes */

#include "hw.h"
#include "portManipulations.h"

namespace UI
{
	void update() {
		const TeensyHW::hw_t *hw = TeensyHW::getHW();

		TeensyHW::setLed(TeensyHW::hw_t::LED_A, hw->button);
		
	}
}
