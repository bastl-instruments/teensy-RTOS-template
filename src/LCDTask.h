#ifndef _LCDTASK_H_
#define _LCDTASK_H_
/* system includes */
/* local includes */
#include "string.h"

#ifdef  __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}

namespace Tasks {


namespace LCDTask {

	int create();
	void update(const char *msg, size_t size);
	void updatef(const char *msg, ...);
	void notify();
	void notifyFromISR();

}


}
#endif

#endif /* _LCDTASK_H_ */

