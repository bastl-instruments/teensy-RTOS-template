#ifndef _EEPROM_ADDR_H_
#define _EEPROM_ADDR_H_
/* system includes */
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

struct knob_cal {
	uint16_t k1;
	uint16_t k2;
	uint16_t k3;
	uint16_t k4;
};
#define EEPROM_ADDR_KNOB_CALMIN		((knob_cal*)0x000)
#define EEPROM_ADDR_KNOB_CALMAX		(EEPROM_ADDR_KNOB_CALMIN + sizeof(knob_cal))

#define EEPROM_ADDR_CV_CALMIN		EEPROM_ADDR_KNOB_CALMAX+sizeof(knob_cal)
#define EEPROM_ADDR_CV_CALMAX		EEPROM_ADDR_CV_CALMIN+sizeof(knob_cal)

#define EEPROM_ADDR_DAC_0V			EEPROM_ADDR_CV_CALMAX+sizeof(knob_cal)
#define EEPROM_ADDR_DAC_1V			EEPROM_ADDR_DAC_0V+sizeof(uint16_t)


#endif

#endif /* _EEPROM_ADDR_H_ */

