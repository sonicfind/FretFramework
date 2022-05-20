#include "DrumNote_cht.hpp"

bool DrumNote_Legacy::s_is5Lane = false;
void DrumNote_Legacy::init_chartV1(unsigned char lane, uint32_t sustain)
{
	DrumNote<5, DrumPad_Pro>::init_chartV1(lane, sustain);
	if (lane == 5)
		s_is5Lane = true;
}

void DrumNote_Legacy::init(unsigned char lane, uint32_t sustain)
{
	DrumNote<5, DrumPad_Pro>::init(lane, sustain);
	if (lane == 5)
		s_is5Lane = true;
}

void DrumNote_Legacy::modify(char modifier, unsigned char lane)
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

void DrumNote_Legacy::modify_binary(char modifier, unsigned char lane)
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

void DrumNote_Legacy::resetLaning()
{
	s_is5Lane = false;
}

void DrumNote_Legacy::convert(DrumNote<4, DrumPad_Pro>& note) const
{
	note.m_special = m_special;
	memcpy(note.m_colors, m_colors, sizeof(DrumPad_Pro) * 4);
	note.m_isFlamed = m_isFlamed;
}

void DrumNote_Legacy::convert(DrumNote<5, DrumPad>& note) const
{
	note.m_special = m_special;
	for (int i = 0; i < 5; ++i)
		note.m_colors[i] = m_colors[i];
	note.m_isFlamed = m_isFlamed;
}
