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

bool DrumPad::modify_binary(char modifier)
{
	if (modifier & 4)
	{
		m_isAccented.toggle();
		if (m_isAccented)
			m_isGhosted = false;
		return true;
	}
	else if (modifier & 8)
	{
		m_isGhosted.toggle();
		if (m_isGhosted)
			m_isAccented = false;
		return true;
	}
	return false;
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

void DrumPad::save_modifier_bch(char* buffer) const
{
	if (m_isAccented)
		buffer[0] |= 4 + 128;
	else if (m_isGhosted)
		buffer[0] |= 8 + 128;
}

bool DrumPad::save_modifier_bch(int lane, char*& outPtr) const
{
	if (!m_isAccented && !m_isGhosted)
		return false;

	if (m_isAccented)
		*buffer++ |= 4 + 128;
	else
		*buffer++ |= 8 + 128;

	*buffer++ = (char)lane;
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

bool DrumPad_Pro::modify_binary(char modifier)
{
	bool modded = false;
	if (modifier & 16)
	{
		m_isCymbal.toggle();
		modded = true;
	}
	return DrumPad::modify_binary(modifier) || modded;
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

void DrumPad_Pro::save_modifier_bch(char* buffer) const
{
	DrumPad::save_modifier_bch(buffer);
	if (m_isCymbal)
		buffer[0] |= 16 + 128;
}

bool DrumPad_Pro::save_modifier_bch(int lane, char*& buffer) const
{
	char* base = buffer;
	bool modded = DrumPad::save_modifier_bch(lane, buffer);
	if (m_isCymbal)
	{
		*base++ |= 16 + 128;
		*base++ = (char)lane;
		buffer = base;
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

bool DrumPad_Bass::modify_binary(char modifier)
{
	if (modifier & 2)
	{
		m_isDoubleBass.toggle();
		return true;
	}
	return false;
}

void DrumPad_Bass::save_modifier_cht(std::fstream& outFile) const
{
	if (m_isDoubleBass)
		outFile << " +";
}

void DrumPad_Bass::save_modifier_cht(int lane, std::fstream& outFile) const
{
	if (m_isDoubleBass)
		outFile << " +" << lane;
}

void DrumPad_Bass::save_modifier_bch(char* buffer) const
{
	if (m_isDoubleBass)
		buffer[0] |= 2 + 128;
}

bool DrumPad_Bass::save_modifier_bch(int lane, char*& buffer) const
{
	if (m_isDoubleBass)
	{
		*buffer++ |= 2 + 128;
		*buffer++ = (char)lane;
		return true;
	}
	return false;
}
