#pragma once
#include "DrumNote.h"

template<int numPads, class PadType>
inline void DrumNote<numPads, PadType>::write_modifiers_single(char*& buffer) const
{
	buffer[0] = 0;
	if (m_special)
		m_special.save_modifier_bch(buffer);
	else
	{
		if (m_isFlamed)
			buffer[0] = 1;

		int i = 0;
		while (!m_colors[i])
			++i;

		m_colors[i].save_modifier_bch(buffer);
	}

	if (buffer[0] > 0)
		++buffer;
}

template<int numPads, class PadType>
inline char DrumNote<numPads, PadType>::write_modifiers_chord(char*& buffer) const
{
	int numMods = 0;
	if (m_isFlamed)
	{
		*buffer++ = 1;
		numMods = 1;
	}

	if (m_special && m_special.save_modifier_bch(0, buffer))
		++numMods;

	for (int i = 0; i < numPads; ++i)
		if (m_colors[i] && m_colors[i].save_modifier_bch(i + 1, buffer))
			++numMods;
	return numMods;
}
