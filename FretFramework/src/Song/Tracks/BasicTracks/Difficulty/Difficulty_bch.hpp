#pragma once
#include <iostream>
#include "Difficulty.h"
#include "NoteExceptions.h"
#include "Note_bch.hpp"
#include "Chords\Chord_bch.hpp"

template<typename T>
inline void Difficulty<T>::load_bch(BinaryTraversal& traversal)
{
	clear();
	try
	{
		traversal.move(4);
		uint32_t size = traversal;
		m_notes.reserve(size);
		size = traversal;
		m_effects.reserve(size);
		size = traversal;
		m_events.reserve(size);
	}
	// The only error that should be caught signals to start parsing events
	catch (...) {}

	while (traversal.next())
	{
		switch (traversal.getEventType())
		{
		case 3:
			if (m_events.empty() || m_events.back().first < traversal.getPosition())
			{
				static std::pair<uint32_t, std::vector<std::string>> pairNode;
				pairNode.first = traversal.getPosition();
				m_events.push_back(pairNode);
			}

			m_events.back().second.push_back(traversal.extractText());
			break;
		case 6:
		case 7:
			if (m_notes.empty() || m_notes.back().first != traversal.getPosition())
			{
				static std::pair<uint32_t, T> pairNode;
				pairNode.first = traversal.getPosition();
				m_notes.push_back(std::move(pairNode));
			}

			try
			{
				if (traversal.getEventType() == 6)
					m_notes.back().second.init_bch_single(traversal);
				else
					m_notes.back().second.init_bch_chord(traversal);
			}
			catch (std::runtime_error err)
			{
				std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": " << err.what() << std::endl;
				if (traversal.getEventType() == 7 || m_notes.back().second.getNumActive() == 0)
					m_notes.pop_back();
			}
			break;
		case 8:
			if (!m_notes.empty() && m_notes.back().first == traversal.getPosition())
				m_notes.back().second.modify_bch(traversal);
			break;
		case 5:
		{
			unsigned char phrase = traversal.extract();
			uint32_t duration = 0;
			auto check = [&]()
			{
				traversal.extractVarType(duration);
				if (m_effects.empty() || m_effects.back().first < traversal.getPosition())
				{
					static std::pair<uint32_t, std::vector<SustainablePhrase*>> pairNode;
					pairNode.first = traversal.getPosition();
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
				std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": unrecognized special phrase type (" << phrase << ')' << std::endl;
			}
			break;
		}
		default:
			std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": unrecognized node type(" << traversal.getEventType() << ')' << std::endl;
		}
	}

	if (m_notes.size() < m_notes.capacity())
		m_notes.shrink_to_fit();
}

template<typename T>
inline void Difficulty<T>::save_bch(std::fstream& outFile) const
{
	outFile.write("DIFF", 4);
	const uint32_t headerLength = 17;
	outFile.write((char*)&headerLength, 4);

	auto start = outFile.tellp();
	uint32_t length = 0;
	outFile.write((char*)&length, 4);

	// Header Data
	outFile.put(m_difficultyID);
	uint32_t numEvents = 0;
	const uint32_t numNotes = (uint32_t)m_notes.size();
	const uint32_t numPhrases = (uint32_t)m_effects.size();
	const uint32_t numTextEvents = (uint32_t)m_events.size();
	
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
	length = uint32_t(end - start) - (headerLength + 4);
	outFile.seekp(start);
	outFile.write((char*)&length, 4);
	outFile.put(m_difficultyID);
	outFile.write((char*)&numEvents, 4);
	outFile.seekp(end);
}
