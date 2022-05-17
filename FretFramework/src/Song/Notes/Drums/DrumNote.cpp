#include "DrumNote.h"
bool DrumNote::s_is5Lane = false;
void DrumNote::checkFlam()
{
	int numActive = 0;
	for (int i = 0; i < 4; ++i)
		if (m_colors[i])
			++numActive;
	m_isFlamed = numActive < 2;
}

void DrumNote::init(unsigned char lane, uint32_t sustain)
{
	Note<5, DrumPad_Pro, DrumPad_Bass>::init(lane, sustain);
	if (lane == 5)
		s_is5Lane = true;
}

void DrumNote::modify(char modifier, unsigned char lane)
{
	switch (modifier)
	{
	case 'F':
	case 'f':
		m_isFlamed.toggle();
		break;
	default:
		if (lane == 0)
			m_special.modify(modifier);
		else if (lane < 5)
			m_colors[lane - 1].modify(modifier);
		else if (lane == 5)
			reinterpret_cast<DrumPad*>(&m_colors[4])->modify(modifier);
	}
}

void DrumNote::modify_binary(char modifier, unsigned char lane)
{
	if (modifier & 1)
		m_isFlamed.toggle();

	if (modifier < 0)
	{
		if (lane == 0)
			m_special.modify_binary(modifier);
		else if (lane < 5)
			m_colors[lane - 1].modify_binary(modifier);
		else if (lane == 5)
			reinterpret_cast<DrumPad*>(&m_colors[4])->modify_binary(modifier);
	}
}

void DrumNote::resetLaning()
{
	s_is5Lane = false;
}
