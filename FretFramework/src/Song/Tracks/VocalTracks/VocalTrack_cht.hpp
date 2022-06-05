#pragma once
#include "VocalTrack.h"

template <int numTracks>
inline void VocalTrack<numTracks>::init_single(uint32_t position, TextTraversal& traversal)
{
	static Vocal vocalNode;
	static VocalPercussion percNode;

	uint32_t lane;
	if (!traversal.extract(lane))
		throw EndofLineException();

	if (lane > numTracks)
		throw InvalidNoteException(lane);

	if (lane == 0)
	{
		if (m_percussion.empty() || m_percussion.back().first != position)
		{
			if (unsigned char mod; traversal.extract(mod))
				percNode.modify(mod);
				
			m_percussion.emplace_back(position, std::move(percNode));
		}
	}
	else if (m_vocals[lane - 1].empty() || m_vocals[lane - 1].back().first != position)
	{
		vocalNode.setLyric(traversal.extractLyric());
		
		// Read pitch if found
		if (uint32_t pitch; traversal.extract(pitch))
		{
			uint32_t sustain;
			if (!traversal.extract(sustain))
				throw EndofLineException();

			vocalNode.setPitch(pitch);
			vocalNode.init(sustain);
		}

		m_vocals[lane - 1].emplace_back(position, std::move(vocalNode));
	}
}

template <int numTracks>
inline void VocalTrack<numTracks>::load_cht(TextTraversal& traversal)
{
	clear();

	for (auto& track : m_vocals)
		track.reserve(1000);
	m_percussion.reserve(200);

	struct
	{
		uint32_t position = 0;
		bool active = false;
	} phrases[2];

	uint32_t prevPosition = 0;
	do
	{
		if (traversal == '}' || traversal == '[')
			break;

		uint32_t position = UINT32_MAX;
		if (traversal.extract(position))
		{
			if (prevPosition <= position)
			{
				traversal.skipEqualsSign();
				char type = traversal.extract();
				switch (type)
				{
				case 'v':
				case 'V':
					try
					{
						init_single(position, traversal);
						prevPosition = position;
					}
					catch (std::runtime_error err)
					{
						std::cout << "Line " << traversal.getLineNumber() << " - Position " << position << ": " << err.what() << std::endl;
					}
					break;
				case 'p':
				case 'P':
				{
					bool phraseStart = true;
					int index = 0;
					if (traversal == 'e' || traversal == 'E')
					{
						phraseStart = false;
						traversal.move(1);
					}

					prevPosition = position;
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
			std::cout << "Line " << traversal.getLineNumber() << ": improper node setup(" << traversal.extractText() << ')' << std::endl;
	} while (traversal.next());

	for (auto& track : m_vocals)
		if ((track.size() < 100 || 2000 <= track.size()) && track.size() < track.capacity())
			track.shrink_to_fit();

	if ((m_percussion.size() < 20 || 400 <= m_percussion.size()) && m_percussion.size() < m_percussion.capacity())
		m_percussion.shrink_to_fit();
}

template <int numTracks>
inline void VocalTrack<numTracks>::save_cht(std::fstream& outFile) const
{
	if (!occupied())
		return;

	std::vector<std::pair<uint32_t, std::vector<std::pair<int, const Vocal*>>>> vocalList;
	{
		static std::vector<std::pair<int, const Vocal*>> node;
		int i = 0;
		while (i < numTracks && m_vocals[i].empty())
			++i;

		if (i < numTracks)
		{
			vocalList.reserve(m_vocals[i].size());
			for (const auto& vocal : m_vocals[i])
			{
				node.push_back({ i + 1, &vocal.second });
				vocalList.push_back({ vocal.first, std::move(node) });
			}

			++i;
			while (i < numTracks)
			{
				for (const auto& vocal : m_vocals[i])
					VectorIteration::try_emplace(vocalList, vocal.first).push_back({ i + 1, &vocal.second });
				++i;
			}
		}
	}

	outFile << m_name << "\n{\n";

	auto vocalIter = vocalList.begin();
	auto percIter = m_percussion.begin();
	auto effectIter = m_effects.begin();
	auto eventIter = m_events.begin();
	bool vocalValid = vocalIter != vocalList.end();
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
			for (const auto& vocal : vocalIter->second)
			{
				std::stringstream buffer;
				vocal.second->save_cht(vocal.first, buffer);
				vocal.second->save_pitch_cht(buffer);
				outFile << '\t' << vocalIter->first << " = V" << buffer.rdbuf() << '\n';
			}
			
			vocalValid = ++vocalIter != vocalList.end();
		}

		while (percValid &&
			(!effectValid || percIter->first < effectIter->first) &&
			(!vocalValid || percIter->first < vocalIter->first) &&
			(!eventValid || percIter->first <= eventIter->first))
		{
			outFile << '\t' << percIter->first << percIter->second.save_cht();
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
