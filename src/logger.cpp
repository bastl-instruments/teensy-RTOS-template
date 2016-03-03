/**
* @brief 
* @file logger.cpp
* @author J.H. 
* @date 2016-01-05
*/

/* module header */
#include "logger.h"

/* system includes C */
#include <stdarg.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <semphr.h>

/* system includes C++ */

/* local includes */
#include "core_pins.h"
#include "usb_serial.h"
#include "usb_dev.h"


#define USB_LOG	
#define LOGBUF_SIZE	64
static char s_logbuf[LOGBUF_SIZE];
static SemaphoreHandle_t s_logSem = NULL;

namespace Log {

int init()
{
	if(s_logSem == NULL) {
		s_logSem  = xSemaphoreCreateMutex();
		if(s_logSem == NULL) {
			return -1;
		}
	}
	return 0;
}

	
void print(loglevel_t lvl, const char *file, int line, const char *fmt, ...) 
{
#ifdef USB_LOG
	if(usb_configuration == 0) return;
	if(usb_serial_write_buffer_free() == 0) return;

	if(xSemaphoreTake(s_logSem, 10) == pdFALSE) return;

	int size;
	// print time of log
	size = sniprintf(s_logbuf, LOGBUF_SIZE, "%d [%08ld] ", usb_tx_packet_count(CDC_TX_ENDPOINT),  millis());
	usb_serial_write(s_logbuf, size);
	
//     print loglevel
	switch(lvl) {
		case loglevel_t::LOG_FATAL: usb_serial_putchar('F'); break;
		case loglevel_t::LOG_ERROR: usb_serial_putchar('E'); break;
		case loglevel_t::LOG_WARNING: usb_serial_putchar('W'); break;
		case loglevel_t::LOG_INFO: usb_serial_putchar('I'); break;
		case loglevel_t::LOG_DEBUG: usb_serial_putchar('D'); break;
		default:					usb_serial_putchar('?');
	}

	usb_serial_putchar(' ');


	// message
	va_list args;
	va_start (args, fmt);
	size = vsniprintf(s_logbuf, LOGBUF_SIZE, fmt , args);
	va_end(args);
	usb_serial_write(s_logbuf, size);

	// file, line
	size = sniprintf(s_logbuf, LOGBUF_SIZE, " (%s:%d)", file, line);
	usb_serial_write(s_logbuf, size);

	usb_serial_putchar('\n');
	xSemaphoreGive(s_logSem);
#endif
}

}

