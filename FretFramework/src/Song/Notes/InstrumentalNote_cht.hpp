#pragma once
#include "InstrumentalNote.h"

template<int numColors, class NoteType>
inline unsigned char InstrumentalNote_NoSpec<numColors, NoteType>::read_note(TextTraversal& traversal)
{
	unsigned char color = (unsigned char)traversal.extractU32();
	uint32_t sustain = 0;
	if (color >= 128)
	{
		color &= 127;
		sustain = traversal.extractU32();
	}
	init(color, sustain);
	return color;
}

template<int numColors, class NoteType>
inline void InstrumentalNote_NoSpec<numColors, NoteType>::init_single(TextTraversal& traversal)
{
	try
	{
		unsigned char color = read_note(traversal);

		uint32_t numMods;
		if (traversal.extract(numMods))
		{
			unsigned char modifier;
			for (uint32_t i = 0; i < numMods && traversal.extract(modifier); ++i)
				modify(modifier, color);
		}
	}
	catch (Traversal::NoParseException)
	{
		throw EndofLineException();
	}
}

template <int numColors, class NoteType>
inline void InstrumentalNote_NoSpec<numColors, NoteType>::init_chord(TextTraversal& traversal)
{
	try
	{
		uint32_t numColorsToParse = traversal.extractU32();
		for (unsigned char i = 0; i < numColorsToParse; ++i)
			read_note(traversal);
	}
	catch (Traversal::NoParseException)
	{
		throw EndofLineException();
	}
}

template<int numColors, class NoteType>
inline void InstrumentalNote_NoSpec<numColors, NoteType>::modify(TextTraversal& traversal)
{
	try
	{
		uint32_t numMods = traversal.extractU32();
		unsigned char modifier;
		for (uint32_t i = 0; i < numMods && traversal.extract(modifier); ++i)
		{
			uint32_t color = 0;
			// Checks for a possible lane value after the modifier
			traversal.extract(color);
			modify(modifier, color);
		}
	}
	catch (Traversal::NoParseException)
	{
		throw EndofLineException();
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
