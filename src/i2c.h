#ifndef _I2C_H_
#define _I2C_H_
/* system includes */
#include <stdint.h>
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

//
// init first i2c of the mk20dx256 controller, pins 18, 19 on teensy (or portb 2+3)
//
extern void i2c_init();

//
// send one byte via i2c
//
extern int i2c_send(uint8_t saddr, uint8_t b);


#ifdef __cplusplus
}
#endif

#endif /* _I2C_H_ */

