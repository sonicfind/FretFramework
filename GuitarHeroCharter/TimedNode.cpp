#include "TimedNode.h"

inline void Fret::init(uint32_t sustain)
{
	m_isActive = true;
	m_sustain = sustain;
}

inline void Fret::write_chart(uint32_t position, int lane, std::ofstream& outFile, char type) const
{
	outFile << "  " << position << " = " << type << " " << lane << ' ' << m_sustain << "\n";
}

inline void Fret::write_chart2(uint32_t position, int lane, std::ofstream& outFile, char type) const
{
	outFile << "  " << position << " = " << type << " " << lane << ' ' << m_sustain << "\n";
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

void DrumPad_Pro::write_chart(uint32_t position, int lane, std::ofstream& outFile) const
{
	Fret::write_chart(position, lane, outFile);
	if (m_isCymbal)
		outFile << "  " << position << " = N " << lane + 64 << " 0\n";
}

void DrumPad_Pro::write_chart2(uint32_t position, int lane, std::ofstream& outFile) const
{
	if (m_sustain || !m_isCymbal)
		Fret::write_chart2(position, lane, outFile);

	if (m_isCymbal)
		outFile << "  " << position << " = M C " << lane << '\n';
}

bool DrumPad_Pro::activateModifier(char modifier)
{
	if (modifier == 'c' || modifier == 'C')
	{
		m_isCymbal = true;
		return true;
	}
	return DrumPad::activateModifier(modifier);
}

void DrumPad_Bass::write_chart(uint32_t position, int lane, std::ofstream& outFile) const
{
	outFile << "  " << position << " = N 0 0\n";
	if (m_isDoubleBass)
		outFile << "  " << position << " = N 32 0\n";
}

void DrumPad_Bass::write_chart2(uint32_t position, int lane, std::ofstream& outFile) const
{
	if (m_isDoubleBass)
		outFile << "  " << position << " = M X\n";
	else
		outFile << "  " << position << " = M K\n";
}

// Pulls values from a V1 .chart file
// Returns whether a valid value could be utilized
bool GuitarNote_5Fret::init_chart(size_t lane, uint32_t sustain)
{
	if (!GuitarNote<5>::init_chart(lane, sustain) && lane >= 8)
		return false;
	else if (lane < 5)
		m_colors[lane].init(sustain);
	return true;
}

void GuitarNote_5Fret::write_chart(const uint32_t position, std::ofstream& outFile) const
{
	for (int lane = 0; lane < 5; ++lane)
		if (m_colors[lane])
			m_colors[lane].write_chart(position, lane, outFile);

	GuitarNote::write_chart(position, outFile);
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

void GuitarNote_6Fret::write_chart(const uint32_t position, std::ofstream& outFile) const
{
	for (int lane = 0; lane < 5; ++lane)
		if (m_colors[lane])
			if (lane != 2)
				m_colors[lane].write_chart(position, lane < 3 ? lane + 3 : lane - 3, outFile);
			else
				m_colors[2].write_chart(position, 8, outFile);

	GuitarNote::write_chart(position, outFile);
}
