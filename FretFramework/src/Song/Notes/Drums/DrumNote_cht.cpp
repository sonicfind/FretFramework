#include "InstrumentalNote_cht.hpp"
#include "DrumNote.h"

template<>
void DrumNote<4, DrumPad_Pro>::init_chartV1(const unsigned char lane, const uint32_t sustain)
{
	if (lane == 0)
		m_special.init(sustain);
	else if (lane <= 4)
		m_colors[lane - 1].init(sustain);
	else if (lane == 32)
		m_special.m_isDoubleBass = true;
	else if (lane >= 66 && lane <= 68)
		m_colors[lane - 65].modify('C');
	else
		throw InvalidNoteException(lane);
}

template<>
void DrumNote<5, DrumPad>::init_chartV1(const unsigned char lane, const uint32_t sustain)
{
	if (lane == 0)
		m_special.init(sustain);
	else if (lane <= 5)
		m_colors[lane - 1].init(sustain);
	else if (lane == 32)
		m_special.m_isDoubleBass = true;
	else
		throw InvalidNoteException(lane);
}
