#include "GuitarNote.h"

template<>
void GuitarNote<5>::init_chartV1(const unsigned char lane, const uint32_t sustain)
{
	if (lane < 5)
		m_colors[lane].init(sustain);
	else if (lane == 7)
		m_special.init(sustain);
	else if (!checkModifiers(lane))
		throw InvalidNoteException(lane);
}

template<>
void GuitarNote<6>::init_chartV1(const unsigned char lane, const uint32_t sustain)
{
	// The original .chart format is a jumbled mess
	if (lane < 5)
	{
		static constexpr int lanes[5] = { 3, 4, 5, 0, 1 };
		m_colors[lanes[lane]].init(sustain);
	}
	else if (lane == 8)
		m_colors[2].init(sustain);
	else if (lane == 7)
		m_special.init(sustain);
	else if (!checkModifiers(lane))
		throw InvalidNoteException(lane);
}
