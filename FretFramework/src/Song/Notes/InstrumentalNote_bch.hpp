#pragma once
#include "InstrumentalNote.h"
#include "Variable Types/VariableLengthQuantity.h"
#include "Variable Types/WebType.h"

template<int numColors, class NoteType>
inline void InstrumentalNote_NoSpec<numColors, NoteType>::init_single(BCHTraversal& traversal)
{
	try
	{
		// Read note
		unsigned char val = traversal.extractChar();
		unsigned char color = val & 127;

		uint32_t sustain = val >= 128 ? traversal.extractVarType() : 0;
		init(color, sustain);

		// Read modifiers
		// If the end of the event is already reached, then no value is extracted
		if (traversal.extract(val))
			modify_binary(val, color);
	}
	catch (Traversal::NoParseException)
	{
		throw EndofEventException();
	}
}

template<int numColors, class NoteType>
inline void InstrumentalNote_NoSpec<numColors, NoteType>::init_chord(BCHTraversal& traversal)
{
	try
	{
		unsigned char colors = traversal.extractChar();
		for (unsigned char i = 0; i < colors; ++i)
		{
			unsigned char lane = traversal.extractChar();
			uint32_t sustain = lane >= 128 ? traversal.extractVarType() : 0;
			init(lane & 127, sustain);
		}
	}
	catch (Traversal::NoParseException)
	{
		throw EndofEventException();
	}
	
}

template<int numColors, class NoteType>
inline void InstrumentalNote_NoSpec<numColors, NoteType>::modify(BCHTraversal& traversal)
{
	try
	{
		unsigned char numMods = traversal.extractChar();
		unsigned char modifier;
		for (char i = 0; i < numMods && traversal.extract(modifier); ++i)
		{
			unsigned char lane = modifier >= 128 ? traversal.extractChar() : 0;
			modify_binary(modifier, lane);
		}
	}
	catch (Traversal::NoParseException)
	{
		throw EndofEventException();
	}
}

template<int numColors, class NoteType>
inline char InstrumentalNote_NoSpec<numColors, NoteType>::write_notes(char*& dataPtr) const
{
	char numActive = 0;
	for (int lane = 0; lane < numColors; ++lane)
		if (m_colors[lane])
		{
			m_colors[lane].save_bch(lane + 1, dataPtr);
			++numActive;
		}
	return numActive;
}

template <int numColors, class NoteType>
inline void InstrumentalNote_NoSpec<numColors, NoteType>::write_modifiers_single(char*& buffer) const {}

template <int numColors, class NoteType>
inline char InstrumentalNote_NoSpec<numColors, NoteType>::write_modifiers_chord(char*& buffer) const
{
	return 0;
}

template<int numColors, class NoteType>
inline uint32_t InstrumentalNote_NoSpec<numColors, NoteType>::save_bch(std::fstream& outFile) const
{
	static char buffer[6 + 6 * (numColors + 1) + 1];
	static char* const start = buffer + 1;
	char* current = start;
	
	buffer[0] = write_notes(current);
	if (buffer[0] == 1)
	{
		write_modifiers_single(current);

		const uint32_t length(uint32_t(current - start));
		// Event type - Single
		outFile.put(6);
		WebType::writeToFile(length, outFile);
		outFile.write(start, length);
		return 1;
	}
	else
	{
		const uint32_t length(uint32_t(current - buffer));
		// Event type - Chord
		outFile.put(7);
		WebType::writeToFile(length, outFile);
		outFile.write(buffer, length);

		current = buffer + 4;
		buffer[3] = write_modifiers_chord(current);
		if (buffer[3] > 0)
		{
			buffer[0] = 0;
			// Event type - Modifiers
			buffer[1] = 8;
			buffer[2] = char(current - buffer - 3);
			outFile.write(buffer, buffer[2] + 3ULL);
			return 2;
		}
		return 1;
	}
}

template <int numColors, class NoteType, class SpecialType>
inline char InstrumentalNote<numColors, NoteType, SpecialType>::write_notes(char*& buffer) const
{
	int numActive = 0;
	if (m_special)
	{
		m_special.save_bch(0, buffer);
		++numActive;
	}

	numActive += InstrumentalNote_NoSpec<numColors, NoteType>::write_notes(buffer);
	return numActive;
}
