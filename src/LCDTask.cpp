/**
* @brief 
* @file LCDTask.cpp
* @author J.H. 
* @date 2015-11-26
*/

/* module header */
#include "LCDTask.h"

/* system includes C */
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdarg.h>


/* system includes C++ */


/* local includes */
#include "lcd1602.h"
static TaskHandle_t s_xLCDTask = NULL;
#define DISPLAY_SIZE	32
static char s_dBuf[DISPLAY_SIZE];

namespace Tasks {

#define LCD_I2C_ADDR	0x3f

void LCDUpdate(void *pvParameters)
{
	// 1) init LCD 
	LCD::lcd1602_t lcd;
	lcd.i2c_addr = LCD_I2C_ADDR;
	lcd.backlight	= 1;
	LCD::init(lcd);

	while(1) {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		LCD::sendCmd(lcd, CMD_HOME);
		vTaskDelay(400);
		for(uint8_t i = 0; i < DISPLAY_SIZE; i++) {
			LCD::putChar(lcd, s_dBuf[i]);
		}
	}
}

namespace LCDTask {


int create()
{
	if(s_xLCDTask != NULL) return -2;
	if(xTaskCreate( Tasks::LCDUpdate, "lcd", 
					configMINIMAL_STACK_SIZE*16, 
					NULL, tskIDLE_PRIORITY + 2, 
					&s_xLCDTask) != pdTRUE) return -1;
	update("hello world", 11);
	notify();
	return 0;
}


void update(const char *msg, size_t size)
{
	memset(s_dBuf, ' ', DISPLAY_SIZE);
	memcpy(s_dBuf, msg, (size > DISPLAY_SIZE) ? DISPLAY_SIZE : size);
}

void updatef(const char *msg, ...)
{
	va_list args;
	va_start (args, msg);
	vsniprintf(s_dBuf, DISPLAY_SIZE, msg , args);
	va_end(args);

}
void notify() {	xTaskNotifyGive(s_xLCDTask); }
void notifyFromISR() {	
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(s_xLCDTask, &xHigherPriorityTaskWoken); 
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	
}
}

}
