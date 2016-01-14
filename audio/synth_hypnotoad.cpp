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

#include "synth_hypnotoad.h"
#include "utility/dspinst.h"

// data_waveforms.c
extern "C" {
extern const int16_t AudioWaveformSine[257];
}


void AudioSynthWaveformHypnotoad::addSineToBlock(uint32_t n, audio_block_t *block)
{
	uint32_t i, ph, inc, index, scale;
	int32_t val1, val2;
	if(block == NULL) return;
	sine_nfo_t *nfo = &m_nfo[n];
	if(nfo->magnitude) {
		ph = nfo->phase_acc;
		inc = nfo->phase_inc;
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
			index = ph >> 24;
			val1 = AudioWaveformSine[index];
			val2 = AudioWaveformSine[index+1];
			scale = (ph >> 8) & 0xFFFF;
			val2 *= scale;
			val1 *= 0xFFFF - scale;
			//block->data[i] = (((val1 + val2) >> 16) * magnitude) >> 16;
			block->data[i] += multiply_32x32_rshift32(val1 + val2, nfo->magnitude);
			ph += inc;
		}
		nfo->phase_acc = ph;
	}
	nfo->phase_acc+= nfo->phase_inc* AUDIO_BLOCK_SAMPLES;
}


void AudioSynthWaveformHypnotoad::update(void)
{
	audio_block_t *block;

	block = allocate();
	if (block) {
		for(int i = 0; i < NSINES; i++) {
			addSineToBlock(i, block);
		}
		transmit(block);
		release(block);
		return;
	}
}


