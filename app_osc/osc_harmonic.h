#ifndef _OSC_HARMONIC_H_
#define _OSC_HARMONIC_H_
/* system includes */
/* local includes */
#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
namespace OscHarmonic {
	void setup();
	void suspend();
	void resume();
	int16_t update();
}
#endif

#endif /* _OSC_HARMONIC_H_ */

