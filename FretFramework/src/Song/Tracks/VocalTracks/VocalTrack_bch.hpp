#pragma once
#include "VocalTrack.h"

template<int numTracks>
inline void VocalTrack<numTracks>::init_single(uint32_t position, BinaryTraversal& traversal)
{
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
			static std::pair<uint32_t, VocalPercussion> pairNode;
			pairNode.first = position;
			m_percussion.push_back(pairNode);

			// Read mod
			if (traversal.extract(lane) && lane & 1)
				m_percussion.back().second.modify('N');
		}
	}
	else
	{
		unsigned char length;
		if (!traversal.extract(length))
			throw EndofEventException();

		if (m_vocals[lane - 1].empty() || m_vocals[lane - 1].back().first != position)
		{
			static std::pair<uint32_t, Vocal> pairNode;
			pairNode.first = position;
			m_vocals[lane - 1].push_back(pairNode);
		}

		m_vocals[lane - 1].back().second.setLyric(traversal.extractText(length));

		// Read pitch
		unsigned char pitch;
		if (traversal.extract(pitch))
		{
			uint32_t sustain;
			if (!traversal.extract(sustain))
				throw EndofEventException();

			m_vocals[lane - 1].back().second.setPitch(pitch);
			m_vocals[lane - 1].back().second.init(sustain);
		}
	}
}

template<int numTracks>
inline void VocalTrack<numTracks>::init_chord(uint32_t position, BinaryTraversal& traversal)
{
	unsigned char colors;
	if (!traversal.extract(colors))
		throw EndofEventException();

	for (char i = 0; i < colors; ++i)
	{
		unsigned char lane;
		if (!traversal.extract(lane))
			throw EndofEventException();

		if (lane == 0 || lane > numTracks)
			throw InvalidNoteException(lane);

		if (!m_percussion.empty() && m_percussion.back().first == position)
			m_percussion.pop_back();

		unsigned char length;
		if (!traversal.extract(length))
			throw EndofEventException();

		if (m_vocals[lane - 1].empty() || m_vocals[lane - 1].back().first != position)
		{
			static std::pair<uint32_t, Vocal> pairNode;
			pairNode.first = position;
			m_vocals[lane - 1].push_back(pairNode);
			m_vocals[lane - 1].back().second.setLyric(traversal.extractText(length));
		}
		else
			traversal.move(length);
	}
}

template<int numTracks>
inline void VocalTrack<numTracks>::modify(uint32_t position, BinaryTraversal& traversal)
{
	unsigned char numMods;
	if (traversal.extract(numMods))
	{
		unsigned char modifier;
		for (char i = 0; i < numMods && traversal.extract(modifier); ++i)
			if (modifier & 1 && !m_percussion.empty() && m_percussion.back().first == position)
				m_percussion.back().second.modify('N');
	}
}

template<int numTracks>
inline void VocalTrack<numTracks>::vocalize_bch(uint32_t position, BinaryTraversal& traversal)
{
	int numVocalized = 0;
	unsigned char numPitches;
	if (traversal.extract(numPitches))
	{
		unsigned char lane;
		unsigned char pitch;
		uint32_t sustain;
		for (char i = 0;
			i < numPitches && 
			traversal.extract(lane) && 0 < lane && lane <= numTracks &&
			traversal.extract(pitch) &&
			traversal.extract(sustain); ++i)
		{
			if (!m_vocals[lane - 1].empty() && m_vocals[lane - 1].back().first == position)
			{
				m_vocals[lane - 1].back().second.setPitch(pitch);
				m_vocals[lane - 1].back().second.init(sustain);
				++numVocalized;
			}
		}
	}

	if (numVocalized == 0)
		std::cout << "Unable to vocalize lyrics at position " << std::endl;
}

template<int numTracks>
inline void VocalTrack<numTracks>::load_bch(BinaryTraversal& traversal)
{
	clear();
	try
	{
		traversal.move(4);
		uint32_t size;
		for (auto& track : m_vocals)
		{
			size = traversal;
			track.reserve(size);
		}
		size = traversal;
		m_percussion.reserve(size);
		size = traversal;
		m_effects.reserve(size);
		size = traversal;
		m_events.reserve(size);
	}
	// The only error that should be caught signals to start parsing events
	catch (...) {}

	while (traversal.next())
	{
		switch (traversal.getEventType())
		{
		case 3:
		{
			if (m_events.empty() || m_events.back().first < traversal.getPosition())
			{
				static std::pair<uint32_t, std::vector<std::string>> pairNode;
				pairNode.first = traversal.getPosition();
				m_events.push_back(pairNode);
			}

			m_events.back().second.push_back(traversal.extractText());
			break;
		}
		case 9:
		case 10:
			try
			{
				if (traversal.getEventType() == 9)
					init_single(traversal.getPosition(), traversal);
				else
					init_chord(traversal.getPosition(), traversal);
			}
			catch (std::runtime_error err)
			{
				std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": " << err.what() << std::endl;
				for (auto& track : m_vocals)
					if (!track.empty() && track.back().first == traversal.getPosition())
						track.pop_back();
			}
			break;
		case 11:
			try
			{
				vocalize_bch(traversal.getPosition(), traversal);
			}
			catch (std::runtime_error err)
			{
				std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": unable to parse full list of pitches" << std::endl;
			}
			break;
		case 8:
			modify(traversal.getPosition(), traversal);
			break;
		case 5:
		{
			unsigned char phrase = traversal.extract();
			uint32_t duration = 0;
			auto check = [&]()
			{
				traversal.extractVarType(duration);
				if (m_effects.empty() || m_effects.back().first < traversal.getPosition())
				{
					static std::pair<uint32_t, std::vector<Phrase*>> pairNode;
					pairNode.first = traversal.getPosition();
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
				std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": unrecognized special phrase type (" << phrase << ')' << std::endl;
			}
			break;
		}
		default:
			std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": unrecognized node type(" << traversal.getEventType() << ')' << std::endl;
		}
	}

	for (auto& track : m_vocals)
		if (track.size() < track.capacity())
			track.shrink_to_fit();

	if (m_percussion.size() < m_percussion.capacity())
		m_percussion.shrink_to_fit();

	if (traversal.validateChunk("ANIM"))
		traversal.skipTrack();
}

#include "VocalGroup.h"

template <int numTracks>
inline bool VocalTrack<numTracks>::save_bch(std::fstream& outFile) const
{
	if (!occupied())
		return false;

	std::vector<std::pair<uint32_t, VocalGroup<numTracks>>> vocalGroups;
	{
		int i = 0;
		while (i < numTracks && m_vocals[i].empty())
			++i;

		if (i < numTracks)
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

			while (i < numTracks)
			{
				for (const auto& vocal : m_vocals[i])
					VectorIteration::try_emplace(vocalGroups, vocal.first).m_vocals[i] = &vocal.second;
				++i;
			}
		}
	}

	outFile.write("INST", 4);
	uint32_t length = 0;
	auto start = outFile.tellp();
	outFile.write((char*)&length, 4);
	outFile.put(m_instrumentID);

	const uint32_t headerLength = 16 + 4 * numTracks;
	uint32_t numEvents = 0;
	const uint32_t numPercussion = (uint32_t)m_percussion.size();
	const uint32_t numPhrases = (uint32_t)m_effects.size();
	const uint32_t numTextEvents = (uint32_t)m_events.size();

	outFile.write((char*)&headerLength, 4);
	outFile.write((char*)&numEvents, 4);
	for (const auto& track : m_vocals)
	{
		uint32_t numNotes = (uint32_t)track.size();
		outFile.write((char*)&numNotes, 4);
	}
	outFile.write((char*)&numPercussion, 4);
	outFile.write((char*)&numPhrases, 4);
	outFile.write((char*)&numTextEvents, 4);

	auto vocalIter = vocalGroups.begin();
	auto percIter = m_percussion.begin();
	auto effectIter = m_effects.begin();
	auto eventIter = m_events.begin();
	bool vocalValid = vocalIter != vocalGroups.end();
	auto percValid = percIter != m_percussion.end();
	bool effectValid = effectIter != m_effects.end();
	bool eventValid = eventIter != m_events.end();

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
			numEvents += vocalIter->second.save_bch(vocalIter->first - prevPosition, outFile);
			prevPosition = vocalIter->first;
			vocalValid = ++vocalIter != vocalGroups.end();
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
	length = uint32_t(end - start) - (headerLength + 4);
	outFile.seekp(start);
	outFile.write((char*)&length, 4);
	outFile.put(m_instrumentID);
	outFile.write((char*)&headerLength, 4);
	outFile.write((char*)&numEvents, 4);
	outFile.seekp(end);
	return true;
}
