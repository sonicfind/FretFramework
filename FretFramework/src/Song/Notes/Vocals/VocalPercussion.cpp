#include "VocalPercussion.h"

const char VocalPercussion::s_playableBuffer[3] = { 6, 1, 0 };
const char VocalPercussion::s_noiseBuffer[4] = { 6, 2, 0, 1 };

VocalPercussion::VocalPercussion()
{
	m_isPlayable = true;
}

VocalPercussion::VocalPercussion(const VocalPercussion& other)
{
	m_isActive = other.m_isActive;
}

bool VocalPercussion::modify(char modifier)
{
	switch (modifier)
	{
	case 'n':
	case 'N':
		m_isPlayable.toggle();
		return true;
	default:
		return false;
	}
}

const char* VocalPercussion::save_cht() const
{
	if (m_isPlayable)
		return " = N 0\n";
	else
		return " = N 0 1 N\n";
}

void VocalPercussion::save_bch(std::fstream& outFile) const
{
	if (m_isPlayable)
		outFile.write(s_playableBuffer, 3);
	else
		outFile.write(s_noiseBuffer, 4);
}
