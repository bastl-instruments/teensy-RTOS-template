#ifndef _VALMAP_H_
#define _VALMAP_H_
/* system includes */
/* local includes */
#include <stdint.h>
#include <stddef.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct {
	uint16_t to;
	uint16_t y;
} knob_map_t;

uint16_t map_value(uint16_t val, const knob_map_t map[], size_t size);
uint16_t map_value_exp(uint16_t val);


#ifdef __cplusplus
}
#endif

#endif /* _VALMAP_H_ */

