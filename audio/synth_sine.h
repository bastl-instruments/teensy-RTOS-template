/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef synth_sine_h_
#define synth_sine_h_

#include "AudioStream.h"
#include "arm_math.h"
#include "utils.h"

// TODO: investigate making a high resolution sine wave
// using Taylor series expansion.
// http://www.musicdsp.org/showone.php?id=13

#define NSINES_MAX 1

class AudioSynthWaveformSine : public AudioStream
{
public:
	AudioSynthWaveformSine() : AudioStream(0, NULL), m_nfo(), m_nsines(1) {}
	void frequency(float freq) {
		if (freq < 0.0) freq = 0.0;
		else if (freq > AUDIO_SAMPLE_RATE_EXACT/2) freq = AUDIO_SAMPLE_RATE_EXACT/2;
		for(int i = 0; i < NSINES_MAX; i++) {
			m_nfo[i].phase_inc = freq * (4294967296.0 / AUDIO_SAMPLE_RATE_EXACT);
			freq *= m_fInc;
			freq = (freq < 0) ? 0 : (freq > AUDIO_SAMPLE_RATE_EXACT/2) ? AUDIO_SAMPLE_RATE_EXACT*2 : freq;
		}
	}
	void phase(float angle) {
		if (angle < 0.0) angle = 0.0;
		else if (angle > 360.0) {
			angle = angle - 360.0;
			if (angle >= 360.0) return;
		}
		m_nfo[0].phase_acc = angle * (4294967296.0 / 360.0);
	}
	void amplitude(float n) {
		if (n < 0) n = 0;
		else if (n > 1.0) n = 1.0;
		for(int i = 0; i < NSINES_MAX; i++) m_nfo[i].magnitude = n* (65536.0 / NSINES_MAX);
	}
	void nsines(uint8_t n) {
		if(n == 0) n =1;
		else if(n > NSINES_MAX) n = NSINES_MAX;
		m_nsines = n;
	}
	void finc(float f) {
		if(f < 0) f = 1;
		else if(f > 10) f = 10;
		m_fInc = f;
	}
	virtual void update(void);
private:
	void addSineToBlock(uint32_t n, audio_block_t *block);
	sine_nfo_t m_nfo[NSINES_MAX];
	uint8_t m_nsines;
	float m_fInc;
};


#endif
