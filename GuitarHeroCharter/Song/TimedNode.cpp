#include "TimedNode.h"

uint16_t Hittable::s_tickRate = 480;
float Hittable::s_forceThreshold = 160;
void Hittable::save_cht(int lane, std::fstream& outFile) const
{
	outFile << ' ' << lane;
}

void Sustainable::save_cht(int lane, std::fstream& outFile) const
{
	if (m_sustain > 0)
		outFile << ' ' << (lane | 128) << ' ' << m_sustain;
	else
		outFile << ' ' << lane;
}

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

bool DrumPad_Pro::modify(char modifier)
{
	switch(modifier)
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

// Pulls values from a V1 .chart file
// Returns whether a valid value could be utilized
template<>
void GuitarNote<6>::init_chartV1(int lane, uint32_t sustain)
{
	// The original .chart format is a jumbled mess
	if (lane == 8)
		m_colors[2].init(sustain);
	else if (lane < 3)
		m_colors[lane + 3].init(sustain);
	else if (lane < 5)
		m_colors[lane - 3].init(sustain);
	else if (!checkModifiers(lane, sustain))
		throw InvalidNoteException(lane);
}
