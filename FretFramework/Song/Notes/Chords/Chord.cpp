#include "Chord.h"

template<>
void Chord<6>::init_chartV1(int lane, uint32_t sustain)
{
	// The original .chart format is a jumbled mess
	if (lane == 8)
		m_colors[2].init(sustain);
	else if (lane < 3)
		m_colors[lane + 3].init(sustain);
	else if (lane < 5)
		m_colors[lane - 3].init(sustain);
	else if (!checkModifiers(lane, sustain))
		throw InvalidNoteException(lane);
}