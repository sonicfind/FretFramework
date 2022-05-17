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
}

void VocalPercussion::save_bch(std::fstream& outFile) const
{
	outFile.put(6);
	if (m_isPlayable)
	{
		outFile.put(1);
		outFile.put(0);
	}
	else
	{
		outFile.put(2);
		outFile.put(0);
		outFile.put(1);
	}
}
