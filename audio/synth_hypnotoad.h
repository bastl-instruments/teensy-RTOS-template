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

#ifndef synth_hypnotoad_h_
#define synth_hypnotoad_h_

#include "AudioStream.h"
#include "arm_math.h"
#include "src/logger.h"

// TODO: investigate making a high resolution sine wave
// using Taylor series expansion.
// http://www.musicdsp.org/showone.php?id=13
//

typedef struct _sine_nfo {
	_sine_nfo() : phase_acc(0), phase_inc(0), magnitude (16384) {}
	uint32_t phase_acc;
	uint32_t phase_inc;
	int32_t magnitude;
} sine_nfo_t;

#define NSINES	1

class AudioSynthWaveformHypnotoad : public AudioStream
{
public:
	AudioSynthWaveformHypnotoad() : AudioStream(0, NULL), m_nfo() {}
	void frequency(float freq) {
		if (freq < 0.0) freq = 0.0;
		else if (freq > AUDIO_SAMPLE_RATE_EXACT/2) freq = AUDIO_SAMPLE_RATE_EXACT/2;
		for(int i = 0; i < NSINES; i++) {
			m_nfo[i].phase_inc =   freq * (4294967296.0 / AUDIO_SAMPLE_RATE_EXACT);
//            freq = freq * 1.03;
		}
	}
	void phase(float angle) {
		if (angle < 0.0) angle = 0.0;
		else if (angle > 360.0) {
			angle = angle - 360.0;
			if (angle >= 360.0) return;
		}
		for(int i = 0; i < NSINES; i++) {
			m_nfo[i].phase_acc = angle * (4294967296.0 / 360.0);
		}
	}
	void amplitude(float n) {
		if (n < 0) n = 0;
		else if (n > 1.0) n = 1.0;
		for(int i = 0; i < NSINES; i++) {
			m_nfo[i].magnitude = 65536.0;
//            m_nfo[i].magnitude = n* (65536.0 / NSINES);
		}
	}
	virtual void update(void);
private:
	void addSineToBlock(uint32_t n, audio_block_t *block);
	sine_nfo_t m_nfo[NSINES];
};


#endif
