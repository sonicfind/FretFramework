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

void DrumNote::init(unsigned char lane, uint32_t sustain)
{
	if (lane == 5)
	{
		s_is5Lane = true;
		m_fifthLane.init(sustain);
	}
	else
		Note<4, DrumPad_Pro, DrumPad_Bass>::init(lane, sustain);
}

void DrumNote::modify(char modifier, unsigned char lane)
{
	switch (modifier)
	{
	case 'F':
		m_isFlamed.toggle();
		break;
	default:
		if (lane == 0)
			m_special.modify(modifier);
		else if (lane == 5)
			m_fifthLane.modify(modifier);
		else if (lane < 5)
			m_colors[lane - 1].modify(modifier);
	}
}

void DrumNote::resetLaning()
{
	s_is5Lane = false;
}
