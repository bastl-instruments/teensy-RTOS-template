/**
* @brief 
* @file utils.cpp
* @author J.H. 
* @date 2016-01-25
*/

/* module header */
#include "utils.h"

/* system includes C */

/* system includes C++ */


/* local includes */
#include "utility/dspinst.h"

extern "C" {
extern const int16_t AudioWaveformSine[257];
}

int16_t DDS::next()
{
	uint16_t index;
	uint32_t scale;
	int32_t val1, val2;
	uint32_t mag = 1;
	short tmp_amp = 1;
	int16_t ret = 0;

	if((m_cycles_max > 0) && (m_cycles >= m_cycles_max)) return 0;
	
	switch(m_waveform) {
		case SINE:
			index = m_acc >> 24;
			scale;
			val1 = AudioWaveformSine[index];
			val2 = AudioWaveformSine[index+1];
			scale = (m_acc >> 8) & 0xFFFF;
			val2 *= scale;
			val1 *= 0xFFFF - scale;
			//block->data[i] = (((val1 + val2) >> 16) * magnitude) >> 16;
			if((m_acc + m_inc) < m_acc) m_cycles++;
			m_acc += m_inc;
			ret = multiply_32x32_rshift32(val1 + val2, m_mag);
		break;
		case SQUARE:
			if(m_acc & 0x40000000) ret = m_mag;
			else ret = -m_mag;
			m_acc += m_inc;
		break;
		case SAW:
			ret = ((short)(m_acc>>15)*m_mag) >> 15;
			m_acc -= m_inc;
		break;
		case TRIANGLE:
			if(m_acc & 0x80000000) 	tmp_amp = m_mag;
			else 					tmp_amp = -m_mag;
			mag = m_acc << 2;
			// Determine which quadrant
			if(m_acc & 0x40000000)	mag = ~mag + 1;
			ret = ((short)(mag>>17)*tmp_amp) >> 15;
			if((m_acc + (2*m_inc)) < m_acc) m_cycles++;
			m_acc += 2*m_inc;
		break;
		default: break;
	}

	return ret;
}
