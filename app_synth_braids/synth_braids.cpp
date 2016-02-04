/**
* @brief 
* @file synth_braids.cpp
* @author J.H. 
* @date 2016-02-04
*/

/* module header */
#include "synth_braids.h"

/* system includes C */

/* system includes C++ */


/* local includes */

AudioSynthBraids::AudioSynthBraids() : AudioStream(0, NULL), m_osc(), m_sync_buffer()
{
    m_osc.Init();
    m_osc.set_shape(braids::MacroOscillatorShape::MACRO_OSC_SHAPE_CSAW);
    m_osc.set_parameters(0, 0);
}

void AudioSynthBraids::update(void)
{
	audio_block_t *block;

	block = allocate();
	if (block) {
		m_osc.Render(m_sync_buffer, block->data, AUDIO_BLOCK_SAMPLES);
		transmit(block);
		release(block);
		return;
	}
}

void AudioSynthBraids::setShape(uint8_t shape)
{
	braids::MacroOscillatorShape osc_shape = static_cast<braids::MacroOscillatorShape>(shape);//
	m_osc.set_shape(osc_shape);
}
