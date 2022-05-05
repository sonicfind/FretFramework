#include "Sustainable.h"

uint16_t Sustainable::s_forceThreshold = 160;
uint16_t Sustainable::s_sustainThreshold = 160;
void Sustainable::save_cht(int lane, std::fstream& outFile) const
{
	if (m_sustain > 0)
		outFile << ' ' << (lane | 128) << ' ' << m_sustain;
	else
		outFile << ' ' << lane;
}
