#include "Drums.h"

bool DrumPad::modify(char modifier)
{
	switch (modifier)
	{
	case 'a':
	case 'A':
		m_isAccented.toggle();
		if (m_isAccented)
			m_isGhosted = false;
		break;
	case 'g':
	case 'G':
		m_isGhosted.toggle();
		if (m_isGhosted)
			m_isAccented = false;
		break;
	default:
		return false;
	}
	return true;
}

void DrumPad::save_modifier_cht(std::fstream& outFile) const
{
	if (m_isAccented)
		outFile << " A";
	else if (m_isGhosted)
		outFile << " G";
}

void DrumPad::save_modifier_cht(int lane, std::fstream& outFile) const
{
	if (m_isAccented)
		outFile << " A " << lane;
	else if (m_isGhosted)
		outFile << " G " << lane;
}

void DrumPad::save_modifier_bch(char* modifier) const
{
	if (m_isAccented)
		modifier[0] |= 4 + 128;
	else if (m_isGhosted)
		modifier[0] |= 8 + 128;
}

bool DrumPad::save_modifier_bch(int lane, char*& outPtr) const
{
	if (!m_isAccented && !m_isGhosted)
		return false;

	if (m_isAccented)
		*outPtr++ |= 4 + 128;
	else
		*outPtr++ |= 8 + 128;

	*outPtr++ = (char)lane;
	return true;
}

bool DrumPad_Pro::modify(char modifier)
{
	switch (modifier)
	{
	case 'c':
	case 'C':
		m_isCymbal.toggle();
		return true;
	default:
		return DrumPad::modify(modifier);
	}
}

void DrumPad_Pro::save_modifier_cht(std::fstream& outFile) const
{
	DrumPad::save_modifier_cht(outFile);
	if (m_isCymbal)
		outFile << " C";
}

void DrumPad_Pro::save_modifier_cht(int lane, std::fstream& outFile) const
{
	DrumPad::save_modifier_cht(lane, outFile);
	if (m_isCymbal)
		outFile << " C " << lane;
}

void DrumPad_Pro::save_modifier_bch(char* modifier) const
{
	DrumPad::save_modifier_bch(modifier);
	if (m_isCymbal)
		modifier[0] |= 16 + 128;
}

bool DrumPad_Pro::save_modifier_bch(int lane, char*& outPtr) const
{
	char* base = outPtr;
	bool modded = DrumPad::save_modifier_bch(lane, outPtr);
	if (m_isCymbal)
	{
		*base++ |= 16 + 128;
		*base++ = (char)lane;
		outPtr = base;
		modded = true;
	}
	return modded;
}

bool DrumPad_Bass::modify(char modifier)
{
	switch (modifier)
	{
	case '+':
		m_isDoubleBass.toggle();
		return true;
	default:
		return false;
	}
}

void DrumPad_Bass::save_modifier_cht(std::fstream& outFile) const
{
	if (m_isDoubleBass)
		outFile << " +";
}

void DrumPad_Bass::save_modifier_cht(int lane, std::fstream& outFile) const
{
	if (m_isDoubleBass)
		outFile << " +";
}

void DrumPad_Bass::save_modifier_bch(char* modifier) const
{
	if (m_isDoubleBass)
		modifier[0] |= 2;
}

bool DrumPad_Bass::save_modifier_bch(int lane, char*& outPtr) const
{
	if (m_isDoubleBass)
	{
		*outPtr++ |= 2;
		return true;
	}
	return false;
}
