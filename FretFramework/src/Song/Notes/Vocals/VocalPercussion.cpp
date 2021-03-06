#include "VocalPercussion.h"

const char VocalPercussion::s_playableBuffer[3] = { 6, 1, 0 };
const char VocalPercussion::s_noiseBuffer[4] = { 6, 2, 0, 1 };

void VocalPercussion::modify(char modifier)
{
	switch (modifier)
	{
	case 'n':
	case 'N':
		m_isPlayable.toggle();
	}
}

void VocalPercussion::modify_binary(char modifier)
{
	if (modifier & 1)
		m_isPlayable = false;
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
