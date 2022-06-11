#pragma once
#include "VocalTrack.h"

template <int numTracks>
inline bool VocalTrack<numTracks>::scan_single(uint32_t position, BCHTraversal& traversal)
{
	// NOTE: Scanning does not take the actual error thrown into account, so there is no need for a try_catch block in this function
	// Errors will be caught by init_single()

	unsigned char lane = traversal.extractChar();
	if (lane > numTracks)
		throw InvalidNoteException(lane);

	if (lane == 0)
	{
		if (m_percussion.empty() || m_percussion.back().first != position)
		{
			// Logic: if no modifier is found OR the modifier can't be applied (the only one being "NoiseOnly"), then it can be played
			unsigned char mod;
			if (!traversal.extract(mod) || (mod & 1) == 0)
				return true;

			if (m_percussion.empty())
				m_percussion.emplace_back(position, VocalPercussion());
			else
				m_percussion.back().first = position;
		}
	}
	else if (m_vocals[lane - 1].empty() || m_vocals[lane - 1].back().first != position)
	{
		unsigned char length = traversal.extractChar();
		traversal.move(length);

		// If a valid pitch AND sustain is found, the scan is a success
		if (unsigned char pitch; traversal.extract(pitch))
		{
			// If no exception is thrown, then the sustain could be pulled
			traversal.extractVarType();
			return true;
		}

		if (m_vocals[lane - 1].empty())
			m_vocals[lane - 1].emplace_back(position, Vocal());
		else
			m_vocals[lane - 1].back().first = position;
	}
	return false;
}

template<int numTracks>
inline void VocalTrack<numTracks>::init_single(uint32_t position, BCHTraversal& traversal)
{
	static Vocal vocalNode;
	static VocalPercussion percNode;

	try
	{
		// Read note
		unsigned char lane = traversal.extractChar();
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
			unsigned char length = traversal.extractChar();
			vocalNode.setLyric(traversal.extractLyric(length));

			// Read pitch
			if (unsigned char pitch; traversal.extract(pitch))
			{
				uint32_t sustain = traversal.extractVarType();
				vocalNode.setPitch(pitch);
				vocalNode.init(sustain);
			}

			m_vocals[lane - 1].emplace_back(traversal.getPosition(), std::move(vocalNode));
		}
	}
	catch (Traversal::NoParseException)
	{
		throw EndofLineException();
	}
}

template<int numTracks>
inline int VocalTrack<numTracks>::scan_bch(BCHTraversal& traversal)
{
	clear();

	uint32_t vocalPhraseEnd = 0;
	while (traversal.next())
	{
		try
		{
			switch (traversal.getEventType())
			{
			case 9:
				// Only scan for valid vocals
				if (traversal.getPosition() >= vocalPhraseEnd)
					continue;

				// So long as the init returns true, it can be concluded that this difficulty does conatin playable notes
				if (scan_single(traversal.getPosition(), traversal))
				{
					// No need to check the rest of the difficulty's data
					traversal.skipTrack();
					return 1;
				}
				break;
			case 5:
			{
				if (traversal.extractChar() == 4 && traversal.getPosition() >= vocalPhraseEnd)
					vocalPhraseEnd = traversal.getPosition() + traversal.extractVarType();
				break;
			}
			}
		}
		catch (std::runtime_error err)
		{
		}
	}

	for (auto& track : m_vocals)
		if ((track.size() < 100 || 2000 <= track.size()) && track.size() < track.capacity())
			track.shrink_to_fit();

	if ((m_percussion.size() < 20 || 400 <= m_percussion.size()) && m_percussion.size() < m_percussion.capacity())
		m_percussion.shrink_to_fit();

	if (traversal.validateChunk("ANIM"))
		traversal.skipTrack();
	return 0;
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
	uint32_t vocalPhraseEnd[2] = { 0, 0 };
	uint32_t starPowerEnd = 0;
	uint32_t soloEnd = 0;
	uint32_t rangeShiftEnd = 0;
	while (traversal.next())
	{
		try
		{
			switch (traversal.getEventType())
			{
			case 9:
				init_single(traversal.getPosition(), traversal);
				break;
			case 3:
				if (m_events.empty() || m_events.back().first < traversal.getPosition())
					m_events.emplace_back(traversal.getPosition(), eventNode);

				m_events.back().second.push_back(traversal.extractText());
				break;
			case 5:
			{
				unsigned char phrase = traversal.extractChar();
				uint32_t duration = traversal.extractVarType();
				auto check = [&](uint32_t& end, const char* noteType)
				{
					// Handles phrase conflicts
					if (traversal.getPosition() < end)
						throw std::string(noteType) + " note conflicts with current active " + noteType + " phrase (ending at tick " + std::to_string(end) + ')';

					if (m_effects.empty() || m_effects.back().first < traversal.getPosition())
						m_effects.emplace_back(traversal.getPosition(), phraseNode);

					end = traversal.getPosition() + duration;
				};

				switch (phrase)
				{
				case 2:
					check(starPowerEnd, "star power");
					m_effects.back().second.push_back(new StarPowerPhrase(duration));
					break;
				case 3:
					check(soloEnd, "solo");
					m_effects.back().second.push_back(new Solo(duration));
					break;
				case 4:
					check(vocalPhraseEnd[0], "vocal phrase");
					m_effects.back().second.push_back(new LyricLine(duration));
					break;
				case 5:
					check(rangeShiftEnd, "range shift");
					m_effects.back().second.push_back(new RangeShift(duration));
					break;
				case 6:
					check(vocalPhraseEnd[1], "harmony phrase");
					m_effects.back().second.push_back(new HarmonyLine(duration));
					break;
				case 64:
				case 65:
				case 66:
					break;
				case 67:
					// No placement check needed as lyric shift is instantaneous
					if (m_effects.empty() || m_effects.back().first < traversal.getPosition())
						m_effects.emplace_back(traversal.getPosition(), phraseNode);

					m_effects.back().second.push_back(new LyricShift());
					break;
				default:
					throw ": unrecognized special phrase type (" + std::to_string(phrase) + ')';
				}
				break;
			}
			default:
				throw std::string(": unrecognized node type(") + (char)traversal.getEventType() + ')';
			}
		}
		catch (std::runtime_error err)
		{
			std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": " << err.what() << std::endl;
		}
		catch (const std::string& str)
		{
			std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": " << str << std::endl;
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
