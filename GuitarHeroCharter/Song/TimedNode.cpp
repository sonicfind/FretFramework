#include "TimedNode.h"
void Hittable::save_chart(uint32_t position, int lane, std::fstream& outFile) const
{
	outFile << "  " << position << " = N " << lane << '\n';
}

void Fret::save_chart(uint32_t position, int lane, std::fstream& outFile) const
{
	outFile << "  " << position << " = N " << lane << ' ' << m_sustain << '\n';
}

bool DrumPad::modify(char modifier)
{
	switch (modifier)
	{
	case 'a':
	case 'A':
		m_isAccented.toggle();
		break;
	case 'g':
	case 'G':
		m_isGhosted.toggle();
		break;
	default:
		return false;
	}
	return true;
}

void DrumPad::save_chart(uint32_t position, int lane, std::fstream& outFile) const
{
	Hittable::save_chart(position, lane, outFile);
	if (m_isAccented)
		outFile << "  " << position << " = M A " << lane << '\n';
	else if (m_isGhosted)
		outFile << "  " << position << " = M G " << lane << '\n';
}

void DrumPad_Pro::save_chart(uint32_t position, int lane, std::fstream& outFile) const
{
	DrumPad::save_chart(position, lane, outFile);
	if (m_isCymbal)
		outFile << "  " << position << " = M C " << lane << '\n';
}

bool DrumPad_Pro::modify(char modifier)
{
	switch(modifier)
	{
	case 'c':
	case 'C':
		if (!m_lockTom)
			m_isCymbal.toggle();
		else
			m_lockTom = false;
		return true;
	case 't':
	case 'T':
		m_lockTom = true;
		m_isCymbal = false;
		return true;
	default:
		return DrumPad::modify(modifier);
	}
}

bool DrumPad_Bass::modify(char modifier)
{
	switch (modifier)
	{
	case 'x':
	case 'X':
		m_isDoubleBass.toggle();
		return true;
	default:
		return false;
	}
}

void DrumPad_Bass::save_chart(uint32_t position, int lane, std::fstream& outFile) const
{
	Hittable::save_chart(position, lane, outFile);
	if (m_isDoubleBass)
		outFile << "  " << position << " = M X\n";
}

// Pulls values from a V1 .chart file
// Returns whether a valid value could be utilized
template<>
bool GuitarNote<5>::initFromChartV1(size_t lane, uint32_t sustain)
{
	if (!checkModifiers(lane, sustain) && lane >= 8)
		return false;
	else if (lane < 5)
		m_colors[lane].init(sustain);
	return true;
}

// Pulls values from a V1 .chart file
// Returns whether a valid value could be utilized
template<>
bool GuitarNote<6>::initFromChartV1(size_t lane, uint32_t sustain)
{
	if (!checkModifiers(lane, sustain))
	{
		// The original .chart format is a jumbled mess
		if (lane == 8)
			m_colors[2].init(sustain);
		else if (lane < 3)
			m_colors[lane + 3].init(sustain);
		else if (lane < 5)
			m_colors[lane - 3].init(sustain);
		else
			return false;
	}
	return true;
}
