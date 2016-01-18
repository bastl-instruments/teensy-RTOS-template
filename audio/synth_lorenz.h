#ifndef _SYNTH_LORENZ_H_
#define _SYNTH_LORENZ_H_
/* system includes */
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#include "AudioStream.h"
#include "arm_math.h"

class AudioSynthLorenz : public AudioStream
{
public:
	AudioSynthLorenz() : AudioStream(0, NULL)  { reset(); }
	void update();
	void reset();

	void setSigma(float f) { m_sigma = f; }
	void setRo(float f) { m_ro = f; }
	void setBeta(float f) { m_beta = f; }
	void setDt(float f) { m_dt = f; }

private:
	volatile float m_x;
	volatile float m_y;
	volatile float m_z;

	volatile float m_sigma 	= 10;
	volatile float m_ro		= 18;
	volatile float m_beta	= 8.0/3;
	volatile float m_dt		= 0.01;
};
#endif

#endif /* _SYNTH_LORENZ_H_ */

