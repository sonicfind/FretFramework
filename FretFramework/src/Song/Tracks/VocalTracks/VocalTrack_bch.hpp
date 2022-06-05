#pragma once
#include "VocalTrack.h"

template<int numTracks>
inline int VocalTrack<numTracks>::init_single(uint32_t position, BCHTraversal& traversal)
{
	static Vocal vocalNode;
	static VocalPercussion percNode;

	// Read note
	unsigned char lane;
	if (!traversal.extract(lane))
		throw EndofEventException();

	if (lane > numTracks)
		throw InvalidNoteException(lane);

	if (lane == 0)
	{
		if (m_percussion.empty() || m_percussion.back().first != position)
		{
			// Read mod
			if (traversal.extract(lane) && lane & 1)
				percNode.modify('N');

			m_percussion.emplace_back(traversal.getPosition(), std::move(percNode));
		}
	}
	else if (m_vocals[lane - 1].empty() || m_vocals[lane - 1].back().first != position)
	{
		unsigned char length;
		if (!traversal.extract(length))
			throw EndofEventException();

		vocalNode.setLyric(traversal.extractLyric(length));
		
		// Read pitch
		if (unsigned char pitch; traversal.extract(pitch))
		{
			uint32_t sustain;
			if (!traversal.extractVarType(sustain))
				throw EndofEventException();

			vocalNode.setPitch(pitch);
			vocalNode.init(sustain);
		}

		m_vocals[lane - 1].emplace_back(traversal.getPosition(), std::move(vocalNode));
	}
	return lane;
}

template<int numTracks>
inline void VocalTrack<numTracks>::load_bch(BCHTraversal& traversal)
{
	clear();

	for (auto& track : m_vocals)
		track.reserve(1000);
	m_percussion.reserve(200);

	const static std::vector<std::string> eventNode;
	const static std::vector<Phrase*> phraseNode;
	while (traversal.next())
	{
		switch (traversal.getEventType())
		{
		case 9:
			try
			{
				init_single(traversal.getPosition(), traversal);
			}
			catch (std::runtime_error err)
			{
				std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": " << err.what() << std::endl;
			}
			break;
		case 3:
			if (m_events.empty() || m_events.back().first < traversal.getPosition())
				m_events.emplace_back(traversal.getPosition(), eventNode);

			m_events.back().second.push_back(traversal.extractText());
			break;
		case 5:
		{
			unsigned char phrase = traversal.extract();
			uint32_t duration = 0;
			auto check = [&]()
			{
				traversal.extractVarType(duration);
				if (m_effects.empty() || m_effects.back().first < traversal.getPosition())
					m_effects.emplace_back(traversal.getPosition(), phraseNode);
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
				std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": unrecognized special phrase type (" << phrase << ')' << std::endl;
			}
			break;
		}
		default:
			std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": unrecognized node type(" << traversal.getEventType() << ')' << std::endl;
		}
	}

	for (auto& track : m_vocals)
		if ((track.size() < 100 || 2000 <= track.size()) && track.size() < track.capacity())
			track.shrink_to_fit();

	if ((m_percussion.size() < 20 || 400 <= m_percussion.size()) && m_percussion.size() < m_percussion.capacity())
		m_percussion.shrink_to_fit();

	if (traversal.validateChunk("ANIM"))
		traversal.skipTrack();
}

template <int numTracks>
inline bool VocalTrack<numTracks>::save_bch(std::fstream& outFile) const
{
	if (!occupied())
		return false;

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

	outFile.write("VOCL", 4);

	auto start = outFile.tellp();
	uint32_t length = 0;
	outFile.write((char*)&length, 4);
	outFile.put(m_instrumentID);

	uint32_t numEvents = 0;
	unsigned char isPlayed = m_percussion.size() ? 1 : 0;
	outFile.put(isPlayed);
	outFile.write((char*)&numEvents, 4);

	auto vocalIter = vocalList.begin();
	auto percIter = m_percussion.begin();
	auto effectIter = m_effects.begin();
	auto eventIter = m_events.begin();
	bool vocalValid = vocalIter != vocalList.end();
	bool percValid = percIter != m_percussion.end();
	bool effectValid = effectIter != m_effects.end();
	bool eventValid = eventIter != m_events.end();

	static char buffer[263];

	uint32_t prevPosition = 0;
	while (effectValid || vocalValid || eventValid)
	{
		while (effectValid &&
			(!vocalValid || effectIter->first <= vocalIter->first) &&
			(!percValid || effectIter->first <= percIter->first) &&
			(!eventValid || effectIter->first <= eventIter->first))
		{
			WebType delta(effectIter->first - prevPosition);
			for (const auto& eff : effectIter->second)
			{
				delta.writeToFile(outFile);
				eff->save_bch(outFile);
				delta = 0;
			}
			numEvents += (uint32_t)effectIter->second.size();
			prevPosition = effectIter->first;
			effectValid = ++effectIter != m_effects.end();
		}

		while (vocalValid &&
			(!effectValid || vocalIter->first < effectIter->first) &&
			(!percValid || vocalIter->first <= percIter->first) &&
			(!eventValid || vocalIter->first <= eventIter->first))
		{
			WebType delta(vocalIter->first - prevPosition);
			for (const auto& vocal : vocalIter->second)
			{
				delta.writeToFile(outFile);
				delta = 0;

				char* current = buffer;
				vocal.second->save_bch(vocal.first, current);
				vocal.second->save_pitch_bch(current);

				const uint32_t length(uint32_t(current - buffer));
				// Event type - Single (Lyric)
				outFile.put(9);
				WebType::writeToFile(length, outFile);
				outFile.write(buffer, length);

				if (vocal.second->m_isPitched)
					isPlayed = 1;
				++numEvents;
			}

			prevPosition = vocalIter->first;
			vocalValid = ++vocalIter != vocalList.end();
		}

		while (percValid &&
			(!effectValid || percIter->first < effectIter->first) &&
			(!vocalValid || percIter->first < vocalIter->first) &&
			(!eventValid || percIter->first <= eventIter->first))
		{
			WebType(eventIter->first - prevPosition).writeToFile(outFile);
			percIter->second.save_bch(outFile);
			prevPosition = percIter->first;
			percValid = ++percIter != m_percussion.end();
			++numEvents;
		}

		while (eventValid &&
			(!effectValid || eventIter->first < effectIter->first) &&
			(!percValid || eventIter->first < percIter->first) &&
			(!vocalValid || eventIter->first < vocalIter->first))
		{
			WebType delta(eventIter->first - prevPosition);
			for (const auto& str : eventIter->second)
			{
				delta.writeToFile(outFile);
				outFile.put(3);
				WebType length((uint32_t)str.length());
				length.writeToFile(outFile);
				outFile.write(str.data(), length);
				delta = 0;
			}
			numEvents += (uint32_t)eventIter->second.size();
			prevPosition = eventIter->first;
			eventValid = ++eventIter != m_events.end();
		}
	}

	const auto end = outFile.tellp();
	length = uint32_t(end - start) - 4;
	outFile.seekp(start);
	outFile.write((char*)&length, 4);
	outFile.put(m_instrumentID);
	outFile.put(isPlayed);
	outFile.write((char*)&numEvents, 4);
	outFile.seekp(end);
	return true;
}
