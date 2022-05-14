#pragma once
#include "Note.h"

template <int numColors, class NoteType, class SpecialType>
void Note<numColors, NoteType, SpecialType>::init_cht_chord(TextTraversal& traversal)
{
	uint32_t colors;
	if (size_t count = traversal.extractUInt(colors))
	{
		int numAdded = 0;
		traversal.move(count);
		uint32_t lane;
		for (int i = 0; i < colors; ++i)
		{
			if (!(count = traversal.extractUInt(lane)))
				throw EndofLineException();

			traversal.move(count);
			unsigned char color = lane & 127;
			uint32_t sustain = 0;
			if (lane & 128)
			{
				if (!(count = traversal.extractUInt(sustain)))
					throw EndofLineException();
				traversal.move(count);
			}

			if (color == 0)
			{
				m_special.init(sustain);
				++numAdded;
			}
			else if (color <= numColors)
			{
				m_colors[color - 1].init(sustain);
				++numAdded;
			}
		}

		if (numAdded == 0)
			throw InvalidNoteException();
	}
	else
		throw EndofLineException();
}

template <int numColors, class NoteType, class SpecialType>
uint32_t Note<numColors, NoteType, SpecialType>::write_notes_cht(uint32_t position, std::fstream& outFile, const char* const tabs) const
{
	uint32_t numActive = getNumActiveColors();
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
