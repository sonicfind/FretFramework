#pragma once
#include "VocalTrack.h"

template <size_t numTracks>
void VocalTrack<numTracks>::init_cht_single(uint32_t position, TextTraversal& traversal)
{
	uint32_t lane;
	size_t count = traversal.extract(lane);
	if (!count)
		throw EndofLineException();

	if (lane > numTracks)
		throw InvalidNoteException(lane);

	traversal.move(count);
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
		if (count = traversal.extract(pitch))
		{
			traversal.move(count);
			uint32_t sustain;
			if (traversal.extract(sustain))
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

template <size_t numTracks>
void VocalTrack<numTracks>::init_cht_chord(uint32_t position, TextTraversal& traversal)
{
	uint32_t colors;
	if (size_t count = traversal.extract(colors))
	{
		traversal.move(count);
		int numAdded = 0;
		uint32_t lane;
		for (uint32_t i = 0; i < colors; ++i)
		{
			if (!(count = traversal.extract(lane)))
				throw EndofLineException();

			if (lane > numTracks)
				throw InvalidNoteException(lane);

			traversal.move(count);
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

template <size_t numTracks>
void VocalTrack<numTracks>::modify_cht(uint32_t position, TextTraversal& traversal)
{
	uint32_t numMods;
	if (size_t count = traversal.extract(numMods))
	{
		traversal.move(count);
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

template <size_t numTracks>
void VocalTrack<numTracks>::vocalize_cht(uint32_t position, TextTraversal& traversal)
{
	uint32_t numPitches;
	if (size_t count = traversal.extract(numPitches))
	{
		traversal.move(count);

		int numVocalized = 0;
		for (uint32_t i = 0; i < numPitches; ++i)
		{
			uint32_t lane;
			if (!(count = traversal.extract(lane)))
				throw EndofLineException();
			traversal.move(count);

			uint32_t pitch;
			if (!(count = traversal.extract(pitch)))
				throw EndofLineException();

			uint32_t sustain;
			if (!(count = traversal.extract(sustain)))
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

template <size_t numTracks>
void VocalTrack<numTracks>::load_cht(TextTraversal& traversal)
{
	clear();

	if (numTracks == 1)
	{
		if (strncmp(traversal.getCurrent(), "Lyrics", 6) == 0)
		{
			traversal.move(6);
			traversal.skipEqualsSign();

			uint32_t numNotes;
			traversal.extract(numNotes);
			m_vocals[0].reserve(numNotes);
			traversal.nextLine();
		}
	}
	else if (numTracks > 1)
	{
		if (strncmp(traversal.getCurrent(), "Harm1", 5) == 0)
		{
			traversal.move(5);
			traversal.skipEqualsSign();

			uint32_t numNotes;
			traversal.extract(numNotes);
			m_vocals[0].reserve(numNotes);
			traversal.nextLine();
		}

		if (strncmp(traversal.getCurrent(), "Harm2", 5) == 0)
		{
			traversal.move(5);
			traversal.skipEqualsSign();

			uint32_t numNotes;
			traversal.extract(numNotes);
			m_vocals[1].reserve(numNotes);
			traversal.nextLine();
		}

		if (numTracks > 2)
		{
			if (strncmp(traversal.getCurrent(), "Harm3", 5) == 0)
			{
				traversal.move(5);
				traversal.skipEqualsSign();

				uint32_t numNotes;
				traversal.extract(numNotes);
				m_vocals[2].reserve(numNotes);
				traversal.nextLine();
			}
		}
	}

	if (strncmp(traversal.getCurrent(), "Percussion", 10) == 0)
	{
		traversal.move(6);
		traversal.skipEqualsSign();

		uint32_t numNotes;
		traversal.extract(numNotes);
		m_percussion.reserve(numNotes);
		traversal.nextLine();
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
		if (size_t count = traversal.extract(position))
		{
			if (prevPosition <= position)
			{
				traversal.move(count);
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
					long phrase;
					if (count = traversal.extract(phrase))
					{
						uint32_t duration = 0;
						auto check = [&]()
						{
							prevPosition = position;
							traversal.move(count);
							traversal.extract(duration);
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

	for (auto& track : m_vocals)
		if (track.size() < track.capacity())
			track.shrink_to_fit();

	if (m_percussion.size() < m_percussion.capacity())
		m_percussion.shrink_to_fit();
}
