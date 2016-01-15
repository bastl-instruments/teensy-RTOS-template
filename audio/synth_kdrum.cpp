/**
* @brief 
* @file synth_kdrum.cpp
* @author J.H. 
* @date 2016-01-15
*/

/* module header */
#include "synth_kdrum.h"

/* system includes C */

/* system includes C++ */


/* local includes */

#include "utility/dspinst.h"

extern "C" {
extern const int16_t AudioWaveformSine[257];
}


#define TODO_FREQ_INCREMENT	3

void AudioKDrum::addSineToBlock(uint32_t n, audio_block_t *block)
{
	uint32_t i, ph, index, scale;
	int32_t val1, val2;
	if(block == NULL) return;
	sine_nfo_t *nfo = &m_nfo;
	if(nfo->magnitude) {
		ph = nfo->phase_acc;
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
			index = ph >> 24;
			val1 = AudioWaveformSine[index];
			val2 = AudioWaveformSine[index+1];
			scale = (ph >> 8) & 0xFFFF;
			val2 *= scale;
			val1 *= 0xFFFF - scale;
			//block->data[i] = (((val1 + val2) >> 16) * magnitude) >> 16;
			block->data[i] += multiply_32x32_rshift32(val1 + val2, nfo->magnitude);
			nfo->phase_inc -= m_fInc * (4294967296.0 / AUDIO_SAMPLE_RATE_EXACT);
			ph += nfo->phase_inc;
		}
		nfo->phase_acc = ph;
		return;
	}
	nfo->phase_acc+= nfo->phase_inc* AUDIO_BLOCK_SAMPLES;
}


void AudioKDrum::update(void)
{
	audio_block_t *block;

	block = allocate();
	memset(block, 0, sizeof(audio_block_t));
	if (block) {
		addSineToBlock(0, block);
		transmit(block);
		release(block);
		return;
	}
}
