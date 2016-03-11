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
