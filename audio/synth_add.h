#ifndef _SYNTH_ADD_H_
#define _SYNTH_ADD_H_
/* system includes */
/* local includes */

#include "AudioStream.h"
#include "arm_math.h"
#include "utils.h"

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

#define NSINES_MAX	16

class AudioSynthAdditive : public AudioStream
{
public:
	AudioSynthAdditive() : AudioStream(0, NULL), m_nfo()  {
		for(int i = 0; i < NSINES_MAX; i++) m_nfo[i].magnitude = 65536 / NSINES_MAX;
	}
	void frequency(int n, float freq) {
		if (freq < 0.0) freq = 0.0;
		else if (freq > AUDIO_SAMPLE_RATE_EXACT/2) freq = AUDIO_SAMPLE_RATE_EXACT/2;
		if (n > NSINES_MAX) return;
		m_nfo[n].phase_inc = freq * (4294967296.0 / AUDIO_SAMPLE_RATE_EXACT);
	}
	virtual void update(void);

	virtual void lfo(float freq) {
		if (freq < 0.0) freq = 0.0;
		else if (freq > AUDIO_SAMPLE_RATE_EXACT/2) freq = AUDIO_SAMPLE_RATE_EXACT/2;
		m_lfo.phase_inc = freq * (4294967296.0 / AUDIO_SAMPLE_RATE_EXACT);
	}

private:
	void addSineToBlock(uint32_t n, audio_block_t *block);
	sine_nfo_t m_nfo[NSINES_MAX];
	sine_nfo_t m_lfo;
};
#endif

#endif /* _SYNTH_ADD_H_ */

