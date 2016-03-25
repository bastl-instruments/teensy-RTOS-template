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

static int32_t taylor(uint32_t ph)
{
	int32_t angle, sum, p1, p2, p3, p5, p7, p9, p11;

	if (ph >= 0xC0000000 || ph < 0x40000000) {                            // ph:  0.32
		angle = (int32_t)ph; // valid from -90 to +90 degrees
	} else {
		angle = (int32_t)(0x80000000u - ph);                        // angle: 2.30
	}
	p1 =  multiply_32x32_rshift32_rounded(angle, 1686629713) << 2;        // p1:  2.30
	p2 =  multiply_32x32_rshift32_rounded(p1, p1) << 1;                   // p2:  3.29
	p3 =  multiply_32x32_rshift32_rounded(p2, p1) << 2;                   // p3:  3.29
	sum = multiply_subtract_32x32_rshift32_rounded(p1, p3, 1431655765);   // sum: 2.30
	p5 =  multiply_32x32_rshift32_rounded(p3, p2);                        // p5:  6.26
	sum = multiply_accumulate_32x32_rshift32_rounded(sum, p5, 572662306);
	p7 =  multiply_32x32_rshift32_rounded(p5, p2);                        // p7:  9.23
	sum = multiply_subtract_32x32_rshift32_rounded(sum, p7, 109078534);
	p9 =  multiply_32x32_rshift32_rounded(p7, p2);                        // p9: 12.20
	sum = multiply_accumulate_32x32_rshift32_rounded(sum, p9, 12119837);
	p11 = multiply_32x32_rshift32_rounded(p9, p2);                       // p11: 15.17
	sum = multiply_subtract_32x32_rshift32_rounded(sum, p11, 881443);
	return sum <<= 1;                                                 // return:  1.31
}

int16_t DDS::next()
{
	uint16_t index;
	uint32_t scale;
	int32_t val1, val2, val3;
	uint32_t mag = 1;
	short tmp_amp = 1;
	int16_t ret = 0;

	if((m_cycles_max > 0) && (m_cycles >= m_cycles_max)) return 0;
	
	switch(m_waveform) {
		case SINE:
			index = m_acc >> 24;			// first eight bits are index
			val1 = AudioWaveformSine[index];
			val2 = AudioWaveformSine[index+1];
//            val3 = ((val2-val1)*((m_acc>>16)&0xff));
//            val3 = (val3<0) ? val1+((0xffff+val3)>>8) : val1+(val3>>8);
			scale = (m_acc >> 8) & 0xFFFF; // 24 bits are fraction, but we use only 16 of them
			val1 *= 0xFFFF - scale;
			val2 *= scale;
			if((m_acc + m_inc) < m_acc) m_cycles++;
			if(m_backward)	m_acc -= m_inc;
			else			m_acc += m_inc;
			ret = multiply_32x32_rshift32(val1+val2, m_mag);
		break;
		case SINE_HIRES:
			val1 = taylor(m_acc);
			m_acc += m_inc;
			ret = multiply_32x32_rshift32(val1, m_mag);
		break;
		case SQUARE:
			if(m_acc & (2<<30)) ret = (m_mag>>1);
			else ret = -(m_mag>>1);
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
