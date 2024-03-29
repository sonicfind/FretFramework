#pragma once
#include <iostream>
#include "Difficulty.h"
#include "NoteExceptions.h"
#include "InstrumentalNote_bch.hpp"
#include "Drums\DrumNote_bch.hpp"
#include "Chords\GuitarNote\GuitarNote_bch.hpp"

template<typename T>
inline void Difficulty<T>::load_bch(BCHTraversal& traversal)
{
	clear();
	traversal.move(4);
	m_notes.reserve(5000);

#ifndef _DEBUG
	static constexpr std::vector<std::u32string> eventNode;
	static constexpr T noteNode;
	static constexpr std::vector<SustainablePhrase*> phraseNode;
#else
	static const std::vector<std::u32string> eventNode;
	static constexpr T noteNode;
	static const std::vector<SustainablePhrase*> phraseNode;
#endif // !_DEBUG

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
			const uint32_t position = traversal.getPosition();
			switch (traversal.getEventType())
			{
			case 6:
				if (m_notes.empty() || m_notes.back().first != position)
					m_notes.emplace_back(position, noteNode);

				try
				{
					init_single(traversal);
				}
				catch (std::runtime_error err)
				{
					if (m_notes.back().second.getNumActive() == 0)
						m_notes.pop_back();
					throw err;
				}
				break;
			case 7:
				if (m_notes.empty() || m_notes.back().first != position)
					m_notes.emplace_back(position, noteNode);

				try
				{
					init_chord(traversal);
				}
				catch (std::runtime_error err)
				{
					m_notes.pop_back();
					throw err;
				}
				break;
			case 3:
				if (m_events.empty() || m_events.back().first < position)
					m_events.emplace_back(position, eventNode);

				m_events.back().second.push_back(traversal.extractText());
				break;
			case 8:
				if (!m_notes.empty() && m_notes.back().first == position)
					m_notes.back().second.modify(traversal);
				break;
			case 5:
			{
				unsigned char phrase = traversal.extract<unsigned char>();
				uint32_t duration = traversal.extract<WebType::WebType_t>();
				auto check = [&](uint32_t& end, const char* noteType)
				{
					// Handles phrase conflicts
					if (position < end)
						throw std::string(noteType) + " note conflicts with current active " + noteType + " phrase (ending at tick " + std::to_string(end) + ')';

					if (m_effects.empty() || m_effects.back().first < position)
						m_effects.emplace_back(position, phraseNode);

					end = position + duration;
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
			uint32_t position = effectIter->first - prevPosition;
			for (const auto& eff : effectIter->second)
			{
				WebType::writeToFile(position, outFile);
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
			uint32_t position = eventIter->first - prevPosition;
			for (const auto& str : eventIter->second)
			{
				WebType::writeToFile(position, outFile);
				outFile.put(3);
				UnicodeString::U32ToBCH(str, outFile);
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
