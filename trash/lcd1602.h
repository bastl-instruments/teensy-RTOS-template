#ifndef _LCD1602_H_
#define _LCD1602_H_
/* system includes */
#include <stdint.h>
/* local includes */
#include "macros.h"

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

namespace LCD {

// a simple driver for interfacing lcd1602 display via i2c

typedef struct _lcd1602 {
	_lcd1602() : backlight(0), i2c_addr(0) {}
	uint8_t backlight:1;
	uint8_t i2c_addr;	// i2c address, not aligned
} lcd1602_t; 

#define CMD_CLEAR		0x1
#define CMD_CUR_RIGHT	B(00010100)
#define CMD_CUR_LEFT	B(00010000)
#define CMD_HOME		B(00000010)

//
// try to initialize the display
// @return 0 on success, -1 on error
int init(const lcd1602_t &lcd);
int sendCmd(const lcd1602_t &lcd, uint8_t cmd);
int putChar(const lcd1602_t &lcd, uint8_t c);
int send4bit(const lcd1602_t &lcd, uint8_t c, uint8_t rs);

}


#endif

#endif /* _LCD1602_H_ */

