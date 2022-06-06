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
	try
	{
		// Read note
		uint32_t val = traversal.extractChar();
		unsigned char color = val & 127;

		uint32_t sustain = val >= 128 ? traversal.extractU32() : 0;
		init(color, sustain);

		// Read modifiers
		if (traversal.extract(val))
		{
			unsigned char modifier;
			for (uint32_t i = 0; i < val && traversal.extract(modifier); ++i)
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
		uint32_t colors = traversal.extractU32();
		for (unsigned char i = 0; i < colors; ++i)
		{
			uint32_t lane = traversal.extractU32();
			uint32_t sustain = lane >= 128 ? traversal.extractU32() : 0;
			init(lane & 127, sustain);
		}
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
			uint32_t lane = 0;
			// Checks for a possible lane value after the modifier
			traversal.extract(lane);
			modify_binary(modifier, lane);
		}
	}
	catch (Traversal::NoParseException)
	{
		throw EndofEventException();
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
