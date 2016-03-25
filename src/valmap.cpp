/**
* @brief 
* @file valmap.cpp
* @author J.H. 
* @date 2016-03-03
*/

/* module header */
#include "valmap.h"

/* system includes C */

/* system includes C++ */


/* local includes */
#include "resources.h"

uint16_t map_value(uint16_t val, const knob_map_t map[], size_t size)
{
	for(uint16_t i = 0; i < (size-1); i++) {
		if((map[i].to <= val) && (val <= map[i+1].to)) {
			uint32_t py = (map[i+1].y - map[i].y) << 16;
			uint32_t px = (map[i+1].to - map[i].to);
			int32_t k = py/px;
//            int32_t res = //(k * (int32_t(val - map[i].to))) + int32_t(map[i].y);
			return (((val - map[i].to)*k) >> 16) + map[i].y;
		}
	}
	return val;
}
#include "src/logger.h"
uint16_t map_value_exp(uint16_t val)
{
	uint32_t v1, v2;
	v1 = lut_exponential[val >> 8];
	v2 = lut_exponential[(val >> 8)+1];

//    LOG_PRINT(Log::LOG_DEBUG, "v1=%d  v2=%d", v1, v2);
	uint16_t scale;
	scale = (val >> 8) & 0xff;
	v2 *= scale;
	v1 *= 0xFFFF - scale;
	return (v1+v2)>>16;
}
