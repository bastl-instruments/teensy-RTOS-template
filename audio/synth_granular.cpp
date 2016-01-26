/**
* @brief 
* @file synth_granular.cpp
* @author J.H. 
* @date 2016-01-25
*/

/* module header */
#include "synth_granular.h"

/* system includes C */

/* system includes C++ */


/* local includes */
#include "audio/utils.h"
#include "utility/dspinst.h"


void AudioSynthGranular::update(void)
{
	audio_block_t *block;

	block = allocate();
	if (block) {
		for (int i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
			m_source.m_mag = m_mod.next();
//            m_source.setFrequency((m_mod.next() + 16384) /8);
			block->data[i] = m_source.next();
		}
		
		transmit(block);
		release(block);
		return;
	}
}
