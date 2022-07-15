#include <iostream>
#include "Difficulty_DrumLegacy.h"
#include "NoteExceptions.h"
#include "InstrumentalNote_cht.hpp"
#include "InstrumentalNote_bch.hpp"


template <>
template <>
Difficulty<DrumNote<4, DrumPad_Pro>>& Difficulty<DrumNote<4, DrumPad_Pro>>::operator=(const Difficulty<DrumNote_Legacy>& diff)
{
	m_effects = std::move(diff.m_effects);
	m_events = std::move(diff.m_events);
	m_notes.reserve(diff.m_notes.size());
	m_notes.clear();
	for (const auto& pair : diff.m_notes)
		m_notes.emplace_back(pair.first, pair.second);
	return *this;
}

template <>
template <>
Difficulty<DrumNote<5, DrumPad>>& Difficulty<DrumNote<5, DrumPad>>::operator=(const Difficulty<DrumNote_Legacy>& diff)
{
	m_effects = std::move(diff.m_effects);
	m_events = std::move(diff.m_events);
	m_notes.reserve(diff.m_notes.size());
	m_notes.clear();
	for (const auto& pair : diff.m_notes)
		m_notes.emplace_back(pair.first, pair.second);
	return *this;
}

#define PHRASESCAN(end) if (position >= end) { end = position + traversal.extractU32(); prevPosition = position; }

bool Difficulty_Scan<DrumNote_Legacy>::scan_chart_V1(TextTraversal& traversal)
{
	// End positions to protect from conflicting special phrases
	uint32_t starPowerEnd = 0;
	uint32_t starActivationEnd = 0;

	uint32_t prevPosition = 0;
	while (traversal && traversal != '}' && traversal != '[')
	{
		try
		{
			uint32_t position = traversal.extractU32();
			if (prevPosition <= position)
			{
				traversal.skipEqualsSign();
				char type = traversal.extractChar();

				// Special Phrases & Text Events are only important for validating proper event order in regards to tick position
				switch (type)
				{
				case 'n':
				case 'N':
				{
					const int lane = traversal.extractU32();
					const uint32_t sustain = traversal.extractU32();
					init_chart_V1(lane, sustain);

					// So long as the init does not throw an exception, it can be concluded that this difficulty does contain notes
					// No need to check the rest of the difficulty's data
					traversal.skipTrack();
					return true;
				}
				case 's':
				case 'S':
				{
					uint32_t phrase = traversal.extractU32();
					if (phrase == 2)
					{
						PHRASESCAN(starPowerEnd)
					}
					else if (phrase == 64)
					{
						PHRASESCAN(starActivationEnd)
					}
					break;
				}
				case 'e':
				case 'E':
					prevPosition = position;
					break;
				}
			}
		}
		catch (std::runtime_error err)
		{

		}

		traversal.next();
	}

	// If the execution of the function reaches here, it can be concluded that the difficulty does not contain any notes
	return false;
}

void Difficulty<DrumNote_Legacy>::load_chart_V1(TextTraversal& traversal)
{
	clear();
	m_notes.reserve(5000);

#ifndef _DEBUG
	static constexpr std::vector<UnicodeString> eventNode;
	static constexpr DrumNote_Legacy noteNode;
	static constexpr std::vector<SustainablePhrase*> phraseNode;
#else
	static const std::vector<UnicodeString> eventNode;
	static constexpr DrumNote_Legacy noteNode;
	static const std::vector<SustainablePhrase*> phraseNode;
#endif // !_DEBUG

	// End positions to protect from conflicting special phrases
	uint32_t starPowerEnd = 0;
	uint32_t starActivationEnd = 0;

	uint32_t solo = 0;
	uint32_t prevPosition = 0;
	while (traversal && traversal != '}' && traversal != '[')
	{
		uint32_t position = UINT32_MAX;
		try
		{
			position = traversal.extractU32();
			if (prevPosition > position)
				throw "position out of order (previous:  " + std::to_string(prevPosition) + ')';

			traversal.skipEqualsSign();
			char type = traversal.extractChar();
			switch (type)
			{
			case 'n':
			case 'N':
			{
				uint32_t lane = traversal.extractU32();
				uint32_t sustain = traversal.extractU32();

				if (m_notes.empty() || m_notes.back().first != position)
					m_notes.emplace_back(position, noteNode);

				try
				{
					init_chart_V1(lane, sustain);
					prevPosition = position;
				}
				catch (std::runtime_error err)
				{
					if (m_notes.back().second.getNumActive() == 0)
						m_notes.pop_back();
					throw err;
				}
				break;
			}
			case 's':
			case 'S':
			{
				uint32_t phrase = traversal.extractU32();
				uint32_t duration = traversal.extractU32();
				auto check = [&](uint32_t& end, const char* noteType)
				{
					// Handles phrase conflicts
					if (position < end)
						throw std::string(noteType) + " note conflicts with current active " + noteType + " phrase (ending at tick " + std::to_string(end) + ')';

					if (m_effects.empty() || m_effects.back().first < position)
						m_effects.emplace_back(position, phraseNode);

					prevPosition = position;
					end = position + duration;
				};

				switch (phrase)
				{
				case 2:
					check(starPowerEnd, "star power");
					m_effects.back().second.push_back(new StarPowerPhrase(duration));
					break;
				case 64:
					check(starActivationEnd, "star power activation");
					m_effects.back().second.push_back(new StarPowerActivation(duration));
					break;
				default:
					throw "unrecognized special phrase type (" + std::to_string(phrase) + ')';
				}
				break;
			}
			case 'e':
			case 'E':
				prevPosition = position;
				if (strncmp(traversal.getCurrent(), "soloend", 7) == 0)
					addPhrase(position, new Solo(position - solo));
				else if (strncmp(traversal.getCurrent(), "solo", 4) == 0)
					solo = position;
				else
				{
					if (m_events.empty() || m_events.back().first < position)
						m_events.emplace_back(position, eventNode);

					m_events.back().second.push_back(traversal.extractText());
				}
				break;

			default:
				throw "unrecognized node type(" + type + ')';
			}
		}
		catch (std::runtime_error err)
		{
			if (position != UINT32_MAX)
				std::cout << "Line " << traversal.getLineNumber() << " - Position: " << position << err.what() << std::endl;
			else
				std::cout << "Line " << traversal.getLineNumber() << ": position could not be parsed" << std::endl;
		}
		catch (const std::string& str)
		{
			std::cout << "Line " << traversal.getLineNumber() << " - Position: " << position << str << std::endl;
		}

		traversal.next();
	}

	if (m_notes.size() > 10000 && m_notes.size() < m_notes.capacity())
		m_notes.shrink_to_fit();
}
