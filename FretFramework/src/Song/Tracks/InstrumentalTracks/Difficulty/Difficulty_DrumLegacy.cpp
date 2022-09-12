#include <iostream>
#include "Difficulty_DrumLegacy.h"
#include "NoteExceptions.h"
#include "InstrumentalNote_cht.hpp"
#include "InstrumentalNote_bch.hpp"


template <>
template <>
Difficulty<DrumNote<4, DrumPad_Pro>>& Difficulty<DrumNote<4, DrumPad_Pro>>::operator=(Difficulty<DrumNote_Legacy>&& diff)
{
	m_effects = std::move(diff.m_effects);
	m_events = std::move(diff.m_events);
	m_notes.reserve(diff.m_notes.size());
	for (const auto& pair : diff.m_notes)
		m_notes.push_back({ pair.first, pair.second.toDrum4Pro() });
	return *this;
}

template <>
template <>
Difficulty<DrumNote<5, DrumPad>>& Difficulty<DrumNote<5, DrumPad>>::operator=(Difficulty<DrumNote_Legacy>&& diff)
{
	m_effects = std::move(diff.m_effects);
	m_events = std::move(diff.m_events);
	m_notes.reserve(diff.m_notes.size());
	for (const auto& pair : diff.m_notes)
		m_notes.push_back({ pair.first, pair.second.toDrum5() });
	return *this;
}

void Difficulty<DrumNote_Legacy>::load_chart_V1(TextTraversal& traversal)
{
	clear();
	m_notes.reserve(5000);

#ifndef _DEBUG
	static constexpr std::vector<std::u32string> eventNode;
	static constexpr DrumNote_Legacy noteNode;
	static constexpr std::vector<SustainablePhrase*> phraseNode;
#else
	static const std::vector<std::u32string> eventNode;
	static constexpr DrumNote_Legacy noteNode;
	static const std::vector<SustainablePhrase*> phraseNode;
#endif // !_DEBUG

	// End positions to protect from conflicting special phrases
	uint32_t starPowerEnd = 0;
	uint32_t starActivationEnd = 0;
	uint32_t solo = 0;

	while (traversal && traversal != '}' && traversal != '[')
	{
		uint32_t position = UINT32_MAX;
		try
		{
			position = traversal.extractPosition();
			char type = traversal.extract<unsigned char>();
			switch (type)
			{
			case 'N':
			case 'n':
			{
				uint32_t lane = traversal.extract<uint32_t>();
				uint32_t sustain = traversal.extract<uint32_t>();

				if (m_notes.empty() || m_notes.back().first != position)
					m_notes.emplace_back(position, noteNode);

				try
				{
					init_chart_V1(lane, sustain);
				}
				catch (std::runtime_error err)
				{
					if (m_notes.back().second.getNumActive() == 0)
						m_notes.pop_back();
					throw err;
				}
				break;
			}
			case 'S':
			case 's':
			{
				uint32_t phrase = traversal.extract<uint32_t>();
				uint32_t duration = traversal.extract<uint32_t>();
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
				case 64:
					check(starActivationEnd, "star power activation");
					m_effects.back().second.push_back(new StarPowerActivation(duration));
					break;
				default:
					throw "unrecognized special phrase type (" + std::to_string(phrase) + ')';
				}
				break;
			}
			case 'E':
			case 'e':
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
