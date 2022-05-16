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

void VocalPercussion::save_cht(std::fstream& outFile) const
{
	Hittable::save_cht(0, outFile);
	if (!m_isPlayable)
		outFile << " N";
	outFile << '\n';
}

void VocalPercussion::save_modifier_cht(int lane, std::fstream& outFile) const
{
	if (!m_isPlayable)
		outFile << " N";
}
