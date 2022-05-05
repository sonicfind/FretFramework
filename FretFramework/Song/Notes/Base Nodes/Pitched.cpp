#include "Pitched.h"

void Pitched::setPitch(char pitch)
{
	m_pitch = pitch;
}

void Pitched::save_pitch_cht(std::fstream& outFile) const
{
	outFile << ' ' << (int)m_pitch << ' ' << m_sustain;
}

void Pitched::save_pitch_cht(int lane, std::fstream& outFile) const
{
	outFile << ' ' << lane << ' ' << (int)m_pitch << ' ' << m_sustain;
}
