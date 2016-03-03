/**
* @brief 
* @file fault_isr.cpp
* @author J.H. 
* @date 2016-01-20
*/

/* module header */
#include "fault_isr.h"

/* system includes C */

/* system includes C++ */


/* local includes */
#include "hw.h"
#include "kinetis.h"
#include "logger.h"

#define SCB_SHCSR_USGFAULTENA (uint32_t)1<<18
#define SCB_SHCSR_BUSFAULTENA (uint32_t)1<<17
#define SCB_SHCSR_MEMFAULTENA (uint32_t)1<<16

#include "FreeRTOS.h"
#include "task.h"

#include "core_pins.h"
#include "usb_dev.h"
#include "usb_serial.h"

extern "C" {

void init_fault_isr()
{
	// enable fault handler for errors
	SCB_SHCSR |= SCB_SHCSR_BUSFAULTENA | SCB_SHCSR_USGFAULTENA | SCB_SHCSR_MEMFAULTENA;	
}


void __attribute__((naked)) fault_isr (void) 
{
	vTaskSuspendAll();
	uint8_t toggle = 0;

	uint32_t* sp=0;
	// this is from "Definitive Guide to the Cortex M3" pg 423
	asm volatile ( "TST LR, #0x4\n\t"   // Test EXC_RETURN number in LR bit 2
			"ITE EQ\n\t"         // if zero (equal) then
			"MRSEQ %0, MSP\n\t"  //   Main Stack was used, put MSP in sp
			"MRSNE %0, PSP\n\t"  // else Process stack was used, put PSP in sp
			: "=r" (sp) : : "cc");

#define SCB_SHPR1_USGFAULTPRI *(volatile uint8_t *)0xE000ED20
#define SCB_SHPR1_BUSFAULTPRI *(volatile uint8_t *)0xE000ED19
#define SCB_SHPR1_MEMFAULTPRI *(volatile uint8_t *)0xE000ED18

	// this should allow usb interrupts to preempt
	SCB_SHPR1_BUSFAULTPRI = (uint8_t)255;
	SCB_SHPR1_USGFAULTPRI = (uint8_t)255;
	SCB_SHPR1_MEMFAULTPRI = (uint8_t)255;


#define MSGBUF_SIZE	16
//    char msgbuf[MSGBUF_SIZE+1];
//    sniprintf(msgbuf, MSGBUF_SIZE, "0x%x 0x%x\n", sp[6], sp[5]);
	while(1) {
		toggle = !toggle;
		TeensyHW::setLed(TeensyHW::hw_t::LED_PCB, toggle);
		TeensyHW::setLed(TeensyHW::hw_t::LED_1, toggle);
		TeensyHW::setLed(TeensyHW::hw_t::LED_2,  toggle);
		TeensyHW::setLed(TeensyHW::hw_t::LED_3,  toggle);
		TeensyHW::setLed(TeensyHW::hw_t::LED_4,  toggle);
		TeensyHW::setLed(TeensyHW::hw_t::LED_A,  toggle);
//        LOG_PRINT(Log::LOG_FATAL, "crash: pc=0x%x lr=0x%x", sp[6], sp[5]);
		usb_serial_write("XX\n", 3);

		for (int n=0; n<1000000; n++) { asm("nop");} ;
	}
}


}
