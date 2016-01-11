/**
* @brief 
* @file lcd1602.cpp
* @author J.H. 
* @date 2015-11-26
*/

/* module header */
#include "lcd1602.h"

/* system includes C */

/* system includes C++ */


/* local includes */
#include "macros.h"
#include "i2c.h"

#include <avr_emulation.h>

namespace LCD {
// i2c configuration for current controller
//    RS
// 0: 10011110000 RS
// 1: 01011110000 RW
// 2: 00111110000 E
// 3: 00011110000 Backlight
// 4: 00011111000 D4
// 5: 00011110100 D5
// 6: 00011110010 D6
// 7: 00011110001 D7

#define RS_EXT_PIN	0
#define RW_EXT_PIN	1
#define E_EXT_PIN	2
#define LED_EXT_PIN	3


int send4bit(const lcd1602_t &lcd, uint8_t d, uint8_t rs)
{
	uint8_t err= 0;

	// first send data with E high
	err = i2c_send(lcd.i2c_addr, (d << 4) | (rs << RS_EXT_PIN) | (lcd.backlight<<LED_EXT_PIN) | (1<<E_EXT_PIN)); 
	if(err != 0) return err;

	delayMicroseconds(4);
	// pulse clock with E low
	err = i2c_send(lcd.i2c_addr, (d << 4) | (rs << RS_EXT_PIN) | (lcd.backlight<<LED_EXT_PIN)); 
	if(err != 0) return err;

	delayMicroseconds(50);

	return err;
}

int sendCmd(const lcd1602_t &lcd, uint8_t cmd)
{
	if(send4bit(lcd, cmd >> 4, 0) != 0) return -1;
	if(send4bit(lcd, cmd, 0) != 0) return -1;
	return 0;
}

int init(const lcd1602_t &lcd)
{
	i2c_init();
	// set 4bit mode
	send4bit(lcd, B(0011),0);
	delayMicroseconds(4500);
//    send4bit(lcd, B(0011),0);
//    delayMicroseconds(150);
	send4bit(lcd, B(0011),0);
	delayMicroseconds(150);
	send4bit(lcd, B(0010),0);
	delayMicroseconds(150);

	send4bit(lcd, B(0000),0);
	send4bit(lcd, B(1110),0);
	delayMicroseconds(150);

	sendCmd(lcd, CMD_CLEAR);
	delayMicroseconds(580);
	return 0;
}



int putChar(const lcd1602_t &lcd, uint8_t c)
{
	if(send4bit(lcd, c>>4, 1) != 0) return -1;
	if(send4bit(lcd, c, 1) != 0) return -1;
	return 0;
	
}

}
