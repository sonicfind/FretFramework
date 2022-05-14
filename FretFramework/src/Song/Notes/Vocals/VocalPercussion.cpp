#include "VocalPercussion.h"

VocalPercussion::VocalPercussion()
{
	m_isPlayable = true;
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

void VocalPercussion::save_modifier_cht(std::fstream& outFile) const
{
	if (!m_isPlayable)
		outFile << " N";
}

void VocalPercussion::save_modifier_cht(int lane, std::fstream& outFile) const
{
	if (!m_isPlayable)
		outFile << " N";
}
