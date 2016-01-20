#ifndef _TASKCALIBRATE_H_
#define _TASKCALIBRATE_H_
/* system includes */
/* local includes */

#include "FreeRTOS.h"
#include "task.h"

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
namespace Tasks {
namespace Calibrate {
	int create(TaskHandle_t notifyTask);
}
}
#endif

#endif /* _TASKCALIBRATE_H_ */

