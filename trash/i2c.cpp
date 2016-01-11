/**
* @brief 
* @file i2c.cpp
* @author J.H. 
* @date 2015-11-25
*/

/* module header */
#include "i2c.h"

/* system includes C */
#include <kinetis.h>

/* system includes C++ */

/* local includes */

#include "hw.h"

#define I2C_MULT_OFFSET	6
#define I2C_MULT	(2 << I2C_MULT_OFFSET)

#define I2C_WAIT { while((s_i2c->S & I2C_S_IICIF) == 0) {};	s_i2c->S |= I2C_S_IICIF; }

#define BREADBOARD_USED	0


#if BREADBOARD_USED
static KINETIS_I2C_t *s_i2c = &KINETIS_I2C0;
#else
static KINETIS_I2C_t *s_i2c = &KINETIS_I2C1;
#endif

extern "C" {

// for breadboard pins are PORTB 2,3 and i2c0 is used
// for module these pins are occupied by the A5, A4 so we have to use i2c1 that is located on the bottom of the board

void i2c_init()
{
#if BREADBOARD_USED
	PORTB_PCR2 = PORT_PCR_MUX(2) | PORT_PCR_ODE ; 	// set i2c for the pin + open drain enable
	PORTB_PCR3 = PORT_PCR_MUX(2)  | PORT_PCR_ODE;
	SIM_SCGC4 |= SIM_SCGC4_I2C0; 					// clock gating for the i2c module
	SIM_SCGC5 |= SIM_SCGC5_PORTB; 					// clock gating for the portb

#else
	PORTC_PCR10 = PORT_PCR_MUX(2) | PORT_PCR_ODE ; 	// set i2c for the pin + open drain enable
	PORTC_PCR11 = PORT_PCR_MUX(2)  | PORT_PCR_ODE;
	SIM_SCGC4 |= SIM_SCGC4_I2C1; 					// clock gating for the i2c module
	SIM_SCGC5 |= SIM_SCGC5_PORTC; 					// clock gating for the portb
#endif
	s_i2c->C1  = I2C_C1_IICEN ;						// enable i2c
	s_i2c->C2  = I2C_C2_HDRS | I2C_C2_SBRC ;			// ?? but it works + slave baud rate control to allow rate adjust
	s_i2c->F 	= 0x35;									// frequency, TODO tuning
}

int i2c_send(uint8_t saddr, uint8_t b)
{
	int8_t ret = 0;
	// set master + tx
	s_i2c->C1 |= I2C_C1_TX | I2C_C1_MST;

	// send slave addr
	s_i2c->D = (saddr << 1) ;
	I2C_WAIT
	if((s_i2c->S & I2C_S_RXAK) == 1) { ret = -1; goto end; }

	// send data byte
	s_i2c->D = b;
	I2C_WAIT
	if((s_i2c->S & I2C_S_RXAK) == 1)  { ret = -1; goto end; }

	end:
	s_i2c->C1 &= ~(I2C_C1_MST | I2C_C1_TX);
	return ret;
}


}
