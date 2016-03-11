#ifndef _UTILS_H_
#define _UTILS_H_
/* system includes */
#include <stdint.h>
#include "AudioStream.h"
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

typedef struct _sine_nfo {
	_sine_nfo() : phase_acc(0), phase_inc(0), magnitude (65535) {}
	uint32_t phase_acc;
	uint32_t phase_inc;
	uint16_t magnitude;
} sine_nfo_t;



#ifdef __cplusplus
}

#ifndef DDS_SAMPLE_RATE
#define DDS_SAMPLE_RATE 44100
#endif

class DDS {
	public:
		enum WaveForm {
			SINE,
			SINE_HIRES,
			SQUARE,
			SAW,
			TABLE,
			TRIANGLE
		};
		DDS() : m_acc(0), m_inc(0), m_mag(65535), m_waveform(SINE), m_cycles(0), m_cycles_max(0), m_backward(0) {}
		void setFrequency(uint16_t freq) {
			if (freq < 0.0) freq = 0.0;
			else if (freq > DDS_SAMPLE_RATE/2) freq = DDS_SAMPLE_RATE/2;
			m_inc = freq * (4294967296.0 / DDS_SAMPLE_RATE);
		}
		void setFrequency(float freq) {
			if (freq < 0.0) freq = 0.0;
			else if (freq > DDS_SAMPLE_RATE/2) freq = DDS_SAMPLE_RATE/2;
			m_inc = freq * (4294967296.0 / DDS_SAMPLE_RATE);
		}
		void setFrequencyRel(float freq) {
			// TODO adjust for boundaries
			if (freq < 0.0) freq = 0.0;
			else if (freq > DDS_SAMPLE_RATE/2) freq = DDS_SAMPLE_RATE/2;
			m_inc += freq * (4294967296.0 / DDS_SAMPLE_RATE);
		}
		void setAmplitude(float n) {
			if (n < 0) n = 0;
			else if (n > 1.0) n = 1.0;
			m_mag = 65535.0 * n;
		}
		int16_t next();
		void setType(enum WaveForm w) {
			m_waveform = w;
		}

	
		volatile uint32_t m_acc;
		volatile uint32_t m_inc;
		uint16_t m_mag;
		enum WaveForm  m_waveform;
		uint16_t m_cycles;
		uint16_t m_cycles_max;

		uint8_t  m_backward;


};
#endif

#endif /* _UTILS_H_ */

