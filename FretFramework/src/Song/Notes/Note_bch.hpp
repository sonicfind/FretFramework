#pragma once
#include "Note.h"
#include "../VariableLengthQuantity.h"
#include "../WebType.h"

template<int numColors, class NoteType, class SpecialType>
inline void Note<numColors, NoteType, SpecialType>::init_bch_single(const unsigned char* current, const unsigned char* const end)
{
	int lane = *current++;
	int color = lane & 127;
	if (lane >= 128)
		init(color, WebType(current));
	else
		init(color);

	if (current < end)
		modify_binary(*current++, color);
}

template<int numColors, class NoteType, class SpecialType>
inline void Note<numColors, NoteType, SpecialType>::init_bch_chord(const unsigned char* current, const unsigned char* const end)
{
	int colors = *current++;
	for (int i = 0; i < colors; ++i)
	{
		int lane = *current++;
		if (lane >= 128)
			init(lane & 127, WebType(current));
		else
			init(lane & 127);
	}
}

template<int numColors, class NoteType, class SpecialType>
inline void Note<numColors, NoteType, SpecialType>::modify_bch(const unsigned char* current, const unsigned char* const end)
{
	char numMods = *current++;
	for (char i = 0; current < end && i < numMods; ++i)
	{
		char modifier = *current++;
		int lane = modifier >= 128 ? *current++ : 0;
		modify_binary(modifier, lane);
	}
}

template<int numColors, class NoteType, class SpecialType>
inline char Note<numColors, NoteType, SpecialType>::write_notes_bch(char*& dataPtr) const
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
