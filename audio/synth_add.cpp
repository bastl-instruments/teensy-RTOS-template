/**
* @brief 
* @file synth_add.cpp
* @author J.H. 
* @date 2016-01-19
*/

/* module header */
#include "synth_add.h"

/* system includes C */

/* system includes C++ */


/* local includes */

#include "utility/dspinst.h"

extern "C" {
extern const int16_t AudioWaveformSine[257];
}

void AudioSynthAdditive::addSineToBlock(uint32_t n, audio_block_t *block)
{
	uint32_t i, ph, inc, index, scale;
	int32_t val1, val2;
	if(block == NULL) return;
	sine_nfo_t *nfo = &m_nfo[n];
	if(nfo->phase_inc == 0) { return ; }
	if(nfo->magnitude == 0) {
		nfo->phase_acc += nfo->phase_inc * AUDIO_BLOCK_SAMPLES;
		return;
	}

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
	nfo->phase_acc=  ph;

}

void AudioSynthAdditive::update(void)
{
	audio_block_t *block;

	block = allocate();
	memset(block, 0, sizeof(audio_block_t));
	if (block) {
		for(int i = 0; i < NSINES_MAX; i++) addSineToBlock(i, block);
		transmit(block);
		release(block);
		return;
	}
}


