#pragma once
#include "InstrumentalNote.h"
#include "Variable Types/VariableLengthQuantity.h"
#include "Variable Types/WebType.h"

template<int numColors, class NoteType>
inline unsigned char InstrumentalNote_NoSpec<numColors, NoteType>::read_note(BCHTraversal& traversal)
{
	unsigned char color = traversal.extract<unsigned char>();
	uint32_t sustain = 0;
	if (color >= 128)
	{
		color &= 127;
		sustain = traversal.extract<WebType::WebType_t>();
	}
	init(color, sustain);
	return color;
}

template<int numColors, class NoteType>
inline void InstrumentalNote_NoSpec<numColors, NoteType>::init_single(BCHTraversal& traversal)
{
	try
	{
		const unsigned char color = read_note(traversal);
		unsigned char modifiers;
		if (traversal.extract(modifiers))
			modify_binary(modifiers, color);
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
		const unsigned char numColorsToParse = traversal.extract<unsigned char>();
		for (unsigned char i = 0; i < numColorsToParse; ++i)
			read_note(traversal);
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
		const unsigned char numMods = traversal.extract<unsigned char>();
		unsigned char modifier;
		for (char i = 0; i < numMods && traversal.extract(modifier); ++i)
		{
			unsigned char lane = 0;
			if (modifier >= 128)
				lane = traversal.extract<unsigned char>();
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

template <class NoteType>
bool scan_note(BCHTraversal& traversal)
{
	unsigned char color = traversal.extract<unsigned char>();
	if (color >= 128)
	{
		color &= 127;
		if (!traversal.testExtract<WebType::WebType_t>())
			return false;
	}

	return NoteType::testIndex(color);
}

template <class NoteType>
bool validate_single(BCHTraversal& traversal)
{
	return scan_note<NoteType>(traversal);
}

template <class NoteType>
bool validate_chord(BCHTraversal& traversal)
{
	const unsigned char numColorsToParse = traversal.extract<unsigned char>();
	for (unsigned char i = 0; i < numColorsToParse; ++i)
		if (!scan_note<NoteType>(traversal))
			return false;
	return true;
}
