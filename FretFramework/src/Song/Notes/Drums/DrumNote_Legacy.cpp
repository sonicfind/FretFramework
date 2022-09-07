#include "DrumNote_cht.hpp"

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

DrumNote<4, DrumPad_Pro> DrumNote_Legacy::toDrum4Pro() const
{
	DrumNote<4, DrumPad_Pro> note;
	memcpy(note.m_colors, m_colors, sizeof(DrumPad_Pro) * 4);
	note.m_special = m_special;
	note.m_isFlamed = m_isFlamed;
	return note;
}

DrumNote<5, DrumPad> DrumNote_Legacy::toDrum5()  const
{
	DrumNote<5, DrumPad> note;
	for (int i = 0; i < 5; ++i)
		note.m_colors[i] = m_colors[i];
	note.m_special = m_special;
	note.m_isFlamed = m_isFlamed;
	return note;
}
