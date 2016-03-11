/**
* @brief 
* @file fixedpoint.cpp
* @author J.H. 
* @date 2016-03-11
*/

/* module header */
#include "fixedpoint.h"

/* system includes C */
#include <math.h>

/* system includes C++ */


/* local includes */

extern "C" {
// convert fractional part of the fixed point integer into a printable integer
uint32_t frac2int(int32_t fp, uint8_t bits)
{
	uint32_t ret = 0;
	if(fp < 0) fp = UINT32_MAX - fp;
	if(bits> 9) { fp>>=bits-9; bits = 9; }
	uint32_t x = pow(10,bits) / (1<<bits);
	for(int i = 0; i<bits; i++) {
		if(fp & 1) ret += x;
		fp >>= 1;
		x <<= 1;
	}
	return ret;
}
uint16_t fp2int(int32_t fp, uint8_t bits)
{
	if(fp < 0) 	return (0xffffffff - fp) >> bits;
	else		 return fp >> bits;
}

}
