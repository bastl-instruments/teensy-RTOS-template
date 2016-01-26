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
	_sine_nfo() : phase_acc(0), phase_inc(0), magnitude (16384) {}
	uint32_t phase_acc;
	uint32_t phase_inc;
	int32_t magnitude;
} sine_nfo_t;



#ifdef __cplusplus
}

class DDS {
	public:
		enum WaveForm {
			SINE,
			SQUARE,
			SAW,
			TABLE,
			TRIANGLE
		};
		DDS() : m_acc(0), m_inc(0), m_mag(32767), m_waveform(SINE), m_cycles(0), m_cycles_max(0) {}
		void setFrequency(uint16_t freq) {
			if (freq < 0.0) freq = 0.0;
			else if (freq > AUDIO_SAMPLE_RATE_EXACT/2) freq = AUDIO_SAMPLE_RATE_EXACT/2;
			m_inc = freq * (4294967296.0 / AUDIO_SAMPLE_RATE_EXACT);
		}
		void setFrequency(float freq) {
			if (freq < 0.0) freq = 0.0;
			else if (freq > AUDIO_SAMPLE_RATE_EXACT/2) freq = AUDIO_SAMPLE_RATE_EXACT/2;
			m_inc = freq * (4294967296.0 / AUDIO_SAMPLE_RATE_EXACT);
		}
		void setFrequencyRel(float freq) {
			// TODO adjust for boundaries
			if (freq < 0.0) freq = 0.0;
			else if (freq > AUDIO_SAMPLE_RATE_EXACT/2) freq = AUDIO_SAMPLE_RATE_EXACT/2;
			m_inc += freq * (4294967296.0 / AUDIO_SAMPLE_RATE_EXACT);
		}
		void setAmplitude(float n) {
			if (n < 0) n = 0;
			else if (n > 1.0) n = 1.0;
			m_mag = 32767.0 * n;
		}
		int16_t next();
		void setType(enum WaveForm w) {
			m_waveform = w;
		}

	
		uint32_t m_acc;
		uint32_t m_inc;
		int16_t m_mag;
		enum WaveForm  m_waveform;
		uint16_t m_cycles;
		uint16_t m_cycles_max;


};
#endif

#endif /* _UTILS_H_ */

