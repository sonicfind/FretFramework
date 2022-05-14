#pragma once
#include "VocalTrack.h"

template <int numTracks>
inline void VocalTrack<numTracks>::init_cht_single(uint32_t position, TextTraversal& traversal)
{
	uint32_t lane;
	if (!traversal.extractUInt(lane))
		throw EndofLineException();

	if (lane > numTracks)
		throw InvalidNoteException(lane);

	if (lane == 0)
	{
		if (m_percussion.empty() || m_percussion.back().first != position)
		{
			static std::pair<uint32_t, VocalPercussion> pairNode;
			pairNode.first = position;
			m_percussion.push_back(pairNode);
		}

		if (traversal == 'N' || traversal == 'n')
			m_percussion.back().second.modify('N');
	}
	else
	{
		if (m_vocals[lane - 1].empty() || m_vocals[lane - 1].back().first != position)
		{
			static std::pair<uint32_t, Vocal> pairNode;
			pairNode.first = position;
			m_vocals[lane - 1].push_back(pairNode);
		}

		m_vocals[lane - 1].back().second.setLyric(std::string(traversal.extractText()));

		// Read pitch if found
		uint32_t pitch;
		if (traversal.extractUInt(pitch))
		{
			uint32_t sustain;
			if (traversal.extractUInt(sustain))
			{
				m_vocals[lane - 1].back().second.setPitch(pitch);
				m_vocals[lane - 1].back().second.init(sustain);
			}
			else
			{
				m_vocals[lane - 1].pop_back();
				throw EndofLineException();
			}
		}
	}
}

template <int numTracks>
inline void VocalTrack<numTracks>::init_cht_chord(uint32_t position, TextTraversal& traversal)
{
	uint32_t colors;
	if (traversal.extractUInt(colors))
	{
		int numAdded = 0;
		uint32_t lane;
		for (uint32_t i = 0; i < colors; ++i)
		{
			if (!traversal.extractUInt(lane))
				throw EndofLineException();

			if (lane > numTracks)
				throw InvalidNoteException(lane);

			if (lane == 0)
			{
				if (m_percussion.empty() || m_percussion.back().first != position)
				{
					for (auto& track : m_vocals)
						if (!track.empty() && track.back().first == position)
							track.pop_back();

					static std::pair<uint32_t, VocalPercussion> pairNode;
					pairNode.first = position;
					m_percussion.push_back(pairNode);
					numAdded = 1;
				}
			}
			else if (m_vocals[lane - 1].empty() || m_vocals[lane - 1].back().first != position)
			{
				if (!m_percussion.empty() && m_percussion.back().first == position)
					m_percussion.pop_back();

				static std::pair<uint32_t, Vocal> pairNode;
				pairNode.first = position;
				m_vocals[lane - 1].push_back(pairNode);
				m_vocals[lane - 1].back().second.setLyric(std::string(traversal.extractText()));
				++numAdded;
			}
		}

		if (numAdded == 0)
			throw InvalidNoteException();
	}
	else
		throw EndofLineException();
}

template <int numTracks>
inline void VocalTrack<numTracks>::modify_cht(uint32_t position, TextTraversal& traversal)
{
	uint32_t numMods;
	if (traversal.extractUInt(numMods))
	{
		for (uint32_t i = 0; i < numMods; ++i)
		{
			switch (traversal.getChar())
			{
			case 'n':
			case 'N':
				if (!m_percussion.empty() && m_percussion.back().first == position)
					m_percussion.back().second.modify('N');
			}
			traversal.move(1);
		}
	}
}

template <int numTracks>
inline void VocalTrack<numTracks>::vocalize_cht(uint32_t position, TextTraversal& traversal)
{
	uint32_t numPitches;
	if (traversal.extractUInt(numPitches))
	{
		int numVocalized = 0;
		for (uint32_t i = 0; i < numPitches; ++i)
		{
			uint32_t lane;
			if (!traversal.extractUInt(lane))
				throw EndofLineException();

			uint32_t pitch;
			if (!traversal.extractUInt(pitch))
				throw EndofLineException();

			uint32_t sustain;
			if (!traversal.extractUInt(sustain))
				throw EndofLineException();
			
			if (0 < lane && lane <= numTracks &&
				!m_vocals[lane - 1].empty() && m_vocals[lane - 1].back().first == position)
			{
				m_vocals[lane - 1].back().second.setPitch(pitch);
				m_vocals[lane - 1].back().second.init(sustain);
				++numVocalized;
			}
		}

		if (numVocalized == 0)
			std::cout << "Unable to vocalize lyrics at position " << std::endl;
	}
}

template <int numTracks>
inline void VocalTrack<numTracks>::load_cht(TextTraversal& traversal)
{
	clear();

	if (numTracks == 1)
	{
		if (strncmp(traversal.getCurrent(), "Lyrics", 6) == 0)
		{
			traversal.move(6);
			traversal.skipEqualsSign();

			uint32_t numNotes;
			traversal.extractUInt(numNotes);
			m_vocals[0].reserve(numNotes);
			traversal.next();
		}
		else
			m_vocals[0].reserve(1000);
	}
	else if (numTracks > 1)
	{
		if (strncmp(traversal.getCurrent(), "Harm1", 5) == 0)
		{
			traversal.move(5);
			traversal.skipEqualsSign();

			uint32_t numNotes;
			traversal.extractUInt(numNotes);
			m_vocals[0].reserve(numNotes);
			traversal.next();
		}
		else
			m_vocals[0].reserve(1000);

		if (strncmp(traversal.getCurrent(), "Harm2", 5) == 0)
		{
			traversal.move(5);
			traversal.skipEqualsSign();

			uint32_t numNotes;
			traversal.extractUInt(numNotes);
			m_vocals[1].reserve(numNotes);
			traversal.next();
		}
		else
			m_vocals[1].reserve(1000);

		if (numTracks > 2)
		{
			if (strncmp(traversal.getCurrent(), "Harm3", 5) == 0)
			{
				traversal.move(5);
				traversal.skipEqualsSign();

				uint32_t numNotes;
				traversal.extractUInt(numNotes);
				m_vocals[2].reserve(numNotes);
				traversal.next();
			}
			else
				m_vocals[2].reserve(1000);
		}
	}

	if (strncmp(traversal.getCurrent(), "Percussion", 10) == 0)
	{
		traversal.move(6);
		traversal.skipEqualsSign();

		uint32_t numNotes;
		traversal.extractUInt(numNotes);
		m_percussion.reserve(numNotes);
		traversal.next();
	}

	struct
	{
		uint32_t position = 0;
		bool active = false;
	} phrases[2];

	uint32_t prevPosition = 0;
	while (traversal && traversal != '}' && traversal != '[')
	{
		uint32_t position = UINT32_MAX;
		if (traversal.extractUInt(position))
		{
			if (prevPosition <= position)
			{
				traversal.skipEqualsSign();
				switch (traversal.getChar())
				{
				case 'v':
				case 'V':
					traversal.move(1);
					try
					{
						vocalize_cht(position, traversal);
					}
					catch (EndofLineException EoL)
					{
						std::cout << "Unable to parse full list of pitches at position " << position << std::endl;
					}
					break;
				case 'p':
				case 'P':
				{
					bool phraseStart = false;
					if (strncmp(traversal.getCurrent(), "PE", 2) == 0)
					{
						phraseStart = true;
						traversal.move(2);
					}
					else if (*(traversal.getCurrent() + 1) == ' ')
						traversal.move(1);
					else
						break;

					prevPosition = position;

					int index = 0;
					if (numTracks > 1)
					{
						if (traversal == 'h' || traversal == 'H')
							index = 1;
					}

					if (phraseStart)
					{
						if (phrases[index].active)
						{
							if (index == 0)
								addPhrase(phrases[0].position, new LyricLine(position - phrases[0].position));
							else
								addPhrase(phrases[1].position, new HarmonyLine(position - phrases[1].position));
						}
						phrases[index].position = position;
						phrases[index].active = true;
					}
					else
					{
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
				}
				case 'n':
				case 'N':
				case 'c':
				case 'C':
					try
					{
						switch (traversal.getChar())
						{
						case 'n':
						case 'N':
							traversal.move(1);
							init_cht_single(position, traversal);
							break;
						default:
							traversal.move(1);
							init_cht_chord(position, traversal);
						}
					}
					catch (EndofLineException EOL)
					{
						std::cout << "Failed to parse full note at tick position " << position << std::endl;
						for (auto& track : m_vocals)
							if (!track.empty() && track.back().first == position)
								track.pop_back();
					}
					catch (InvalidNoteException INE)
					{
						std::cout << "Note at tick position " << position << " had no valid lane numbers" << std::endl;
						for (auto& track : m_vocals)
							if (!track.empty() && track.back().first == position)
								track.pop_back();
					}
					break;
				case 'm':
				case 'M':
					traversal.move(1);
					modify_cht(position, traversal);
					break;
				case 's':
				case 'S':
				{
					traversal.move(1);
					uint32_t phrase;
					if (traversal.extractUInt(phrase))
					{
						uint32_t duration = 0;
						auto check = [&]()
						{
							prevPosition = position;
							traversal.extractUInt(duration);
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
							std::cout << "Line " << traversal.getLineNumber() << ": unrecognized special phrase type (" << phrase << ')' << std::endl;
						}
					}
					break;
				}
				default:
					std::cout << "Line " << traversal.getLineNumber() << ": unrecognized node type(" << traversal.getChar() << ')' << std::endl;
				}
			}
			else
				std::cout << "Line " << traversal.getLineNumber() << ": position out of order; Previous: " << prevPosition << "; Current: " << position << std::endl;
		}
		else if (traversal != '\n')
			std::cout << "Line " << traversal.getLineNumber() << ": improper node setup" << std::endl;

		traversal.next();
	}

	for (auto& track : m_vocals)
		if (track.size() < track.capacity())
			track.shrink_to_fit();

	if (m_percussion.size() < m_percussion.capacity())
		m_percussion.shrink_to_fit();
}

#include "VocalGroup.h"

template <int numTracks>
inline void VocalTrack<numTracks>::save_cht(std::fstream& outFile) const
{
	if (!occupied())
		return;

	std::vector<std::pair<uint32_t, VocalGroup<numTracks>>> vocalGroups;
	{
		int i = 0;
		while (i < 3 && m_vocals[i].empty())
			++i;

		if (i < 3)
		{
			vocalGroups.reserve(m_vocals[i].size());
			for (const auto& vocal : m_vocals[i])
			{
				static std::pair<uint32_t, VocalGroup<numTracks>> pairNode;
				pairNode.first = vocal.first;
				pairNode.second.m_vocals[i] = &vocal.second;
				vocalGroups.push_back(pairNode);
			}
			++i;

			while (i < 3)
			{
				for (const auto& vocal : m_vocals[i])
					VectorIteration::try_emplace(vocalGroups, vocal.first).m_vocals[i] = &vocal.second;
				++i;
			}
		}
	}

	outFile << "[" << m_name << "]\n{\n";
	if (numTracks == 1)
		outFile << "\tLyrics = " << m_vocals[0].size() << '\n';
	else if (numTracks > 1)
	{
		outFile << "\tHarm1 = " << m_vocals[0].size() << '\n';
		outFile << "\tHarm2 = " << m_vocals[1].size() << '\n';
		if (numTracks > 2)
			outFile << "\tHarm3 = " << m_vocals[2].size() << '\n';
	}
	outFile << "\tPercussion = " << m_percussion.size() << '\n';

	auto vocalIter = vocalGroups.begin();
	auto percIter = m_percussion.begin();
	auto effectIter = m_effects.begin();
	auto eventIter = m_events.begin();
	bool vocalValid = vocalIter != vocalGroups.end();
	auto percValid = percIter != m_percussion.end();
	bool effectValid = effectIter != m_effects.end();
	bool eventValid = eventIter != m_events.end();

	while (effectValid || vocalValid || eventValid)
	{
		while (effectValid &&
			(!vocalValid || effectIter->first <= vocalIter->first) &&
			(!percValid || effectIter->first <= percIter->first) &&
			(!eventValid || effectIter->first <= eventIter->first))
		{
			for (const auto& eff : effectIter->second)
				eff->save_cht(effectIter->first, outFile, "\t");
			effectValid = ++effectIter != m_effects.end();
		}

		while (vocalValid &&
			(!effectValid || vocalIter->first < effectIter->first) &&
			(!percValid || vocalIter->first <= percIter->first) &&
			(!eventValid || vocalIter->first <= eventIter->first))
		{
			vocalIter->second.save_cht(vocalIter->first, outFile);
			vocalValid = ++vocalIter != vocalGroups.end();
		}

		while (percValid &&
			(!effectValid || percIter->first < effectIter->first) &&
			(!vocalValid || percIter->first < vocalIter->first) &&
			(!eventValid || percIter->first <= eventIter->first))
		{
			outFile << '\t' << percIter->first << " = N";
			percIter->second.save_cht(0, outFile);
			percIter->second.save_modifier_cht(outFile);
			outFile << '\n';
			percValid = ++percIter != m_percussion.end();
		}

		while (eventValid &&
			(!effectValid || eventIter->first < effectIter->first) &&
			(!percValid || eventIter->first < percIter->first) &&
			(!vocalValid || eventIter->first < vocalIter->first))
		{
			for (const auto& str : eventIter->second)
				outFile << "\t" << eventIter->first << " = E \"" << str << "\"\n";
			eventValid = ++eventIter != m_events.end();
		}
	}

	outFile << "}\n";
	outFile.flush();
}
