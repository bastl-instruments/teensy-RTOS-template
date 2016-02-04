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
	memset(block, 0, sizeof(audio_block_t));
	if (block) {
		for (int i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
			for(int j = 0; j < m_activeLayers; j++) {
				
				int16_t n = m_mod[j].next();
				m_source[j].m_mag = (n == 0) ? 1 : n / 4;
//                m_source[j].m_inc = (n >> 2) * 97391;
				block->data[i] += (m_source[j].next());
			}
		}
		
		transmit(block);
		release(block);
		return;
	}
}

void AudioSynthGranular::setFrequency(float f)
{
	for(int i = 0; i < m_activeLayers; i++) {
		m_source[i].setFrequency(f);
		f *= 1.01;
	}
}


void AudioSynthGranular::setModFrequency(float f)
{
	for(int i = 0; i < m_activeLayers; i++) {
		m_mod[i].setFrequency(f);
		f *= 1.02;
	}
}

void AudioSynthGranular::trigger()
{
	for(int i = 0; i < m_activeLayers; i++) {
		m_mod[i].m_cycles_max = m_repeat; 
		m_mod[i].m_cycles = 0; 
	}
}

void AudioSynthGranular::setRepeat(uint8_t r)
{
	m_repeat = r;
}

AudioSynthGranular::AudioSynthGranular() : AudioStream(0, NULL), m_source(), m_activeLayers(LAYERS_MAX)
{
	for(int i = 0; i < m_activeLayers; i++) {
		m_source[i].setType(DDS::SINE); m_mod[i].setType(DDS::TRIANGLE); 
	}
}

void AudioSynthGranular::setActiveLayers(uint8_t n) 
{ 
	if(n == m_activeLayers) return;
	m_activeLayers = (n > LAYERS_MAX) ? LAYERS_MAX : n; 
	for(int i = 0; i < m_activeLayers; i++) {
		m_source[i].m_mag = 16484 / m_activeLayers;
	}
}
