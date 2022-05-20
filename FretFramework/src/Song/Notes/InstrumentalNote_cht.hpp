#pragma once
#include "InstrumentalNote.h"

template<int numColors, class NoteType>
inline void InstrumentalNote_NoSpec<numColors, NoteType>::init_chartV1(unsigned char lane, uint32_t sustain)
{
	if (lane < numColors)
		m_colors[lane].init(sustain);
	else
		throw InvalidNoteException(lane);
}

template<int numColors, class NoteType>
inline void InstrumentalNote_NoSpec<numColors, NoteType>::init_single(TextTraversal& traversal)
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

template <int numColors, class NoteType>
inline void InstrumentalNote_NoSpec<numColors, NoteType>::init_chord(TextTraversal& traversal)
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

template<int numColors, class NoteType>
inline void InstrumentalNote_NoSpec<numColors, NoteType>::modify(TextTraversal& traversal)
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

template <int numColors, class NoteType>
inline int InstrumentalNote_NoSpec<numColors, NoteType>::write_notes(std::stringstream& buffer) const
{
	int numActive = 0;
	for (int lane = 0; lane < numColors; ++lane)
		if (m_colors[lane])
		{
			m_colors[lane].save_cht(lane + 1, buffer);
			++numActive;
		}
	return numActive;
}

template <int numColors, class NoteType>
inline int InstrumentalNote_NoSpec<numColors, NoteType>::write_modifiers_single(std::stringstream& buffer) const
{
	return 0;
}

template <int numColors, class NoteType>
inline int InstrumentalNote_NoSpec<numColors, NoteType>::write_modifiers_chord(std::stringstream& buffer) const
{
	return 0;
}
template<int numColors, class NoteType>
inline void InstrumentalNote_NoSpec<numColors, NoteType>::save_cht(uint32_t position, std::fstream& outFile) const
{
	std::stringstream buffer;
	int count = write_notes(buffer);
	if (count == 1)
	{
		outFile << "\t\t" << position << " = N" << buffer.rdbuf();
		buffer.clear();
		if (count = write_modifiers_single(buffer))
			outFile << count << buffer.rdbuf();
	}
	else
	{
		outFile << "\t\t" << position << " = C " << count << buffer.rdbuf();
		buffer.clear();
		if (count = write_modifiers_chord(buffer))
			outFile << "\n\t\t" << position << " = M " << count << buffer.rdbuf();
	}
	outFile << '\n';
}

template <int numColors, class NoteType, class SpecialType>
inline int InstrumentalNote<numColors, NoteType, SpecialType>::write_notes(std::stringstream& buffer) const
{
	int numActive = 0;
	if (m_special)
	{
		m_special.save_cht(0, buffer);
		++numActive;
	}

	numActive += InstrumentalNote_NoSpec<numColors, NoteType>::write_notes(buffer);
	return numActive;
}
