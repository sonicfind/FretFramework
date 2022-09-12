#include "Drums.h"

void DrumPad::modify(char modifier)
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
	}
}

void DrumPad::modify_binary(char modifier)
{
	if (modifier & 4)
	{
		m_isAccented.toggle();
		if (m_isAccented)
			m_isGhosted = false;
	}
	else if (modifier & 8)
	{
		m_isGhosted.toggle();
		if (m_isGhosted)
			m_isAccented = false;
	}
}

int DrumPad::save_modifier_cht(std::stringstream& buffer) const
{
	if (!m_isAccented && !m_isGhosted)
		return 0;

	if (m_isAccented)
		buffer << " A";
	else
		buffer << " G";
	return 1;
}

int DrumPad::save_modifier_cht(int lane, std::stringstream& buffer) const
{
	if (!m_isAccented && !m_isGhosted)
		return 0;

	if (m_isAccented)
		buffer << " A " << lane;
	else
		buffer << " G " << lane;
	return 1;
}

void DrumPad::save_modifier_bch(char* buffer) const
{
	if (m_isAccented)
		buffer[0] |= 4 + 128;
	else if (m_isGhosted)
		buffer[0] |= 8 + 128;
}

bool DrumPad::save_modifier_bch(int lane, char*& buffer) const
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

void DrumPad_Pro::modify(char modifier)
{
	switch (modifier)
	{
	case 'c':
	case 'C':
		m_isCymbal.toggle();
		break;
	default:
	DrumPad::modify(modifier);
	}
}

void DrumPad_Pro::modify_binary(char modifier)
{
	DrumPad::modify_binary(modifier);
	if (modifier & 16)
		m_isCymbal.toggle();
}

int DrumPad_Pro::save_modifier_cht(std::stringstream& buffer) const
{
	int numMods = DrumPad::save_modifier_cht(buffer);
	if (m_isCymbal)
	{
		buffer << " C";
		++numMods;
	}
	return numMods;
}

int DrumPad_Pro::save_modifier_cht(int lane, std::stringstream& buffer) const
{
	int numMods = DrumPad::save_modifier_cht(lane, buffer);
	if (m_isCymbal)
	{
		buffer << " C " << lane;
		++numMods;
	}
	return numMods;
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

void DrumPad_Bass::modify(char modifier)
{
	switch (modifier)
	{
	case '+':
		m_isDoubleBass.toggle();
	}
}

void DrumPad_Bass::modify_binary(char modifier)
{
	if (modifier & 2)
		m_isDoubleBass.toggle();
}

int DrumPad_Bass::save_modifier_cht(std::stringstream& buffer) const
{
	if (m_isDoubleBass)
	{
		buffer << " +";
		return 1;
	}
	return 0;
}

int DrumPad_Bass::save_modifier_cht(int lane, std::stringstream& buffer) const
{
	if (m_isDoubleBass)
	{
		buffer << " +" << lane;
		return 1;
	}
	return 0;
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
