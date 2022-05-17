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

void DrumNote::modify_binary(char modifier, unsigned char lane)
{
	if (modifier & 1)
		m_isFlamed.toggle();

	if (modifier & 2)
		m_special.m_isDoubleBass.toggle();

	if (modifier < 0 && 0 < lane && lane <= 5)
	{
		Modifiable& mod = lane < 5 ? m_colors[lane - 1] : m_fifthLane;
		if (modifier & 4)
			mod.modify('A');
		else if (modifier & 8)
			mod.modify('G');

		if (modifier & 16)
			mod.modify('C');
	}
}

void DrumNote::resetLaning()
{
	s_is5Lane = false;
}
