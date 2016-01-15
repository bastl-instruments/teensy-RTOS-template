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

#define NSINES_MAX 16

class AudioKDrum : public AudioStream
{
public:
	AudioKDrum() : AudioStream(0, NULL), m_nfo() {}
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
	void trigger() { _updateFInternal(m_fBase); }
private:
	void _updateFInternal(float f) {
		m_nfo.phase_inc = f * (4294967296.0 / AUDIO_SAMPLE_RATE_EXACT);
	}
	void addSineToBlock(uint32_t n, audio_block_t *block);
	sine_nfo_t m_nfo;
	float m_fInc;
	float m_fBase;
};
#endif

#endif /* _SYNTH_KDRUM_H_ */

