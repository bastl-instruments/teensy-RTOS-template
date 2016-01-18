/**
* @brief 
* @file synth_lorenz.cpp
* @author J.H. 
* @date 2016-01-18
*/

/* module header */
#include "synth_lorenz.h"

/* system includes C */

/* system includes C++ */


/* local includes */
#include "utility/dspinst.h"



void AudioSynthLorenz::update()
{
	audio_block_t *block;

	block = allocate();
	if (block) {
		float rx, ry, rz;
		for (int i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
			rx = m_x + ((m_sigma * m_y - m_sigma * m_x) * m_dt);
			ry = m_y + ((-m_x*m_z + m_ro*m_x - m_y) * m_dt);
			rz = m_z + ((m_x*m_y - m_beta*m_z) * m_dt);

			block->data[i] = (rx / 40)  * 65536;

			m_x = rx;
			m_y = ry;
			m_z = rz;
		}
		transmit(block);
		release(block);
	}
}

void AudioSynthLorenz::reset()
{
	m_x = 1.2;
	m_y = 2.3;
	m_z = 4.4;

	m_sigma = 10;
	m_ro = 18;
	m_beta = 8.0/3;
	m_dt = 0.01;
}
