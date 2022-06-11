#pragma once
#include <iostream>
#include "Difficulty.h"
#include "NoteExceptions.h"
#include "InstrumentalNote_bch.hpp"
#include "Drums\DrumNote_bch.hpp"
#include "Chords\GuitarNote\GuitarNote_bch.hpp"

template<typename T>
inline bool Difficulty<T>::scan_bch(BCHTraversal& traversal)
{
	static T obj;
	traversal.move(4);
	while (traversal.next())
	{
		switch (traversal.getEventType())
		{
		case 6:
		case 7:
			try
			{
				if (traversal.getEventType() == 6)
					obj.init_single(traversal);
				else
					obj.init_chord(traversal);
				

				// So long as the init does not throw an exception, it can be concluded that this difficulty does contain notes
				// No need to check the rest of the difficulty's data

				traversal.skipTrack();
				return true;
			}
			catch (std::runtime_error err)
			{
			}
			break;
		}
	}
	return false;
}

template<typename T>
inline void Difficulty<T>::load_bch(BCHTraversal& traversal)
{
	clear();
	traversal.move(4);
	m_notes.reserve(5000);

	const static std::vector<std::string> eventNode;
	const static T noteNode;
	const static std::vector<SustainablePhrase*> phraseNode;

	// End positions to protect from conflicting special phrases
	uint32_t starPowerEnd = 0;
	uint32_t soloEnd = 0;
	uint32_t starActivationEnd = 0;
	uint32_t tremoloEnd = 0;
	uint32_t trillEnd = 0;
	while (traversal.next())
	{
		try
		{
			switch (traversal.getEventType())
			{
			case 6:
				if (m_notes.empty() || m_notes.back().first != traversal.getPosition())
					m_notes.emplace_back(traversal.getPosition(), noteNode);

				try
				{
					m_notes.back().second.init_single(traversal);
				}
				catch (std::runtime_error err)
				{
					if (m_notes.back().second.getNumActive() == 0)
						m_notes.pop_back();
					throw err;
				}
				break;
			case 7:
				if (m_notes.empty() || m_notes.back().first != traversal.getPosition())
					m_notes.emplace_back(traversal.getPosition(), noteNode);

				try
				{
					m_notes.back().second.init_chord(traversal);
				}
				catch (std::runtime_error err)
				{
					m_notes.pop_back();
					throw err;
				}
				break;
			case 3:
				if (m_events.empty() || m_events.back().first < traversal.getPosition())
					m_events.emplace_back(traversal.getPosition(), eventNode);

				m_events.back().second.push_back(traversal.extractText());
				break;
			case 8:
				if (!m_notes.empty() && m_notes.back().first == traversal.getPosition())
					m_notes.back().second.modify(traversal);
				break;
			case 5:
			{
				unsigned char phrase = traversal.extractChar();
				uint32_t duration = traversal.extractVarType();
				auto check = [&](uint32_t& end, const char* noteType)
				{
					// Handles phrase conflicts
					if (traversal.getPosition() < end)
						throw std::string(noteType) + " note conflicts with current active " + noteType + " phrase (ending at tick " + std::to_string(end) + ')';

					if (m_effects.empty() || m_effects.back().first < traversal.getPosition())
						m_effects.emplace_back(traversal.getPosition(), phraseNode);

					end = traversal.getPosition() + duration;
				};

				switch (phrase)
				{
				case 2:
					check(starPowerEnd, "star power");
					m_effects.back().second.push_back(new StarPowerPhrase(duration));
					break;
				case 3:
					check(soloEnd, "solo");
					m_effects.back().second.push_back(new Solo(duration));
					break;
				case 4:
				case 5:
				case 6:
					break;
				case 64:
					check(starActivationEnd, "star power activation");
					m_effects.back().second.push_back(new StarPowerActivation(duration));
					break;
				case 65:
					check(tremoloEnd, "tremolo");
					m_effects.back().second.push_back(new Tremolo(duration));
					break;
				case 66:
					check(trillEnd, "trill");
					m_effects.back().second.push_back(new Trill(duration));
					break;
				case 67:
					break;
				default:
					throw "unrecognized special phrase type (" + std::to_string((int)phrase) + ')';
				}
				break;
			}
			default:
				throw "unrecognized node type(" + std::to_string((int)traversal.getEventType()) + ')';
			}
		}
		catch (std::runtime_error err)
		{
			std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": " << err.what() << std::endl;
		}
		catch (const std::string& str)
		{
			std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": " << str << std::endl;
		}
	}

	if ((m_notes.size() < 500 || 10000 <= m_notes.size()) && m_notes.size() < m_notes.capacity())
		m_notes.shrink_to_fit();
}

template<typename T>
inline void Difficulty<T>::save_bch(std::fstream& outFile) const
{
	outFile.write("DIFF", 4);

	auto start = outFile.tellp();
	uint32_t length = 0;
	outFile.write((char*)&length, 4);

	// Header Data
	uint32_t numEvents = 0;
	
	outFile.put(m_difficultyID);
	outFile.write((char*)&numEvents, 4);

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
			WebType::writeToFile(noteIter->first - prevPosition, outFile);
			numEvents += noteIter->second.save_bch(outFile);
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
	length = uint32_t(end - start) - 4;
	outFile.seekp(start);
	outFile.write((char*)&length, 4);
	outFile.put(m_difficultyID);
	outFile.write((char*)&numEvents, 4);
	outFile.seekp(end);
}
