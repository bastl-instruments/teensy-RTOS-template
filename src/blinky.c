/**
 * Binky test
 */

//this file gives us the pinMode and digitalWrite defines that are part of the
//ardiuno compatibiltiy stuff. If we really wanted to, we could go totally
//hardcore and write our own.
#include <core_pins.h>

//this is found in our local include directory
#include "blinky.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "random.h"

#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define UARTLog_Task_Priority				( tskIDLE_PRIORITY + 2 )
static xTimerHandle xLEDTimer = NULL;
#define mainLED_TIMER_PERIOD_MS		( 3000UL / portTICK_RATE_MS )
#define mainDONT_BLOCK	(0)


static void prvQueueReceiveTask(void *pvParameters)
{
	while(1)
		{
		}
}

static void prvLEDToggleTask(void *pvParameters)
{
	static int toggle = 0;
	while(1)
		{
				toggle = (toggle) ? 0 : 1;
				digitalWriteFast(13, toggle);
				vTaskDelay(xor_rand(100));
		}
}


static void prvLEDTimerCallback( xTimerHandle xTimer )
{
	static int toggle = 0;

	if(toggle == 0) {
		digitalWriteFast(12, HIGH);
		toggle = 1;
	}
	else {
		digitalWriteFast(12, LOW);
		toggle = 0;
	}
}



int main()
{

	pinMode(13, OUTPUT);
	pinMode(12, OUTPUT);

/*    xTaskCreate( prvQueueReceiveTask, "Rx", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL );*/
	xTaskCreate( prvLEDToggleTask, "Rx", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, NULL );

/*    xLEDTimer = xTimerCreate( 	"LEDTimer", 					|+ A text name, purely to help debugging. +|*/
/*                                ( mainLED_TIMER_PERIOD_MS ),	|+ The timer period, in this case 5000ms (5s). +|*/
/*                                pdTRUE,						|+ This is a one shot timer, so xAutoReload is set to pdFALSE. +|*/
/*                                ( void * ) 0,					|+ The ID is not used, so can be set to anything. +|*/
/*                                prvLEDTimerCallback				|+ The callback function that switches the LED off. +|*/
/*                                );*/
	
/*    xTimerStart( xLEDTimer, mainDONT_BLOCK );*/
	vTaskStartScheduler();

    return 0;
}
