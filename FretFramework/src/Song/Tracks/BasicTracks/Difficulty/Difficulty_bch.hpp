#pragma once
#include <iostream>
#include "Difficulty.h"
#include "NoteExceptions.h"
#include "Note_bch.hpp"
#include "Chords\Chord_bch.hpp"

template<typename T>
inline void Difficulty<T>::load_bch(const unsigned char* current, const unsigned char* const end)
{
	clear();
	{
		uint32_t headerSize = *(uint32_t*)current;
		current += 4;
		const unsigned char* const start = current + headerSize;

		if (current == start)
			goto StartNoteRead;
		current += 4;

		if (current == start)
			goto StartNoteRead;

		uint32_t size = *(uint32_t*)current;
		current += 4;
		m_notes.reserve(size);

		if (current == start)
			goto StartNoteRead;

		size = *(uint32_t*)current;
		current += 4;
		m_effects.reserve(size);

		if (current == start)
			goto StartNoteRead;

		size = *(uint32_t*)current;
		current += 4;
		m_events.reserve(size);
	}

StartNoteRead:
	uint32_t position = 0;
	int eventCount = 0;
	while (current < end)
	{
		position += WebType(current);
		unsigned char type = *current++;
		WebType length(current);

		const unsigned char* const next = current + length;
		if (next > end)
			break;

		switch (type)
		{
		case 3:
			if (m_events.empty() || m_events.back().first < position)
			{
				static std::pair<uint32_t, std::vector<std::string>> pairNode;
				pairNode.first = position;
				m_events.push_back(pairNode);
			}

			m_events.back().second.push_back(std::string((const char*)current, length));
			break;
		case 6:
		case 7:
			if (m_notes.empty() || m_notes.back().first != position)
			{
				static std::pair<uint32_t, T> pairNode;
				pairNode.first = position;
				m_notes.push_back(std::move(pairNode));
			}

			try
			{
				if (type == 6)
					m_notes.back().second.init_bch_single(current, next);
				else
					m_notes.back().second.init_bch_chord(current, next);
			}
			catch (std::runtime_error err)
			{
				std::cout << "Event #" << eventCount << " - Position " << position << ": " << err.what() << std::endl;
				if (type == 7 || m_notes.back().second.getNumActive() == 0)
					m_notes.pop_back();
			}
			break;
		case 8:
			if (!m_notes.empty() && m_notes.back().first == position)
				m_notes.back().second.modify_bch(current, next);
			break;
		case 5:
		{
			unsigned char phrase = *current++;
			uint32_t duration = 0;
			auto check = [&]()
			{
				duration = WebType(current);
				if (m_effects.empty() || m_effects.back().first < position)
				{
					static std::pair<uint32_t, std::vector<SustainablePhrase*>> pairNode;
					pairNode.first = position;
					m_effects.push_back(pairNode);
				}
			};

			switch (phrase)
			{
			case 2:
				check();
				m_effects.back().second.push_back(new StarPowerPhrase(duration));
				break;
			case 3:
				check();
				m_effects.back().second.push_back(new Solo(duration));
				break;
			case 4:
			case 5:
			case 6:
				break;
			case 64:
				check();
				m_effects.back().second.push_back(new StarPowerActivation(duration));
				break;
			case 65:
				check();
				m_effects.back().second.push_back(new Tremolo(duration));
				break;
			case 66:
				check();
				m_effects.back().second.push_back(new Trill(duration));
				break;
			case 67:
				break;
			default:
				std::cout << "Event #" << eventCount << " - Position " << position << ": unrecognized special phrase type (" << phrase << ')' << std::endl;
			}
			break;
		}
		default:
			std::cout << "Event #" << eventCount << " - Position " << position << ": unrecognized node type(" << type << ')' << std::endl;
		}
		++eventCount;
		current = next;
	}

	

	if (m_notes.size() < m_notes.capacity())
		m_notes.shrink_to_fit();
}

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
			WebType position(effectIter->first - prevPosition);
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
			WebType position(eventIter->first - prevPosition);
			for (const auto& str : eventIter->second)
			{
				position.writeToFile(outFile);
				outFile.put(3);
				WebType length((uint32_t)str.length());
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
