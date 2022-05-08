#pragma once
#include <iostream>
#include "Difficulty.h"
#include "NoteExceptions.h"

template <typename T>
void Difficulty<T>::load_chart_V1(std::fstream& inFile)
{
	clear();
	static char buffer[512] = { 0 };
	uint32_t solo = 0;
	uint32_t prevPosition = 0;
	while (inFile.getline(buffer, 512) && buffer[0] != '}')
	{
		const char* str = buffer;
		uint32_t position = UINT32_MAX;
		char type;
		int count;
		int numRead = sscanf_s(str, " %lu = %c%n", &position, &type, 1, &count);
		if (numRead == 2 && prevPosition <= position)
		{
			prevPosition = position;
			str += count;
			switch (type)
			{
			case 'e':
			case 'E':
				++str;
				if (strncmp(str, "soloend", 7) == 0)
					addPhrase(position, new Solo(position - solo));
				else if (strncmp(str, "solo", 4) == 0)
					solo = position;
				else
					addEvent(position, std::string(str));
				break;
			case 'n':
			case 'N':
			{
				int lane;
				uint32_t sustain = 0;
				if (sscanf_s(str, " %i %lu", &lane, &sustain) != 0)
				{
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
						std::cout << "Error at position " << position << ": " << INE.what() << std::endl;
					}
				}
				break;
			}
			case 's':
			case 'S':
			{
				int phrase;
				uint32_t duration = 0;
				if (sscanf_s(str, " %i %lu", &phrase, &duration) != 0)
				{
					auto check = [&]()
					{
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
				std::cout << "Error at position: unrecognized node type (" << type << ')' << std::endl;
			}
		}
		else
		{
			// Need to add a line count tracking variable for easy debugging by the end-user
			std::cout << "Error reading line: ";
			if (numRead != 2)
				std::cout << "Improper node setup (\"" << str << "\")" << std::endl;
			else
				std::cout << "Node position out of order (" << position << ')' << std::endl;
		}
	}

	if (!inFile)
		throw EndofFileException();

	if (m_notes.size() < m_notes.capacity())
		m_notes.shrink_to_fit();
}

template <typename T>
void Difficulty<T>::load_cht(std::fstream& inFile)
{
	clear();
	static char buffer[512] = { 0 };
	inFile.getline(buffer, 512);

	int numNotes = 0;
	if (sscanf_s(buffer, "\t\tNotes = %lu", &numNotes) == 1)
		m_notes.reserve(numNotes);

	uint32_t prevPosition = 0;
	while (inFile.getline(buffer, 512) && !memchr(buffer, '}', 2))
	{
		const char* str = buffer;
		uint32_t position;
		char type;
		int count;
		int numRead = sscanf_s(str, " %lu = %c%n", &position, &type, 1, &count);
		if (numRead == 2 && prevPosition <= position)
		{
			prevPosition = position;
			str += count;
			switch (type)
			{
			case 'e':
			case 'E':
				if (m_events.empty() || m_events.back().first < position)
				{
					static std::pair<uint32_t, std::vector<std::string>> pairNode;
					pairNode.first = position;
					m_events.push_back(pairNode);
				}

				m_events.back().second.push_back({ str + 1 });
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
						m_notes.back().second.init_cht_chord(str);
						break;
					default:
						m_notes.back().second.init_cht_single(str);
					}
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
				if (!m_notes.empty() && m_notes.back().first == position)
					m_notes.back().second.modify_cht(str);
				break;
			case 's':
			case 'S':
			{
				int phrase;
				uint32_t duration = 0;
				if (sscanf_s(str, " %i %lu", &phrase, &duration) != 0)
				{
					auto check = [&]()
					{
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
				std::cout << "Error at position: unrecognized node type (" << type << ')' << std::endl;
			}
		}
		else
		{
			// Need to add a line count tracking variable for easy debugging by the end-user
			std::cout << "Error reading line: ";
			if (numRead != 2)
				std::cout << "Improper node setup (\"" << str << "\")" << std::endl;
			else
				std::cout << "Node position out of order (" << position << ')' << std::endl;
		}
	}

	// buffer[1] is the expected value
	if (!inFile || buffer[0] == '}')
	{
		std::cout << "Error in difficulty " << m_name << std::endl;
		if (!inFile)
			throw EndofFileException();
		else
			throw EndofTrackException();
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
