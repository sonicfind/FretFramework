#include "VocalPercussion.h"

VocalPercussion::VocalPercussion()
{
	m_isActive = true;
}

bool VocalPercussion::modify(char modifier)
{
	switch (modifier)
	{
	case 'n':
	case 'N':
		m_isActive.toggle();
		return true;
	default:
		return false;
	}
}

void VocalPercussion::save_modifier_cht(std::fstream& outFile) const
{
	if (!m_isActive)
		outFile << " N";
}

void VocalPercussion::save_modifier_cht(int lane, std::fstream& outFile) const
{
	if (!m_isActive)
		outFile << " N";
}
