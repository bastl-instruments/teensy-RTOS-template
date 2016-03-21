#ifndef _OSC_TRIPLE_H_
#define _OSC_TRIPLE_H_
/* system includes */
/* local includes */
#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
namespace OscTriple {
	void setup();
	void suspend();
	void resume();
	int16_t update();
}
#endif

#endif /* _OSC_TRIPLE_H_ */

