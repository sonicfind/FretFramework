#include "Sustainable.h"
#include "Variable Types/WebType.h"

uint16_t Sustainable::s_forceThreshold = 64;
uint16_t Sustainable::s_sustainThreshold = 64;
void Sustainable::save_cht(int lane, std::stringstream& buffer) const
{
	buffer << ' ' << lane;
	if (m_sustain > 0)
		buffer << "~ " << m_sustain;
}

void Sustainable::save_bch(int lane, char*& outPtr) const
{
	if (m_sustain > 0)
	{
		*outPtr++ = (char)lane | 128;
		WebType::copyToBuffer(m_sustain, outPtr);
	}
	else
		*outPtr++ = (char)lane;
}
