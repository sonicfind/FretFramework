#pragma once
#include <iostream>
#include "Difficulty.h"
#include "NoteExceptions.h"
#include "..\TextFileManip.h"

template <typename T>
void Difficulty<T>::load_chart_V1(TextTraversal& traversal)
{
	clear();
	uint32_t solo = 0;
	uint32_t prevPosition = 0;
	while (traversal && traversal != '}' && traversal != '[')
	{
		uint32_t position = UINT32_MAX;
		if (size_t count = traversal.extractUInt(position))
		{
			if (prevPosition <= position)
			{
				traversal.move(count);
				traversal.skipEqualsSign();

				switch (traversal.getChar())
				{
				case 'e':
				case 'E':
					traversal.move(1);
					prevPosition = position;
					if (strncmp(traversal.getCurrent(), "soloend", 7) == 0)
						addPhrase(position, new Solo(position - solo));
					else if (strncmp(traversal.getCurrent(), "solo", 4) == 0)
						solo = position;
					else
					{
						if (m_events.empty() || m_events.back().first < position)
						{
							static std::pair<uint32_t, std::vector<std::string>> pairNode;
							pairNode.first = position;
							m_events.push_back(pairNode);
						}

						m_events.back().second.push_back(std::string(traversal.extractText()));
					}
					break;
				case 'n':
				case 'N':
				{
					traversal.move(1);
					unsigned int lane;
					if (count = traversal.extractUInt(lane))
					{
						prevPosition = position;
						traversal.move(count);

						uint32_t sustain;
						traversal.extractUInt(sustain);
						if (m_notes.empty() || m_notes.back().first != position)
						{
							static std::pair<uint32_t, T> pairNode;
							pairNode.first = position;
							m_notes.push_back(pairNode);
						}

						try
						{
							m_notes.back().second.init_chartV1(lane, sustain);
						}
						catch (InvalidNoteException INE)
						{
							std::cout << "Line " << traversal.getLineNumber() << ": " << INE.what() << std::endl;
						}
					}
					break;
				}
				case 's':
				case 'S':
				{
					traversal.move(1);
					uint32_t phrase;
					if (count = traversal.extractUInt(phrase))
					{
						uint32_t duration = 0;
						auto check = [&]()
						{
							prevPosition = position;
							traversal.move(count);
							traversal.extractUInt(duration);
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
							addPhrase(position, new StarPowerPhrase(duration));
							break;
						case 64:
							check();
							addPhrase(position, new StarPowerActivation(duration));
							break;
						default:
							std::cout << "Error at position " << position << ": unrecognized special phrase type (" << phrase << ')' << std::endl;
						}
					}
					break;
				}
				default:
					// Need to add a line count tracking variable for easy debugging by the end-user
					std::cout << "Error at position: unrecognized node type (" << traversal.getChar() << ')' << std::endl;
				}
			}
			else
				std::cout << "Error: Node position out of order\n\t Line: \"" << traversal.extractText() << '\"' << std::endl;
		}
		else if (traversal != '\n')
			// Need to add a line count tracking variable for easy debugging by the end-user
			std::cout << "Error: Improper node setup\n\t Line: \"" << traversal.extractText() << '\"' << std::endl;

		traversal.nextLine();
	}

	if (m_notes.size() < m_notes.capacity())
		m_notes.shrink_to_fit();
}

template <typename T>
void Difficulty<T>::load_cht(TextTraversal& traversal)
{
	clear();

	if (strncmp(traversal.getCurrent(), "Notes", 5) == 0)
	{
		traversal.move(5);
		traversal.skipEqualsSign();

		uint32_t numNotes;
		traversal.extractUInt(numNotes);
		m_notes.reserve(numNotes);
		traversal.nextLine();
	}

	uint32_t prevPosition = 0;
	while (traversal && traversal != '}' && traversal != '[')
	{
		uint32_t position;
		if (size_t count = traversal.extractUInt(position))
		{
			if (prevPosition <= position)
			{
				traversal.move(count);
				traversal.skipEqualsSign();

				switch (traversal.getChar())
				{
				case 'e':
				case 'E':
					traversal.move(1);
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
						switch (traversal.getChar())
						{
						case 'c':
						case 'C':
							traversal.move(1);
							m_notes.back().second.init_cht_chord(traversal);
							break;
						default:
							traversal.move(1);
							m_notes.back().second.init_cht_single(traversal);
						}
						prevPosition = position;
					}
					catch (EndofLineException EOL)
					{
						std::cout << "Failed to parse note at tick position " << position << std::endl;
						m_notes.pop_back();
					}
					catch (InvalidNoteException INE)
					{
						std::cout << "Note at tick position " << position << " had no valid colors" << std::endl;
						m_notes.pop_back();
					}
					break;
				case 'm':
				case 'M':
					traversal.move(1);
					if (!m_notes.empty() && m_notes.back().first == position)
						m_notes.back().second.modify_cht(traversal);
					break;
				case 's':
				case 'S':
				{
					traversal.move(1);
					uint32_t phrase;
					if (count = traversal.extractUInt(phrase))
					{
						uint32_t duration = 0;
						auto check = [&]()
						{
							prevPosition = position;
							traversal.move(count);
							traversal.extractUInt(duration);
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
							std::cout << "Error at position " << position << ": unrecognized special phrase type (" << phrase << ')' << std::endl;
						}
					}
					break;
				}
				default:
					// Need to add a line count tracking variable for easy debugging by the end-user
					std::cout << "Error at position: unrecognized node type (" << traversal.getChar() << ')' << std::endl;
				}
			}
			else
				std::cout << "Error: Node position out of order\n\t Line: \"" << traversal.extractText() << '\"' << std::endl;
		}
		else if (traversal != '\n')
			// Need to add a line count tracking variable for easy debugging by the end-user
			std::cout << "Error: Improper node setup\n\t Line: \"" << traversal.extractText() << '\"' << std::endl;

		traversal.nextLine();
	}

	if (m_notes.size() < m_notes.capacity())
		m_notes.shrink_to_fit();
}

template <typename T>
void Difficulty<T>::save_cht(std::fstream& outFile) const
{
	outFile << '\t' << m_name << "\n\t{\n";
	outFile << "\t\tNotes = " << m_notes.size() << '\n';

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
