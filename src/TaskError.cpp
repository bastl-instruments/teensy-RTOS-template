/**
* @brief 
* @file TaskError.cpp
* @author J.H. 
* @date 2015-11-26
*/

/* module header */

/* system includes C */
#include "FreeRTOS.h"
#include "TaskError.h"
#include "task.h"
#include <stdio.h>

/* system includes C++ */


/* local includes */
#include "LCDTask.h"

static TaskHandle_t s_xErrorTask = NULL;

namespace Tasks {
void ErrorReportTask(void *pvParameters)
{
	while(1) {
		uint8_t errno = ulTaskNotifyTake( pdTRUE, portMAX_DELAY ); // wait until someone unblocks us
		char buf[8];
		snprintf(buf, 8, "err: %d", errno);
		Tasks::LCDTask::update(buf, strlen(buf));
	}
}
namespace Error {

int create() {
	if(s_xErrorTask != NULL) return -2;
	if(xTaskCreate( Tasks::ErrorReportTask, "error", 
					configMINIMAL_STACK_SIZE*2, 
					NULL, tskIDLE_PRIORITY + 2, 
					&s_xErrorTask) != pdTRUE) return -1;
	return 0;
}


void error(uint32_t errno)
{
	xTaskNotify(s_xErrorTask, errno, eSetValueWithOverwrite);
}

}
}
