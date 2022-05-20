#include "Pitched.h"

Pitched::Pitched(const Pitched& other)
{
	m_pitch = other.m_pitch;
	m_sustain = other.m_sustain;
	m_isActive = other.m_isActive;
}

Pitched& Pitched::operator=(const Pitched& other)
{
	m_pitch = other.m_pitch;
	m_sustain = other.m_sustain;
	m_isActive = other.m_isActive;
	return *this;
}

void Pitched::setPitch(char pitch)
{
	m_pitch = pitch;
}

void Pitched::save_pitch_cht(std::fstream& outFile) const
{
		outFile << ' ' << (int)m_pitch << ' ' << m_sustain;
	if (m_isPitched)
}

void Pitched::save_pitch_cht(int lane, std::fstream& outFile) const
{
	outFile << ' ' << lane << ' ' << (int)m_pitch << ' ' << m_sustain;
}

void Pitched::save_pitch_bch(char*& outPtr) const
{
	if (m_isPitched)
	{
		*outPtr++ = m_pitch;
		m_sustain.copyToBuffer(outPtr);
	}
}

bool Pitched::save_pitch_bch(int lane, char*& outPtr) const
{
	if (m_isPitched)
	{
		*outPtr++ = lane;
		*outPtr++ = m_pitch;
		m_sustain.copyToBuffer(outPtr);
		return true;
	}
	return false;
}
