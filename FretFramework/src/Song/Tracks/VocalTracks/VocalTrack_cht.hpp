#pragma once
#include "VocalTrack.h"

template <size_t numTracks>
void VocalTrack<numTracks>::init_cht_single(uint32_t position, const char* str)
{
	int lane, count;
	if (sscanf_s(str, " %i%n", &lane, &count) != 1)
		throw EndofLineException();

	if (lane > numTracks)
		throw InvalidNoteException(lane);

	str += count;
	if (lane == 0)
	{
		if (m_percussion.empty() || m_percussion.back().first != position)
		{
			static std::pair<uint32_t, VocalPercussion> pairNode;
			pairNode.first = position;
			m_percussion.push_back(pairNode);
		}

		char disable;
		if (sscanf_s(str, " %c", &disable, 1) == 1)
			m_percussion.back().second.modify(disable);
	}
	else
	{
		str += 2;
		char strBuf[256] = { 0 };
		if (sscanf_s(str, "%[^\"]%n", &strBuf, 256, &count) == EOF)
			throw EndofLineException();

		if (m_vocals[lane - 1].empty() || m_vocals[lane - 1].back().first != position)
		{
			static std::pair<uint32_t, Vocal> pairNode;
			pairNode.first = position;
			m_vocals[lane - 1].push_back(pairNode);
		}

		m_vocals[lane - 1].back().second.setLyric(strBuf);
		str += count + 1;

		// Read pitch if found
		int pitch = 0;
		uint32_t sustain = 0;
		switch (sscanf_s(str, " %i %lu", &pitch, &sustain))
		{
		case 2:
			m_vocals[lane - 1].back().second.setPitch(pitch);
			m_vocals[lane - 1].back().second.init(sustain);
			break;
		case 1:
			m_vocals[lane - 1].pop_back();
			throw EndofLineException();
		}
	}
}

template <size_t numTracks>
void VocalTrack<numTracks>::init_cht_chord(uint32_t position, const char* str)
{
	int colors;
	int count;
	if (sscanf_s(str, " %i%n", &colors, &count) != 1)
		throw EndofLineException();

	str += count;
	int i = 0;
	int numAdded = 0;
	int lane;
	while (i < colors && sscanf_s(str, " %i%n", &lane, &count) == 1)
	{
		if (lane > numTracks)
			throw InvalidNoteException(lane);

		if (lane == 0)
		{
			if (m_percussion.empty() || m_percussion.back().first != position)
			{
				static std::pair<uint32_t, VocalPercussion> pairNode;
				pairNode.first = position;
				m_percussion.push_back(pairNode);
				numAdded = 1;

				for (auto& track : m_vocals)
					if (!track.empty() && track.back().first == position)
						track.pop_back();
			}
			str += count;
		}
		else
		{
			str += count + 2;
			char strBuf[256] = { 0 };
			if (sscanf_s(str, "%[^\"]%n", &strBuf, 256, &count) == EOF)
				throw EndofLineException();

			if (m_vocals[lane - 1].empty() || m_vocals[lane - 1].back().first != position)
			{
				static std::pair<uint32_t, Vocal> pairNode;
				pairNode.first = position;
				m_vocals[lane - 1].push_back(pairNode);
			}

			m_vocals[lane - 1].back().second.setLyric(strBuf);

			if (m_percussion.empty() || m_percussion.back().first != position)
				++numAdded;
			else
				m_percussion.pop_back();
			str += count + 1;
		}
		++i;
	}

	if (numAdded == 0)
		throw InvalidNoteException();
}

template <size_t numTracks>
void VocalTrack<numTracks>::modify_cht(uint32_t position, const char* str)
{
	int numMods;
	int count;
	if (sscanf_s(str, " %i%n", &numMods, &count) == 1)
	{
		str += count;
		char modifier;
		for (int i = 0;
			i < numMods && sscanf_s(str, " %c%n", &modifier, 1, &count) == 1;
			++i)
		{
			str += count;
			switch (modifier)
			{
			case 'n':
			case 'N':
				if (!m_percussion.empty() && m_percussion.back().first == position)
					m_percussion.back().second.modify('N');
			}
		}
	}
}

template <size_t numTracks>
void VocalTrack<numTracks>::vocalize_cht(uint32_t position, const char* str)
{
	int numPitches;
	int count;
	if (sscanf_s(str, " %i%n", &numPitches, &count) == 1)
	{
		str += count;
		int lane;
		int pitch;
		uint32_t sustain;

		int i = 0;
		int numVocalized = 0;
		while (i < numPitches && sscanf_s(str, " %i %i %lu%n", &lane, &pitch, &sustain, &count) == 3)
		{
			str += count;
			if (0 < lane && lane <= numTracks &&
				!m_vocals[lane - 1].empty() && m_vocals[lane - 1].back().first == position)
			{
				m_vocals[lane - 1].back().second.setPitch(pitch);
				m_vocals[lane - 1].back().second.init(sustain);
				++numVocalized;
			}
			++i;
		}

		if (numVocalized == 0)
			std::cout << "Unable to vocalize lyrics at position " << std::endl;

		if (i < numPitches)
			throw EndofLineException();
	}
}

template <size_t numTracks>
void VocalTrack<numTracks>::load_cht(std::fstream& inFile)
{
	clear();
	static char buffer[512] = { 0 };
	inFile.getline(buffer, 512);

	int numNotes = 0;
	if (numTracks == 1)
	{
		if (sscanf_s(buffer, "\tLyrics = %lu", &numNotes) == 1)
			m_vocals[0].reserve(numNotes);
	}
	else
	{
		if (sscanf_s(buffer, "\tHarm1 = %lu", &numNotes) == 1)
			m_vocals[0].reserve(numNotes);

		if (numTracks > 1)
		{
			inFile.getline(buffer, 512);
			if (sscanf_s(buffer, "\tHarm2 = %lu", &numNotes) == 1)
				m_vocals[1].reserve(numNotes);

			if (numTracks > 2)
			{
				inFile.getline(buffer, 512);
				if (sscanf_s(buffer, "\tHarm3 = %lu", &numNotes) == 1)
					m_vocals[2].reserve(numNotes);
			}
		}
	}

	inFile.getline(buffer, 512);
	if (sscanf_s(buffer, "\tPercussion = %lu", &numNotes) == 1)
		m_percussion.reserve(numNotes);

	struct
	{
		uint32_t position;
		bool active = false;
	} phrases[2];
	uint32_t prevPosition = 0;
	while (inFile.getline(buffer, 512) && !memchr(buffer, '}', 2))
	{
		const char* str = buffer;
		uint32_t position;
		char type[3] = { 0 };
		int count;
		int numRead = sscanf_s(str, " %lu = %[^ ]%n", &position, type, 3, &count);
		if (numRead == 2 && prevPosition <= position)
		{
			prevPosition = position;
			str += count;
			switch (type[0])
			{
			case 'v':
			case 'V':
				try
				{
					vocalize_cht(position, str);
				}
				catch (EndofLineException EoL)
				{
					std::cout << "Unable to parse full list of pitches at position " << position << " (\"" << str << "\")" << std::endl;
				}
				break;
			case 'p':
			case 'P':
			{
				int index = 0;
				if (numTracks > 1)
				{
					char phraseIndex;
					if (sscanf_s(str, " %c", &phraseIndex, 1) != 0 && (phraseIndex == 'h' || phraseIndex == 'H'))
						index = 1;
				}

				switch (type[1])
				{
					// Phrase Start
				case 0:
					if (phrases[index].active)
					{
						if (index == 0)
							addPhrase(phrases[0].position, new LyricLine(position - phrases[0].position));
						else
							addPhrase(phrases[1].position, new HarmonyLine(position - phrases[1].position));
					}
					phrases[index].position = position;
					phrases[index].active = true;
					break;
					// Phrase End
				case 'e':
				case 'E':
					if (index == 0)
						addPhrase(phrases[0].position, new LyricLine(position - phrases[0].position));
					else
						addPhrase(phrases[1].position, new HarmonyLine(position - phrases[1].position));
					phrases[index].active = false;
				}
				break;
			}
			case 'e':
			case 'E':
			{
				++str;
				if (*str == '\"')
					++str;

				char strBuf[256] = { 0 };
				sscanf_s(str, "%[^\"]s", &strBuf, 256);
				if (m_events.empty() || m_events.back().first < position)
				{
					static std::pair<uint32_t, std::vector<std::string>> pairNode;
					pairNode.first = position;
					m_events.push_back(pairNode);
				}

				m_events.back().second.push_back(strBuf);
				break;
			}
			case 'n':
			case 'N':
				init_cht_single(position, str);
				break;
			case 'c':
			case 'C':
				init_cht_chord(position, str);
				break;
			case 'm':
			case 'M':
				modify_cht(position, str);
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
							static std::pair<uint32_t, std::vector<Phrase*>> pairNode;
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
						check();
						m_effects.back().second.push_back(new LyricLine(duration));
						break;
					case 5:
						check();
						m_effects.back().second.push_back(new RangeShift(duration));
						break;
					case 6:
						check();
						m_effects.back().second.push_back(new HarmonyLine(duration));
						break;
					case 64:
					case 65:
					case 66:
						break;
					case 67:
						check();
						m_effects.back().second.push_back(new LyricShift());
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
	{
		std::cout << "Error in track Vocals" << std::endl;
		throw EndofFileException();
	}

	for (auto& track : m_vocals)
		if (track.size() < track.capacity())
			track.shrink_to_fit();

	if (m_percussion.size() < m_percussion.capacity())
		m_percussion.shrink_to_fit();
}
