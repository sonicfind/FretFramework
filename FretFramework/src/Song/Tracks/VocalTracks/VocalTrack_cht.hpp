#pragma once
#include "VocalTrack.h"
#include "NoteExceptions.h"
#include <iostream>

template<int numTracks>
inline void VocalTrack_Scan<numTracks>::scan_cht(TextTraversal& traversal)
{
	bool checked[numTracks]{};
	bool lyricsExist = false;
	const int finalValue = (1 << numTracks) - 1;

	uint32_t phraseEnd = 0;
	uint32_t starPowerEnd = 0;
	uint32_t soloEnd = 0;
	uint32_t rangeShiftEnd = 0;

	traversal.resetPosition();
	do
	{
		if (traversal == '}' || traversal == '[')
			break;

		try
		{
			uint32_t position = traversal.extractPosition();
			char type = traversal.extract<unsigned char>();

			// Special Phrases & Text Events are only important for validating proper event order in regards to tick position
			switch (type)
			{
			case 'V':
			case 'v':
			{
				uint32_t lane = traversal.extract<uint32_t>();

				// Only scan for valid vocals
				if (lane == 0 || lane > numTracks || position >= phraseEnd)
					continue;

				--lane;
				const int val = 1 << lane;
				if (!checked[lane] && traversal.skipLyric())
				{
					lyricsExist = true;

					// Pitch AND sustain required
					if (traversal.skipInt() && traversal.skipInt())
					{
						m_scanValue |= val;
						checked[lane] = true;

						if ((m_scanValue & finalValue) == finalValue)
							traversal.skipTrack();
					}
				}
				break;
			}
			case 'P':
			case 'p':
			{
				if (position < phraseEnd && phraseEnd != UINT32_MAX)
					break;

				bool isPhraseStarting = true;
				if (traversal == 'e' || traversal == 'E')
				{
					isPhraseStarting = false;
					traversal.move(1);
				}

				if constexpr (numTracks > 1)
				{
					if (traversal == 'h' || traversal == 'H')
						break;
				}

				phraseEnd = isPhraseStarting ? UINT32_MAX : 0;
				break;
			}
			case 'S':
			case 's':
				if (traversal.extract<uint32_t>() == 4 && position >= phraseEnd)
					phraseEnd = position + traversal.extract<uint32_t>();
			}
		}
		catch (...)
		{

		}
	} while (traversal.next());
}

template<int numTracks>
inline void VocalTrack<numTracks>::scan_cht(TextTraversal& traversal, std::unique_ptr<NoteTrack_Scan>& track) const
{
	if (track == nullptr)
		track = std::make_unique<VocalTrack_Scan<numTracks>>();
	track->scan_cht(traversal);
}

template <int numTracks>
inline void VocalTrack<numTracks>::load_cht(TextTraversal& traversal)
{
	clear();
	for (auto& track : m_vocals)
		track.reserve(1000);
	m_percussion.reserve(200);

#ifndef _DEBUG
	static constexpr std::vector<std::u32string> eventNode;
	static const Vocal vocalNode;
	static constexpr std::vector<Phrase*> phraseNode;
	static constexpr VocalPercussion percNode;
#else
	static const std::vector<std::u32string> eventNode;
	static const Vocal vocalNode;
	static const std::vector<Phrase*> phraseNode;
	static constexpr VocalPercussion percNode;
#endif // !_DEBUG

	// End positions to protect from conflicting special phrases
	struct
	{
		uint32_t position = 0;
		uint32_t end = 0;
	} vocalPhrases[2];
	uint32_t starPowerEnd = 0;
	uint32_t soloEnd = 0;
	uint32_t rangeShiftEnd = 0;

	traversal.resetPosition();
	do
	{
		if (traversal == '}' || traversal == '[')
			break;

		try
		{
			uint32_t position = traversal.extractPosition();
			char type = traversal.extract<unsigned char>();

			switch (type)
			{
			case 'V':
			case 'v':
			{
				uint32_t lane = traversal.extract<uint32_t>();
				if (lane > numTracks)
					throw InvalidNoteException(lane);

				if (lane == 0)
				{
					if (m_percussion.empty() || m_percussion.back().first != position)
						m_percussion.emplace_back(position, std::move(percNode));

					if (unsigned char mod; traversal.extract(mod))
						m_percussion.back().second.modify(mod);
				}
				else
				{
					--lane;
					if (m_vocals[lane].empty() || m_vocals[lane].back().first != position)
						m_vocals[lane].emplace_back(position, vocalNode);

					m_vocals[lane].back().second.init(traversal);
				}
				break;
			}
			case 'P':
			case 'p':
			{
				bool phraseStart = true;
				int index = 0;
				if (traversal == 'e' || traversal == 'E')
				{
					phraseStart = false;
					traversal.move(1);
				}

				if (numTracks > 1)
				{
					if (traversal == 'h' || traversal == 'H')
						index = 1;
				}

				// Only allow one lyric line type at a time
				// Handles phrase conflicts from using both the lyric line special phrase and Phrase events in a single chart
				if (position < vocalPhrases[index].end && vocalPhrases[index].end != UINT32_MAX)
				{
					if (index == 0)
						throw "Position " + std::to_string(position) + ": vocal phrase event conflicts with currently active vocal phrase note (ending at tick " + std::to_string(vocalPhrases[0].end) + ')';
					else
						throw "Position " + std::to_string(position) + ": harmony phrase event conflicts with currently active harmony phrase note (ending at tick " + std::to_string(vocalPhrases[1].end) + ')';
				}

				if (phraseStart)
				{
					if (vocalPhrases[index].end == UINT32_MAX)
					{
						if (index == 0)
							addPhrase(vocalPhrases[0].position, new LyricLine(position - vocalPhrases[0].position));
						else
							addPhrase(vocalPhrases[1].position, new HarmonyLine(position - vocalPhrases[1].position));
					}
					vocalPhrases[index].position = position;
					vocalPhrases[index].end = UINT32_MAX;
				}
				else
				{
					if (index == 0)
						addPhrase(vocalPhrases[0].position, new LyricLine(position - vocalPhrases[0].position));
					else
						addPhrase(vocalPhrases[1].position, new HarmonyLine(position - vocalPhrases[1].position));
					vocalPhrases[index].end = 0;
				}
				break;
			}
			case 'E':
			case 'e':
			{
				if (m_events.empty() || m_events.back().first < position)
					m_events.emplace_back(position, eventNode);

				m_events.back().second.push_back(traversal.extractText());
				break;
			}
			case 'S':
			case 's':
			{
				uint32_t phrase = traversal.extract<uint32_t>();
				uint32_t duration = 0;
				auto check = [&](uint32_t& end, const char* noteType)
				{
					// Handles phrase conflicts
					if (position < end)
						throw "Position " + std::to_string(position) + ": " + noteType + " note conflicts with current active " + noteType + " phrase (ending at tick " + std::to_string(end) + ')';

					if (m_effects.empty() || m_effects.back().first < position)
						m_effects.emplace_back(position, phraseNode);

					traversal.extract(duration);
					end = position + duration;
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
					check(vocalPhrases[0].end, "vocal phrase");
					m_effects.back().second.push_back(new LyricLine(duration));
					break;
				case 5:
					check(rangeShiftEnd, "range shift");
					m_effects.back().second.push_back(new RangeShift(duration));
					break;
				case 6:
					check(vocalPhrases[1].end, "harmony phrase");
					m_effects.back().second.push_back(new HarmonyLine(duration));
					break;
				case 64:
				case 65:
				case 66:
					break;
				case 67:
					// No placement check needed as lyric shift is instantaneous
					if (m_effects.empty() || m_effects.back().first < position)
						m_effects.emplace_back(position, phraseNode);

					m_effects.back().second.push_back(new LyricShift());
					break;
				default:
					throw "Position " + std::to_string(position) + ": unrecognized special phrase type (" + std::to_string(phrase) + ')';
				}
				break;
			}
			default:
				throw "Position " + std::to_string(position) + ": unrecognized node type(" + type + ')';
			}
		}
		catch (std::runtime_error err)
		{
			std::cout << "Line " << traversal.getLineNumber() << " - " << err.what() << std::endl;
		}
		catch (const std::string& str)
		{
			std::cout << "Line " << traversal.getLineNumber() << " - " << str << std::endl;
		}
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

	std::vector<std::pair<uint32_t, Vocal>>::const_iterator vocalIters[numTracks];
	bool vocalValidations[numTracks] = {};
	for (int i = 0; i < numTracks; ++i)
	{
		vocalIters[i] = m_vocals[i].begin();
		vocalValidations[i] = !m_vocals[i].empty();
	}

	auto checkVocals = [&]()
	{
		for (const bool valid : vocalValidations)
			if (valid)
				return true;
		return false;
	};

	auto comparePosition_pre = [&](uint32_t position)
	{
		for (int i = 0; i < numTracks; ++i)
			if (vocalValidations[i] && vocalIters[i]->first < position)
				return false;
		return true;
	};

	auto comparePosition_post = [&](uint32_t position)
	{
		for (int i = 0; i < numTracks; ++i)
			if (vocalValidations[i] && vocalIters[i]->first <= position)
				return false;
		return true;
	};

	auto percIter = m_percussion.begin();
	auto effectIter = m_effects.begin();
	auto eventIter = m_events.begin();
	bool percValid = percIter != m_percussion.end();
	bool effectValid = effectIter != m_effects.end();
	bool eventValid = eventIter != m_events.end();

	auto checkVocal = [&](size_t index)
	{
		if (!vocalValidations[index])
			return false;

		const uint32_t position = vocalIters[index]->first;

		if (effectValid && effectIter->first <= position)
			return false;

		for (size_t i = 0; i < index; ++i)
			if (vocalValidations[i] && vocalIters[i]->first <= position)
				return false;

		for (size_t i = index + 1; i < numTracks; ++i)
			if (vocalValidations[i] && vocalIters[i]->first < position)
				return false;

		return (!percValid || position <= percIter->first) && (!eventValid || position <= eventIter->first);
	};

	outFile << m_name << "\n{\n";
	while (effectValid || checkVocals() || eventValid)
	{
		while (effectValid &&
			comparePosition_pre(effectIter->first) &&
			(!percValid || effectIter->first <= percIter->first) &&
			(!eventValid || effectIter->first <= eventIter->first))
		{
			for (const auto& eff : effectIter->second)
				eff->save_cht(effectIter->first, outFile, "\t");
			effectValid = ++effectIter != m_effects.end();
		}

		for (int i = 0; i < numTracks; ++i)
		{
			while (checkVocal(i))
			{
				outFile << '\t' << vocalIters[i]->first << " = V " << i + 1;
				vocalIters[i]->second.save_cht(outFile);
				vocalValidations[i] = ++vocalIters[i] != m_vocals[i].end();
			}
		}

		while (percValid &&
			(!effectValid || percIter->first < effectIter->first) &&
			comparePosition_post(percIter->first) &&
			(!eventValid || percIter->first <= eventIter->first))
		{
			outFile << '\t' << percIter->first << percIter->second.save_cht();
			percValid = ++percIter != m_percussion.end();
		}

		while (eventValid &&
			(!effectValid || eventIter->first < effectIter->first) &&
			comparePosition_post(eventIter->first) &&
			(!percValid || eventIter->first < percIter->first))
		{
			for (const auto& str : eventIter->second)
				outFile << "\t" << eventIter->first << " = E \"" << UnicodeString::U32ToStr(str) << "\"\n";
			eventValid = ++eventIter != m_events.end();
		}
	}

	outFile << "}\n";
	outFile.flush();
}
