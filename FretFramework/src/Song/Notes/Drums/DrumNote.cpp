#include "DrumNote.h"
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

bool DrumNote::init(int lane, uint32_t sustain)
{
	if (!Note<4, DrumPad_Pro, DrumPad_Bass>::init(lane, sustain))
	{
		s_is5Lane = true;
		m_fifthLane.init(sustain);
	}
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
		return m_special.modify(modifier);
	else if (lane < 5)
		return m_colors[lane - 1].modify(modifier);
	else
		return m_fifthLane.modify(modifier);
}

void DrumNote::resetLaning()
{
	s_is5Lane = false;
}
