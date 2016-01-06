/**
* @brief 
* @file TaskUI.cpp
* @author J.H. 
* @date 2015-11-30
*/

/* module header */
#include "TaskUI.h"

/* system includes C */
#include "FreeRTOS.h"
#include "task.h"

/* system includes C++ */


/* local includes */


static TaskHandle_t s_xTask = NULL;

namespace Tasks {
void UITask(void *pvParameters)
{

}
namespace UI {
int create() {
	if(s_xTask != NULL) return -2;
	if(xTaskCreate( Tasks::UITask, "ui", 
					configMINIMAL_STACK_SIZE, 
					NULL, tskIDLE_PRIORITY + 2, 
					&s_xTask) != pdTRUE) return -1;
	return 0;
}

}}
