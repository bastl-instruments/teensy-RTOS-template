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

/* system includes C++ */

/* local includes */
#include "core_pins.h"
#include "usb_serial.h"


#define USB_LOG	1
#define LOGBUF_SIZE	64
static char s_logbuf[LOGBUF_SIZE];

namespace Log {
	
void print(loglevel_t lvl, const char *file, int line, const char *fmt, ...) 
{
#if USB_LOG

	int size;
	// print time of log
	size = sniprintf(s_logbuf, LOGBUF_SIZE, "[%08ld] ", millis());
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
#endif
}

}

