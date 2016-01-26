#ifndef _SYNTH_GRANULAR_H_
#define _SYNTH_GRANULAR_H_
/* system includes */
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}


#include "AudioStream.h"
#include "arm_math.h"
#include "utils.h"

#define LAYERS_MAX	8

class AudioSynthGranular: public AudioStream
{
public:
	AudioSynthGranular();
	virtual void update(void);
	void setFrequency(float f);
	void setModFrequency(float f);
	void trigger();
	void setRepeat(uint8_t r);

	void setActiveLayers(uint8_t n);
private:
	DDS	m_source[LAYERS_MAX];
	DDS	m_mod[LAYERS_MAX];

	uint8_t m_activeLayers;
	uint8_t m_repeat;
};
#endif

#endif /* _SYNTH_GRANULAR_H_ */

