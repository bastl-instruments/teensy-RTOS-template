#ifndef _OSC_QUADRO_H_
#define _OSC_QUADRO_H_
/* system includes */
/* local includes */
#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
namespace OscQuadro {
	void setup();
	void suspend();
	void resume();
	int16_t update();
}
#endif

#endif /* _OSC_QUADRO_H_ */

