#pragma once
#include "Note.h"
#include "../VariableLengthQuantity.h"
#include "../WebType.h"

template<int numColors, class NoteType, class SpecialType>
inline void Note<numColors, NoteType, SpecialType>::init_single(BinaryTraversal& traversal)
{
	// Read note
	unsigned char val;
	if (!traversal.extract(val))
		throw EndofEventException();

	uint32_t sustain = 0;
	if (val >= 128 && !traversal.extractVarType(sustain))
		throw EndofEventException();

	unsigned char color = val & 127;
	init(color, sustain);

	// Read modifiers
	if (traversal.extract(val))
		modify_binary(val, color);
}

template<int numColors, class NoteType, class SpecialType>
inline void Note<numColors, NoteType, SpecialType>::init_chord(BinaryTraversal& traversal)
{
	unsigned char colors;
	if (!traversal.extract(colors))
		throw EndofEventException();

	for (unsigned char i = 0; i < colors; ++i)
	{
		unsigned char lane;
		if (!traversal.extract(lane))
			throw EndofEventException();

		uint32_t sustain = 0;
		if (lane >= 128 && !traversal.extractVarType(sustain))
			throw EndofEventException();

		init(lane & 127, sustain);
	}
}

template<int numColors, class NoteType, class SpecialType>
inline void Note<numColors, NoteType, SpecialType>::modify(BinaryTraversal& traversal)
{
	unsigned char numMods;
	if (traversal.extract(numMods))
	{
		unsigned char modifier;
		for (char i = 0; i < numMods && traversal.extract(modifier); ++i)
		{
			unsigned char lane = 0;
			if (modifier >= 128 && !traversal.extract(lane))
				break;
			modify_binary(modifier, lane);
		}
	}
}

template<int numColors, class NoteType, class SpecialType>
inline char Note<numColors, NoteType, SpecialType>::write_notes(char*& dataPtr) const
{
	char numActive = 0;
	if (m_special)
	{
		m_special.save_bch(0, dataPtr);
		++numActive;
	}

	for (int lane = 0; lane < numColors; ++lane)
		if (m_colors[lane])
		{
			m_colors[lane].save_bch(lane + 1, dataPtr);
			++numActive;
		}
	return numActive;
}
