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
	void setShape(uint8_t shape);
	void setParam1(int16_t p) { m_p1 = p; m_osc.set_parameters(m_p1, m_p2); }
	void setParam2(int16_t p) { m_p2 = p; m_osc.set_parameters(m_p1, m_p2); }

	void trigger() { m_osc.Strike(); }
private:
	braids::MacroOscillator m_osc;
	int16_t m_p1;
	int16_t m_p2;
	uint8_t m_sync_buffer[kAudioBlockSize];
};
#endif

#endif /* _SYNTH_BRAIDS_H_ */

