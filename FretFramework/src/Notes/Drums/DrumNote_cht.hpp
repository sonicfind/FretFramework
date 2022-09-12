#pragma once
#include "DrumNote.h"

template<int numPads, class PadType>
inline void DrumNote<numPads, PadType>::init_chartV1(unsigned char lane, uint32_t sustain)
{
	if (lane == 0)
		m_special.init(sustain);
	else if (lane <= numPads)
		m_colors[lane - 1].init(sustain);
	else if (lane == 32)
		m_special.m_isDoubleBass = true;
	else if (lane >= 66 && lane <= 68)
		m_colors[lane - 65].modify('C');
	else
		throw InvalidNoteException(lane);
}

template<int numPads, class PadType>
inline int DrumNote<numPads, PadType>::write_modifiers_single(std::stringstream& buffer) const
{
	int numMods = 0;
	if (m_special)
		numMods = m_special.save_modifier_cht(buffer);
	else
	{
		if (m_isFlamed)
		{
			buffer << " F";
			numMods = 1;
		}

		int i = 0;
		while (!m_colors[i])
			++i;
		numMods += m_colors[i].save_modifier_cht(buffer);
	}
	return numMods;
}

template<int numPads, class PadType>
inline int DrumNote<numPads, PadType>::write_modifiers_chord(std::stringstream& buffer) const
{
	int numMods = 0;
	if (m_isFlamed)
	{
		buffer << " F";
		numMods = 1;
	}

	if (m_special)
		numMods += m_special.save_modifier_cht(0, buffer);

	int i = 0;
	while (!m_colors[i])
		++i;
	numMods += m_colors[i].save_modifier_cht(i + 1, buffer);
	return numMods;
}
