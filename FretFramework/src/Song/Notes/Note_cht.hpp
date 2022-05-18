#pragma once
#include "Note.h"

template<int numColors, class NoteType, class SpecialType>
inline void Note<numColors, NoteType, SpecialType>::init_single(TextTraversal& traversal)
{
	// Read note
	uint32_t val;
	if (!traversal.extract(val))
		throw EndofLineException();

	uint32_t sustain = 0;
	if (val >= 128 && !traversal.extract(sustain))
		throw EndofLineException();

	unsigned char color = val & 127;
	init(color, sustain);

	// Read modifiers
	if (traversal.extract(val))
		for (uint32_t i = 0; i < val; ++i)
			modify(traversal.extract(), color);
}

template <int numColors, class NoteType, class SpecialType>
void Note<numColors, NoteType, SpecialType>::init_chord(TextTraversal& traversal)
{
	uint32_t colors;
	if (!traversal.extract(colors))
		throw EndofLineException();

	for (uint32_t i = 0; i < colors; ++i)
	{
		uint32_t lane;
		if (!traversal.extract(lane))
			throw EndofLineException();

		uint32_t sustain = 0;
		if (lane >= 128 && !traversal.extract(sustain))
			throw EndofLineException();

		init(lane & 127, sustain);
	}
}

template<int numColors, class NoteType, class SpecialType>
inline void Note<numColors, NoteType, SpecialType>::modify(TextTraversal& traversal)
{
	uint32_t numMods;
	if (traversal.extract(numMods))
		for (uint32_t i = 0; i < numMods; ++i)
		{
			unsigned char modifier = traversal.extract();
			uint32_t lane = 0;
			// Checks for a possible lane value after the modifier
			traversal.extract(lane);
			modify(traversal.extract(), lane);
		}
}

template <int numColors, class NoteType, class SpecialType>
uint32_t Note<numColors, NoteType, SpecialType>::write_notes(uint32_t position, std::fstream& outFile, const char* const tabs) const
{
	uint32_t numActive = getNumActive();
	if (numActive == 1)
	{
		outFile << tabs << position << " = N";
		if (m_special)
			m_special.save_cht(0, outFile);
		else
		{
			int lane = 0;
			while (!m_colors[lane])
				++lane;

			m_colors[lane].save_cht(lane + 1, outFile);
		}
	}
	else
	{
		outFile << tabs << position << " = C " << numActive;
		if (m_special)
			m_special.save_cht(0, outFile);

		for (int lane = 0; lane < numColors; ++lane)
			if (m_colors[lane])
				m_colors[lane].save_cht(lane + 1, outFile);
	}
	return numActive;
}
