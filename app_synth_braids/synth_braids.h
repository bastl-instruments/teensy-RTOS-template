#ifndef _SYNTH_BRAIDS_H_
#define _SYNTH_BRAIDS_H_
/* system includes */
/* local includes */
#include "macro_oscillator.h"
#include "AudioStream.h"
#include "arm_math.h"

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

const uint16_t kAudioBlockSize = 28;

class AudioSynthBraids: public AudioStream
{
public:
	AudioSynthBraids();
	virtual void update(void);

	void setPitch(int16_t pitch) { m_osc.set_pitch(pitch); }
	void setParams(int16_t p1, int16_t p2) { m_osc.set_parameters(p1, p2); }
	void setShape(uint8_t shape);

	void trigger() { m_osc.Strike(); }
private:
	braids::MacroOscillator m_osc;
	uint8_t m_sync_buffer[kAudioBlockSize];
};
#endif

#endif /* _SYNTH_BRAIDS_H_ */

