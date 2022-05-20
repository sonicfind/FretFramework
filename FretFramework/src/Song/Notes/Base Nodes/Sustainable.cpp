#include "Sustainable.h"

uint16_t Sustainable::s_forceThreshold = 64;
uint16_t Sustainable::s_sustainThreshold = 64;
void Sustainable::save_cht(int lane, std::stringstream& buffer) const
{
	if (m_sustain > 0)
		buffer << ' ' << (lane | 128) << ' ' << m_sustain;
	else
		buffer << ' ' << lane;
}

void Sustainable::save_bch(int lane, char*& outPtr) const
{
	if (m_sustain > 0)
	{
		*outPtr++ = (char)lane | 128;
		m_sustain.copyToBuffer(outPtr);
	}
	else
		*outPtr++ = (char)lane;
}
