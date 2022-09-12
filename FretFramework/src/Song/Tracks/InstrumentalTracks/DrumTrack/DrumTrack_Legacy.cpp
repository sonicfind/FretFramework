#include "DrumTrack_Legacy.h"
unsigned char InstrumentalTrack<DrumNote_Legacy>::s_starPowerReadNote = 116;

template <>
template <>
InstrumentalTrack<DrumNote<4, DrumPad_Pro>>& InstrumentalTrack<DrumNote<4, DrumPad_Pro>>::operator=(InstrumentalTrack<DrumNote_Legacy>&& track)
{
	for (int i = 0; i < 5; ++i)
		if (!m_difficulties[i].occupied())
			m_difficulties[i] = std::move(track.m_difficulties[i]);
	return *this;
}

template <>
template <>
InstrumentalTrack<DrumNote<5, DrumPad>>& InstrumentalTrack<DrumNote<5, DrumPad>>::operator=(InstrumentalTrack<DrumNote_Legacy>&& track)
{
	for (int i = 0; i < 5; ++i)
		if (!m_difficulties[i].occupied())
			m_difficulties[i] = std::move(track.m_difficulties[i]);
	return *this;
}

void InstrumentalTrack<DrumNote_Legacy>::load_chart_V1(int diff, TextTraversal& traversal)
{
	m_difficulties[diff].load_chart_V1(traversal);
}
