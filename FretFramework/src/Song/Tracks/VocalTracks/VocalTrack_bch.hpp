#pragma once
#include "VocalTrack.h"

template<int numTracks>
inline void VocalTrack<numTracks>::init_bch_single(uint32_t position, const unsigned char* current, const unsigned char* const end)
{
	char lane = *current++;
	if (lane > numTracks)
		throw InvalidNoteException(lane);

	if (lane == 0)
	{
		if (m_percussion.empty() || m_percussion.back().first != position)
		{
			static std::pair<uint32_t, VocalPercussion> pairNode;
			pairNode.first = position;
			m_percussion.push_back(pairNode);

			if (current < end)
				if (*current++ & 1)
					m_percussion.back().second.modify('N');
		}
	}
	else
	{
		if (current + 1 > end)
			throw EndofEventException();

		if (m_vocals[lane - 1].empty() || m_vocals[lane - 1].back().first != position)
		{
			static std::pair<uint32_t, Vocal> pairNode;
			pairNode.first = position;
			m_vocals[lane - 1].push_back(pairNode);
		}

		unsigned char length = (unsigned char)*current++;
		if (current + length > end)
			length = (unsigned char)(end - current);
		m_vocals[lane - 1].back().second.setLyric(std::string((const char*)current, length));
		current += length;

		// Read pitch
		if (current < end)
		{
			char pitch = *current++;
			m_vocals[lane - 1].back().second.setPitch(pitch);
			m_vocals[lane - 1].back().second.init(VariableLengthQuantity(current));
		}
	}
}

template<int numTracks>
inline void VocalTrack<numTracks>::init_bch_chord(uint32_t position, const unsigned char* current, const unsigned char* const end)
{
	char colors = *current++;
	int numAdded = 0;
	for (char i = 0; current < end && i < colors; ++i)
	{
		char lane = *current++;
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

			if (current + 1 > end)
				throw EndofEventException();

			unsigned char length = (unsigned char)*current++;
			if (current + length > end)
				length = (unsigned char)(end - current);

			m_vocals[lane - 1].back().second.setLyric(std::string((const char*)current, length));
			current += length;
			++numAdded;
		}
	}

	if (numAdded == 0)
		throw InvalidNoteException();
}

template<int numTracks>
inline void VocalTrack<numTracks>::modify_bch(uint32_t position, const unsigned char* current, const unsigned char* const end)
{
	unsigned char numMods = *current++;
	for (unsigned char i = 0; current < end && i < numMods; ++i)
	{
		char mod = *current++;
		if (mod & 1)
		{
			if (!m_percussion.empty() && m_percussion.back().first == position)
				m_percussion.back().second.modify('N');
		}
	}
}

template<int numTracks>
inline void VocalTrack<numTracks>::vocalize_bch(uint32_t position, const unsigned char* current, const unsigned char* const end)
{
	char numPitches = *current++;
	int numVocalized = 0;
	for (char i = 0; i < numPitches && current + 6 <= end; ++i)
	{
		char lane = *current++;
		char pitch = *current++;

		if (0 < lane && lane <= numTracks &&
			!m_vocals[lane - 1].empty() && m_vocals[lane - 1].back().first == position)
		{
			m_vocals[lane - 1].back().second.setPitch(pitch);
			m_vocals[lane - 1].back().second.init(VariableLengthQuantity(current));
			++numVocalized;
		}
		else
			VariableLengthQuantity::discard(current);
	}

	if (numVocalized == 0)
		std::cout << "Unable to vocalize lyrics at position " << std::endl;
}

template<int numTracks>
inline void VocalTrack<numTracks>::load_bch(const unsigned char* current, const unsigned char* const end)
{
	clear();
	{
		uint32_t headerSize = *(uint32_t*)current;
		current += 4;
		const unsigned char* const start = current + headerSize;

		if (current == start)
			goto StartNoteRead;
		current += 4;

		if (current == start)
			goto StartNoteRead;

		uint32_t size = *(uint32_t*)current;
		current += 4;
		m_vocals[0].reserve(size);

		if (numTracks == 3)
		{
			if (current == start)
				goto StartNoteRead;

			size = *(uint32_t*)current;
			current += 4;
			m_vocals[1].reserve(size);

			if (current == start)
				goto StartNoteRead;

			size = *(uint32_t*)current;
			current += 4;
			m_vocals[2].reserve(size);
		}

		if (current == start)
			goto StartNoteRead;

		size = *(uint32_t*)current;
		current += 4;
		m_percussion.reserve(size);

		if (current == start)
			goto StartNoteRead;

		size = *(uint32_t*)current;
		current += 4;
		m_effects.reserve(size);

		if (current == start)
			goto StartNoteRead;

		size = *(uint32_t*)current;
		current += 4;
		m_events.reserve(size);
	}

StartNoteRead:
	uint32_t position = 0;
	int eventCount = 0;
	while (current < end)
	{
		position += VariableLengthQuantity(current);
		char type = *current++;
		VariableLengthQuantity length(current);

		const unsigned char* const next = current + length;
		if (next > end)
			break;

		switch (type)
		{
		case 3:
		{
			if (m_events.empty() || m_events.back().first < position)
			{
				static std::pair<uint32_t, std::vector<std::string>> pairNode;
				pairNode.first = position;
				m_events.push_back(pairNode);
			}

			m_events.back().second.push_back(std::string((const char*)current, length));
			break;
		}
		case 9:
		case 10:
			try
			{
				if (type == 9)
					init_bch_single(position, current, next);
				else
					init_bch_chord(position, current, next);
			}
			catch (std::runtime_error err)
			{
				std::cout << "Event #" << eventCount << " - Position " << position << ": " << err.what() << std::endl;
				for (auto& track : m_vocals)
					if (!track.empty() && track.back().first == position)
						track.pop_back();
			}
			break;
		case 11:
			try
			{
				vocalize_bch(position, current, next);
			}
			catch (std::runtime_error err)
			{
				std::cout << "Event #" << eventCount << " - Position " << position << ": unable to parse full list of pitches" << std::endl;
			}
			break;
		case 8:
			modify_bch(position, current, next);
			break;
		case 5:
		{
			char phrase = *current++;
			uint32_t duration = 0;
			auto check = [&]()
			{
				if (current + 4 > end)
					throw "You dun goofed";
				memcpy(&duration, current, 4);
				current += 4;

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
				std::cout << "Event #" << eventCount << " - Position " << position << ": unrecognized special phrase type (" << phrase << ')' << std::endl;
			}
			break;
		}
		default:
			std::cout << "Event #" << eventCount << " - Position " << position << ": unrecognized node type(" << type << ')' << std::endl;
		}
		++eventCount;
		current = next;
	}

	for (auto& track : m_vocals)
		if (track.size() < track.capacity())
			track.shrink_to_fit();

	if (m_percussion.size() < m_percussion.capacity())
		m_percussion.shrink_to_fit();

	if (current + 8 < end)
	{
		static struct {
			char tag[4] = {};
			uint32_t length = 0;
		} chunk;

		memcpy(&chunk, current, sizeof(chunk));
		if (strncmp(chunk.tag, "ANIM", 4) == 0)
		{
			current += sizeof(chunk);
			const unsigned char* next = current + chunk.length;

			if (next > end)
				next = end;

			// Insert implementation here

			current = next;
		}
	}
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
			VariableLengthQuantity position(effectIter->first - prevPosition);
			for (const auto& eff : effectIter->second)
			{
				position.writeToFile(outFile);
				eff->save_bch(outFile);
				position = 0;
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
			VariableLengthQuantity(eventIter->first - prevPosition).writeToFile(outFile);
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
			VariableLengthQuantity position(eventIter->first - prevPosition);
			for (const auto& str : eventIter->second)
			{
				position.writeToFile(outFile);
				outFile.put(3);
				VariableLengthQuantity length((uint32_t)str.length());
				length.writeToFile(outFile);
				outFile.write(str.data(), length);
				position = 0;
			}
			numEvents += (uint32_t)eventIter->second.size();
			prevPosition = eventIter->first;
			eventValid = ++eventIter != m_events.end();
		}
	}

	const auto end = outFile.tellp();
	length = uint32_t(end - start - 4);
	outFile.seekp(start);
	outFile.write((char*)&length, 4);
	outFile.put(m_instrumentID);
	outFile.write((char*)&headerLength, 4);
	outFile.write((char*)&numEvents, 4);
	outFile.seekp(end);
	return true;
}
