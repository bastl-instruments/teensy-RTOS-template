#ifndef _SYNTH_KDRUM_H_
#define _SYNTH_KDRUM_H_
/* system includes */
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#include "AudioStream.h"
#include "arm_math.h"

typedef struct _sine_nfo {
	_sine_nfo() : phase_acc(0), phase_inc(0), magnitude (16384) {}
	uint32_t phase_acc;
	uint32_t phase_inc;
	int32_t magnitude;
} sine_nfo_t;

//
// linear decay stretched on X samples
class VCA {
	public:
		VCA() : m_state(1), m_step(1) {}
		void setLength(uint16_t samples) {  m_step = -1.0/samples; }
		float next() { 
			m_state += m_step;
			if(m_state < 0) m_state = 0;
			if(m_state > 1) m_state = 1;
			return m_state;
		}
		void reset() { m_state = 1; }
//    private:
		float m_state;
		float m_step;
};

class AudioKDrum : public AudioStream
{
public:
	AudioKDrum() : AudioStream(0, NULL), m_nfo() { m_vca.setLength(10000); }
	void frequency(float freq) {
		if (freq < 0.0) freq = 0.0;
		else if (freq > AUDIO_SAMPLE_RATE_EXACT/2) freq = AUDIO_SAMPLE_RATE_EXACT/2;
		m_fBase = freq;
	}
	virtual void update(void);
	void finc(float f) {
		if(f < 0) f = 1;
		else if(f > 10) f = 10;
		m_fInc = f;
	}
	void dec(uint16_t samples) { m_vca.setLength(samples); }
	void trigger() { _updateFInternal(m_fBase); m_vca.reset(); }

	float fse() { return m_vca.m_state; }
	float fsp() { return m_vca.m_step; }
private:
	void _updateFInternal(float f) {
		m_nfo.phase_inc = f * (4294967296.0 / AUDIO_SAMPLE_RATE_EXACT);
	}
	void addSineToBlock(uint32_t n, audio_block_t *block);
	sine_nfo_t m_nfo;
	float m_fInc;
	float m_fBase;
	VCA	m_vca;
};
#endif

#endif /* _SYNTH_KDRUM_H_ */

