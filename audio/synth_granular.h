#ifndef _SYNTH_GRANULAR_H_
#define _SYNTH_GRANULAR_H_
/* system includes */
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

#define NSINES_MAX 4

#include "AudioStream.h"
#include "arm_math.h"
#include "utils.h"


class AudioSynthGranular: public AudioStream
{
public:
	AudioSynthGranular() : AudioStream(0, NULL), m_source() {m_source.setType(DDS::TRIANGLE); m_mod.setType(DDS::TRIANGLE); }
	virtual void update(void);
	void setFrequency(float f) { m_source.setFrequency(f); }
	void setModFrequency(float f) { m_mod.setFrequency(f); }
	void trigger() { m_mod.m_cycles_max = 1; m_mod.m_cycles = 0; }
private:
	DDS	m_source;
	DDS	m_mod;
};
#endif

#endif /* _SYNTH_GRANULAR_H_ */

