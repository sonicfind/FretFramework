#pragma once
#include "InstrumentalNote.h"
#include "Variable Types/VariableLengthQuantity.h"
#include "Variable Types/WebType.h"

template<int numColors, class NoteType>
inline void InstrumentalNote_NoSpec<numColors, NoteType>::init_single(BinaryTraversal& traversal)
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
	// If the end of the event is already reached, then no value is extracted
	if (traversal.extract(val))
		modify_binary(val, color);
}

template<int numColors, class NoteType>
inline void InstrumentalNote_NoSpec<numColors, NoteType>::init_chord(BinaryTraversal& traversal)
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

template<int numColors, class NoteType>
inline void InstrumentalNote_NoSpec<numColors, NoteType>::modify(BinaryTraversal& traversal)
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
inline uint32_t InstrumentalNote_NoSpec<numColors, NoteType>::save_bch(uint32_t position, std::fstream& outFile) const
{
	static char buffer[6 * (numColors + 1) + 1];
	static char* const start = buffer + 1;

	char* current = start;
	buffer[0] = write_notes(current);
	if (buffer[0] == 1)
	{
		write_modifiers_single(current);
		WebType(position).writeToFile(outFile);
		outFile.put(6);
		WebType length(uint32_t(current - start));
		length.writeToFile(outFile);
		outFile.write(start, length);
		return 1;
	}
	else
	{
		WebType(position).writeToFile(outFile);
		outFile.put(7);
		WebType length(uint32_t(current - buffer));
		length.writeToFile(outFile);
		outFile.write(buffer, length);

		current = start;
		buffer[0] = write_modifiers_chord(current);
		if (buffer[0] > 0)
		{
			outFile.put(0);
			outFile.put(8);
			length = uint32_t(current - buffer);
			length.writeToFile(outFile);
			outFile.write(buffer, length);
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
