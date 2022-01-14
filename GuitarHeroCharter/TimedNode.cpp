#include "TimedNode.h"

// Pulls values from a V1 .chart file
// Returns whether a valid value could be utilized
bool GuitarNote_5Fret::init_chart(size_t lane, uint32_t sustain)
{
	if (!GuitarNote<5>::init_chart(lane, sustain) && lane >= 5)
		return false;

	m_colors[lane].init(sustain);
	return true;
}

// Pulls values from a V1 .chart file
// Returns whether a valid value could be utilized
bool GuitarNote_6Fret::init_chart(size_t lane, uint32_t sustain)
{
	if (!GuitarNote<6>::init_chart(lane, sustain))
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

inline void Fret::init(uint32_t sustain)
{
	m_isActive = true;
	m_sustain = sustain;
}

bool DrumPad::activateModifier(char modifier)
{
	switch (modifier)
	{
	case 'f':
	case 'F':
		m_isFlamed = true;
		break;
	case 'a':
	case 'A':
		m_isAccented = true;
		break;
	case 'g':
	case 'G':
		m_isGhosted = true;
		break;
	default:
		return false;
	}
	return true;
}

bool DrumPad_Pro::activateModifier(char modifier)
{
	if (modifier != 'c' && modifier != 'C')
		return DrumPad::activateModifier(modifier);
	else
		m_isCymbal = true;
    return true;
}
