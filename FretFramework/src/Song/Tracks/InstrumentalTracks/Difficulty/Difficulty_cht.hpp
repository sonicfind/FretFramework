#pragma once
#include <iostream>
#include "Difficulty.h"
#include "NoteExceptions.h"
#include "InstrumentalNote_cht.hpp"
#include "Chords\GuitarNote\GuitarNote_cht.hpp"

#define PHRASESCAN(end) if (position >= end) { end = position + traversal.extractU32(); prevPosition = position; }

template<typename T>
inline bool Difficulty<T>::scan_chart_V1(TextTraversal& traversal)
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
					T().init_chartV1(traversal.extractU32(), traversal.extractU32());

					// So long as the init does not throw an exception, it can be concluded that this difficulty does contain notes
					// No need to check the rest of the difficulty's data
					while (traversal.next() && traversal != '}' && traversal != '[');
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

template <typename T>
void Difficulty<T>::load_chart_V1(TextTraversal& traversal)
{
	clear();
	m_notes.reserve(5000);

	const static std::vector<std::string> eventNode;
	const static T noteNode;
	const static std::vector<SustainablePhrase*> phraseNode;

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
					m_notes.back().second.init_chartV1(lane, sustain);
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

template<typename T>
inline bool Difficulty<T>::scan_cht(TextTraversal& traversal)
{
	// End positions to protect from conflicting special phrases
	uint32_t starPowerEnd = 0;
	uint32_t soloEnd = 0;
	uint32_t starActivationEnd = 0;
	uint32_t tremoloEnd = 0;
	uint32_t trillEnd = 0;

	uint32_t prevPosition = 0;
	do
	{
		if (traversal == '}' || traversal == '[')
			break;

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
					T().init_single(traversal);

					// So long as the init does not throw an exception, it can be concluded that this difficulty does contain notes
					// No need to check the rest of the difficulty's data
					while (traversal.next() && traversal != '}' && traversal != '[');
					return true;
				case 'c':
				case 'C':
					T().init_chord(traversal);

					// So long as the init does not throw an exception, it can be concluded that this difficulty does contain notes
					// No need to check the rest of the difficulty's data
					while (traversal.next() && traversal != '}' && traversal != '[');
					return true;
				case 'e':
				case 'E':
					prevPosition = position;
					break;
				case 's':
				case 'S':
				{
					uint32_t phrase = traversal.extractU32();
					switch (phrase)
					{
					case 2:
						PHRASESCAN(starPowerEnd)
						break;
					case 3:
						PHRASESCAN(soloEnd)
						break;
					case 64:
						PHRASESCAN(starActivationEnd)
						break;
					case 65:
						PHRASESCAN(tremoloEnd)
						break;
					case 66:
						PHRASESCAN(trillEnd)
						break;
					}
					break;
				}
				}
			}
		}
		catch (std::runtime_error err)
		{
		}
	} while (traversal.next());

	// If the execution of the function reaches here, it can be concluded that the difficulty does not contain any notes
	return false;
}

template <typename T>
void Difficulty<T>::load_cht(TextTraversal& traversal)
{
	clear();
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

	uint32_t prevPosition = 0;
	do
	{
		if (traversal == '}' || traversal == '[')
			break;

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
				try
				{
					if (m_notes.empty() || m_notes.back().first != position)
						m_notes.emplace_back(position, noteNode);

					m_notes.back().second.init_single(traversal);
					prevPosition = position;
				}
				catch (std::runtime_error err)
				{
					if (m_notes.back().second.getNumActive() == 0)
						m_notes.pop_back();
					throw err;
				}
				break;
			case 'c':
			case 'C':
				try
				{
					if (m_notes.empty() || m_notes.back().first != position)
						m_notes.emplace_back(position, noteNode);

					m_notes.back().second.init_chord(traversal);
					prevPosition = position;
				}
				catch (std::runtime_error err)
				{
					m_notes.pop_back();
					throw err;
				}
				break;
			case 'e':
			case 'E':
				prevPosition = position;
				if (m_events.empty() || m_events.back().first < position)
					m_events.emplace_back(position, eventNode);

				m_events.back().second.push_back(traversal.extractText());
				break;
			case 'm':
			case 'M':
				if (!m_notes.empty() && m_notes.back().first == position)
					m_notes.back().second.modify(traversal);
				break;
			case 's':
			case 'S':
			{
				uint32_t phrase = traversal.extractU32();
				uint32_t duration = 0;
				auto check = [&](uint32_t& end, const char* noteType)
				{
					// Handles phrase conflicts
					if (position < end)
						throw std::string(noteType) + " note conflicts with current active " + noteType + " phrase (ending at tick " + std::to_string(end) + ')';

					if (m_effects.empty() || m_effects.back().first < position)
						m_effects.emplace_back(position, phraseNode);

					prevPosition = position;

					traversal.extract(duration);
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
					throw "unrecognized special phrase type (" + std::to_string(phrase) + ')';
				}
				break;
			}
			default:
				throw std::string("unrecognized node type(") + type + ')';
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
	} while (traversal.next());

	if ((m_notes.size() < 500 || 10000 <= m_notes.size()) && m_notes.size() < m_notes.capacity())
		m_notes.shrink_to_fit();
}

template <typename T>
void Difficulty<T>::save_cht(std::fstream& outFile) const
{
	outFile << '\t' << m_name << "\n\t{\n";

	auto noteIter = m_notes.begin();
	auto effectIter = m_effects.begin();
	auto eventIter = m_events.begin();
	bool notesValid = noteIter != m_notes.end();
	bool effectValid = effectIter != m_effects.end();
	bool eventValid = eventIter != m_events.end();

	while (notesValid || effectValid || eventValid)
	{
		while (effectValid &&
			(!notesValid || effectIter->first <= noteIter->first) &&
			(!eventValid || effectIter->first <= eventIter->first))
		{
			for (const auto& eff : effectIter->second)
				eff->save_cht(effectIter->first, outFile);
			effectValid = ++effectIter != m_effects.end();
		}

		while (notesValid &&
			(!effectValid || noteIter->first < effectIter->first) &&
			(!eventValid || noteIter->first <= eventIter->first))
		{
			noteIter->second.save_cht(noteIter->first, outFile);
			notesValid = ++noteIter != m_notes.end();
		}

		while (eventValid &&
			(!effectValid || eventIter->first < effectIter->first) &&
			(!notesValid || eventIter->first < noteIter->first))
		{
			for (const auto& str : eventIter->second)
				outFile << "\t\t" << eventIter->first << " = E \"" << str << "\"\n";
			eventValid = ++eventIter != m_events.end();
		}
	}

	outFile << "\t}\n";
	outFile.flush();
}
