/**
* @brief 
* @file conversion.cpp
* @author J.H. 
* @date 2016-03-11
*/

/* module header */
#include "conversion.h"

/* system includes C */

/* system includes C++ */


/* local includes */

extern "C" {
// converts voltage (5:11 FP int) to 12 bit value for the DAC
uint16_t voltsToDAC(uint16_t voltage)
{
	// Vmin 	0x0		-7.3 V
	// Vmax		0xfff	 7.5 V
	// one bit = 0.003614163614163614 V
	// 
	// 12bit dac value = (Vmin - $voltage) / (delta(Vmin,Vmax) / 0xfff)
	//
	const uint16_t bit_volts = (1/0.003614163614163614) * (1<<7); // convert to fixed point 9:7
	return voltage * bit_volts;
}

// convert 12bit dac valut to a s3:11 voltage
uint16_t dac2volts(int16_t reg)
{
	const uint32_t bit_volts = 0.003614163614163614 * (1<<27);
	return (reg*bit_volts) >> 15;
}


int32_t cv2volts(int16_t vv)
{
	const uint32_t bit_volts = 0.0066696710229833265 * (1<<27);
	return ((0xc6c-vv)*bit_volts) >> 15;
}

}
