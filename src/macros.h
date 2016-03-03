#ifndef _MACROS_H_
#define _MACROS_H_
/* system includes */
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#define B(x) \
  ((0##x >>  0 & 0x01) | \
  (0##x >>  2 & 0x02) | \
  (0##x >>  4 & 0x04) | \
  (0##x >>  6 & 0x08) | \
  (0##x >>  8 & 0x10) | \
  (0##x >> 10 & 0x20) | \
  (0##x >> 12 & 0x40) | \
  (0##x >> 14 & 0x80))

#define MIN(X, Y)  ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y)  ((X) > (Y) ? (X) : (Y))

#define INIT_INPUT(...)		INIT_INPUT_(__VA_ARGS__)
#define INIT_INPUT_(port, pin)	{PORT##port##_PCR##pin = PORT_PCR_MUX(1) | PORT_PCR_PE | PORT_PCR_PFE | PORT_PCR_PS; GPIO##port##_PDDR &= ~(1<<pin);}

#define INIT_OUTPUT(...)		INIT_OUTPUT_(__VA_ARGS__)
#define INIT_OUTPUT_(port, pin)	{PORT##port##_PCR##pin = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1); GPIO##port##_PDDR |= (1<<pin);}

#define PIN_TOGGLE(...)		PIN_TOGGLE_(__VA_ARGS__)
#define PIN_TOGGLE_(port, pin)		{ GPIO##port##_PTOR = (1<<pin); }
#define PIN_HIGH(...)		PIN_HIGH_(__VA_ARGS__)
#define PIN_HIGH_(port, pin)		{ GPIO##port##_PSOR = (1<<pin); }
#define PIN_LOW(...)		PIN_LOW_(__VA_ARGS__)
#define PIN_LOW_(port, pin)		{ GPIO##port##_PCOR = (1<<pin); }
#define PIN_SET(...)		PIN_SET_(__VA_ARGS__)
#define PIN_SET_(port,pin,state) { if(state) PIN_HIGH(port,pin) else PIN_LOW(port,pin); }

#define PIN_STATE(...)		PIN_STATE_(__VA_ARGS__)
#define PIN_STATE_(port,pin)		(GPIO##port##_PDIR & (1<<pin))

#ifdef __cplusplus
}
#endif

#endif /* _MACROS_H_ */

