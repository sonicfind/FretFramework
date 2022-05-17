#pragma once
#include "VocalTrack.h"

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
