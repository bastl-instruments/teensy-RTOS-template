#ifndef _CONVERSION_H_
#define _CONVERSION_H_
/* system includes */
/* local includes */
#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif

extern uint16_t voltsToDAC(uint16_t voltage);
extern uint16_t dac2volts(int16_t reg);
extern int32_t cv2volts(int16_t vv);
#ifdef __cplusplus
}
#endif

#endif /* _CONVERSION_H_ */

