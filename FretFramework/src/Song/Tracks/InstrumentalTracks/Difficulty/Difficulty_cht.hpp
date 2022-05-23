#pragma once
#include <iostream>
#include "Difficulty.h"
#include "NoteExceptions.h"
#include "InstrumentalNote_cht.hpp"
#include "Chords\GuitarNote\GuitarNote_cht.hpp"

template <typename T>
void Difficulty<T>::load_chart_V1(TextTraversal& traversal)
{
	clear();
	uint32_t solo = 0;
	uint32_t prevPosition = 0;
	m_notes.reserve(5000);
	const static std::vector<std::string> eventNode;
	const static T noteNode;
	const static std::vector<SustainablePhrase*> phraseNode;
	while (traversal && traversal != '}' && traversal != '[')
	{
		uint32_t position = UINT32_MAX;
		if (traversal.extract(position))
		{
			if (prevPosition <= position)
			{
				traversal.skipEqualsSign();
				char type = traversal.extract();
				switch (type)
				{
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

						m_events.back().second.push_back(std::string(traversal.extractText()));
					}
					break;
				case 'n':
				case 'N':
				{
					unsigned int lane;
					if (traversal.extract(lane))
					{
						prevPosition = position;

						uint32_t sustain;
						traversal.extract(sustain);
						if (m_notes.empty() || m_notes.back().first != position)
							m_notes.emplace_back(position, noteNode);

						try
						{
							m_notes.back().second.init_chartV1(lane, sustain);
						}
						catch (std::runtime_error err)
						{
							std::cout << "Line " << traversal.getLineNumber() << " - Position " << position << ": " << err.what() << std::endl;
							if (m_notes.back().second.getNumActive() == 0)
								m_notes.pop_back();
						}
					}
					break;
				}
				case 's':
				case 'S':
				{
					uint32_t phrase;
					if (traversal.extract(phrase))
					{
						uint32_t duration = 0;
						auto check = [&]()
						{
							prevPosition = position;
							traversal.extract(duration);
							if (m_effects.empty() || m_effects.back().first < position)
								m_effects.emplace_back(position, phraseNode);
						};

						switch (phrase)
						{
						case 2:
							check();
							addPhrase(position, new StarPowerPhrase(duration));
							break;
						case 64:
							check();
							addPhrase(position, new StarPowerActivation(duration));
							break;
						default:
							std::cout << "Line " << traversal.getLineNumber() << ": unrecognized special phrase type (" << phrase << ')' << std::endl;
						}
					}
					break;
				}
				default:
					std::cout << "Line " << traversal.getLineNumber() << ": unrecognized node type(" << type << ')' << std::endl;
				}
			}
			else
				std::cout << "Line " << traversal.getLineNumber() << ": position out of order; Previous: " << prevPosition << "; Current: " << position << std::endl;
		}
		else if (traversal != '\n')
			std::cout << "Line " << traversal.getLineNumber() << ": improper node setup" << std::endl;

		traversal.next();
	}

	if (m_notes.size() > 10000 && m_notes.size() < m_notes.capacity())
		m_notes.shrink_to_fit();
}

template <typename T>
void Difficulty<T>::load_cht(TextTraversal& traversal)
{
	clear();

	m_notes.reserve(5000);
	uint32_t prevPosition = 0;
	do
	{
		if (traversal == '}' || traversal == '[')
			break;

		uint32_t position;
		if (traversal.extract(position))
		{
			if (prevPosition <= position)
			{
				traversal.skipEqualsSign();
				char type = traversal.extract();
				switch (type)
				{
				case 'e':
				case 'E':
					prevPosition = position;
					if (m_events.empty() || m_events.back().first < position)
					{
						static std::pair<uint32_t, std::vector<std::string>> pairNode;
						pairNode.first = position;
						m_events.push_back(pairNode);
					}

					m_events.back().second.push_back(std::string(traversal.extractText()));
					break;
				case 'n':
				case 'N':
				case 'c':
				case 'C':
					
					if (m_notes.empty() || m_notes.back().first != position)
					{
						static std::pair<uint32_t, T> pairNode;
						pairNode.first = position;
						m_notes.push_back(pairNode);
					}

					try
					{
						switch (type)
						{
						case 'c':
						case 'C':
							m_notes.back().second.init_chord(traversal);
							break;
						default:
							m_notes.back().second.init_single(traversal);
						}
						prevPosition = position;
					}
					catch (std::runtime_error err)
					{
						std::cout << "Line " << traversal.getLineNumber() << " - Position " << position << ": " << err.what() << std::endl;
						if (type == 'c' || type == 'C' || m_notes.back().second.getNumActive() == 0)
							m_notes.pop_back();
					}
					break;
				case 'm':
				case 'M':
					if (!m_notes.empty() && m_notes.back().first == position)
						m_notes.back().second.modify(traversal);
					break;
				case 's':
				case 'S':
				{
					uint32_t phrase;
					if (traversal.extract(phrase))
					{
						uint32_t duration = 0;
						auto check = [&]()
						{
							prevPosition = position;
							traversal.extract(duration);
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
							std::cout << "Line " << traversal.getLineNumber() << " - Position " << position << ": unrecognized special phrase type (" << phrase << ')' << std::endl;
						}
					}
					break;
				}
				default:
					std::cout << "Line " << traversal.getLineNumber() << " - Position " << position << ": unrecognized node type(" << type << ')' << std::endl;
				}
			}
			else
				std::cout << "Line " << traversal.getLineNumber() << " - Position " << position << ": position out of order; Previous: " << prevPosition << "; Current: " << position << std::endl;
		}
		else if (traversal != '\n')
			std::cout << "Line " << traversal.getLineNumber() << ": improper node setup (" << traversal.extractText() << ')' << std::endl;
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
