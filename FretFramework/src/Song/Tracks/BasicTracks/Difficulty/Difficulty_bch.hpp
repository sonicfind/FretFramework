#pragma once
#include <iostream>
#include "Difficulty.h"
#include "NoteExceptions.h"
#include "Note_bch.hpp"
#include "Chords\Chord_bch.hpp"

template<typename T>
inline void Difficulty<T>::save_bch(std::fstream& outFile) const
{
	outFile.write("DIFF", 4);
	uint32_t length = 0;
	auto start = outFile.tellp();
	outFile.write((char*)&length, 4);
	outFile.put(m_difficultyID);

	const uint32_t headerLength = 16;
	uint32_t numEvents = 0;
	const uint32_t numNotes = (uint32_t)m_notes.size();
	const uint32_t numPhrases = (uint32_t)m_effects.size();
	const uint32_t numTextEvents = (uint32_t)m_events.size();
	
	outFile.write((char*)&headerLength, 4);
	outFile.write((char*)&numEvents, 4);
	outFile.write((char*)&numNotes, 4);
	outFile.write((char*)&numPhrases, 4);
	outFile.write((char*)&numTextEvents, 4);

	auto noteIter = m_notes.begin();
	auto effectIter = m_effects.begin();
	auto eventIter = m_events.begin();
	bool notesValid = noteIter != m_notes.end();
	bool effectValid = effectIter != m_effects.end();
	bool eventValid = eventIter != m_events.end();

	uint32_t prevPosition = 0;
	while (notesValid || effectValid || eventValid)
	{
		while (effectValid &&
			(!notesValid || effectIter->first <= noteIter->first) &&
			(!eventValid || effectIter->first <= eventIter->first))
		{
			VariableLengthQuantity position(effectIter->first - prevPosition);
			for (const auto& eff : effectIter->second)
			{
				position.writeToFile(outFile);
				eff->save_bch(outFile);
				position = 0;
			}
			numEvents += (uint32_t)effectIter->second.size();
			prevPosition = effectIter->first;
			effectValid = ++effectIter != m_effects.end();
		}

		while (notesValid &&
			(!effectValid || noteIter->first < effectIter->first) &&
			(!eventValid || noteIter->first <= eventIter->first))
		{
			numEvents += noteIter->second.save_bch(noteIter->first - prevPosition, outFile);
			prevPosition = noteIter->first;
			notesValid = ++noteIter != m_notes.end();
		}

		while (eventValid &&
			(!effectValid || eventIter->first < effectIter->first) &&
			(!notesValid || eventIter->first < noteIter->first))
		{
			VariableLengthQuantity position(eventIter->first - prevPosition);
			for (const auto& str : eventIter->second)
			{
				position.writeToFile(outFile);
				outFile.put(3);
				VariableLengthQuantity length((uint32_t)str.length());
				length.writeToFile(outFile);
				outFile.write(str.data(), length);
				position = 0;
			}
			numEvents += (uint32_t)eventIter->second.size();
			prevPosition = eventIter->first;
			eventValid = ++eventIter != m_events.end();
		}
	}

	const auto end = outFile.tellp();
	length = uint32_t(end - start - 4);
	outFile.seekp(start);
	outFile.write((char*)&length, 4);
	outFile.put(m_difficultyID);
	outFile.write((char*)&headerLength, 4);
	outFile.write((char*)&numEvents, 4);
	outFile.seekp(end);
}
