#pragma once
#include "Note.h"
#include "../VariableLengthQuantity.h"

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
