#include "TimedNode.h"
bool DrumNote::s_is5Lane = false;
void DrumNote::checkFlam()
{
	int numActive = 0;
	for (int i = 0; i < 4; ++i)
		if (m_colors[i])
			++numActive;
	if (m_fifthLane)
		++numActive;

	m_isFlamed = numActive < 2;
}

// Pulls values from a V1 .chart file
// Returns whether a valid value could be utilized
bool DrumNote::initFromChartV1(size_t lane, uint32_t sustain)
{
	if (lane == 0)
		m_open.init(sustain);
	else if (lane < 5)
		m_colors[lane - 1].init(sustain);
	else if (lane == 5)
	{
		s_is5Lane = true;
		m_fifthLane.init(sustain);
	}
	else if (lane == 32)
		m_open.m_isDoubleBass = true;
	else if (lane >= 66 && lane <= 68)
		m_colors[lane - 65].modify('C');
	else
		return false;
	return true;
}

bool DrumNote::init(size_t lane, uint32_t sustain)
{
	if (!Note<4, DrumPad_Pro, DrumPad_Bass>::init(lane, sustain))
	{
		s_is5Lane = true;
		m_fifthLane.init(sustain);
	}
	return true;
}

bool DrumNote::init_chart2_modifier(std::stringstream& ss)
{
	char modifier;
	ss >> modifier;
	if (!modify(modifier))
	{
		switch (modifier)
		{
		case '+':
			return m_open.modify(modifier);
		default:
		{
			int lane;
			ss >> lane;
			return ss && m_colors[lane - 1].modify(modifier);
		}
		}
	}
	return true;
}

bool DrumNote::initFromMid(size_t lane, uint32_t sustain)
{
	if (lane == 5)
	{
		m_fifthLane.init(sustain);
		s_is5Lane = true;
	}
	else if (!Note<4, DrumPad_Pro, DrumPad_Bass>::init(lane, sustain))
		return false;

	if (m_isFlamed)
		checkFlam();
	return true;
}

bool DrumNote::modify(char modifier, bool toggle)
{
	if (modifier == 'F')
	{
		if (!m_isFlamed)
			checkFlam();
		else
			m_isFlamed = false;
		return true;
	}
	return false;
}

bool DrumNote::modifyColor(int lane, char modifier)
{
	if (lane == 0)
		return m_open.modify(modifier);
	else if (lane < 5)
		return m_colors[lane - 1].modify(modifier);
	else
		return m_fifthLane.modify(modifier);
}

void DrumNote::save_chart(const uint32_t position, std::fstream& outFile) const
{
	Note<4, DrumPad_Pro, DrumPad_Bass>::save_chart(position, outFile);
	if (m_isFlamed)
		outFile << "  " << position << " = M F\n";
}

void DrumNote::resetLaning()
{
	s_is5Lane = false;
}
