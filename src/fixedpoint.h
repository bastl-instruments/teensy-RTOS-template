#ifndef _FIXEDPOINT_H_
#define _FIXEDPOINT_H_
/* system includes */
/* local includes */
#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif

extern uint32_t frac2int(int32_t fp, uint8_t bits);
extern uint16_t fp2int(int32_t fp, uint8_t bits);
#ifdef __cplusplus
}
#endif

#endif /* _FIXEDPOINT_H_ */

