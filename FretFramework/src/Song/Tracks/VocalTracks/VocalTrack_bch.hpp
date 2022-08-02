#pragma once
#include "VocalTrack.h"
#include "NoteExceptions.h"
#include <iostream>

template<int numTracks>
inline void VocalTrack_Scan<numTracks>::scan_bch(BCHTraversal& traversal)
{
	unsigned char expectedScan = 0;
	uint32_t vocalPhraseEnd = 0;
	bool checked[numTracks]{};

	if (!traversal.validateChunk("LYRC"))
		goto ValidateAnim;
	else if (traversal.doesNextTrackExist() && !traversal.checkNextChunk("ANIM") && !traversal.checkNextChunk("INST") && !traversal.checkNextChunk("VOCL"))
	{
		// Sets the next track to whatever next valid track comes first, if any exist

		const unsigned char* const anim = traversal.findNextChunk("ANIM");
		const unsigned char* const inst = traversal.findNextChunk("INST");
		const unsigned char* const vocl = traversal.findNextChunk("VOCL");
		if (anim && (!inst || anim < inst) && (!vocl || anim < vocl))
			traversal.setNextTrack(anim);
		else if (inst && (!vocl || inst < vocl))
			traversal.setNextTrack(inst);
		else
			traversal.setNextTrack(vocl);
	}

	expectedScan = traversal.extractChar();
	if (expectedScan >= 8)
		traversal.skipTrack();

	while (traversal.next())
	{
		try
		{
			if (traversal.getEventType() == 9)
			{
				if (traversal.getPosition() < vocalPhraseEnd)
				{
					unsigned char lane = traversal.extractChar();
					if (0 < lane && lane <= numTracks)
					{
						--lane;
						if (!checked[lane])
						{
							const uint32_t lyricLength = traversal.extractVarType();
							traversal.move(lyricLength);

							// Pitch AND sustain required
							if (traversal.extractChar() && traversal.extractVarType())
							{
								if (expectedScan == 0)
								{
									m_scanValue = 8;
									traversal.skipTrack();
								}
								else
								{
									checked[lane] = true;
									m_scanValue |= 1 << lane;

									if (m_scanValue == expectedScan)
										// No need to check the rest of the noteTrack's data
										traversal.skipTrack();
								}
							}
						}
					}
				}
			}
			else if (traversal.getEventType() == 5)
			{
				const unsigned char phrase = traversal.extractChar();
				if (phrase == 4)
					vocalPhraseEnd = traversal.getPosition() + traversal.extractVarType();
			}
		}
		catch (...)
		{
		}
	}

ValidateAnim:
	if (traversal.validateChunk("ANIM"))
	{
		if (traversal.doesNextTrackExist() && !traversal.checkNextChunk("INST") && !traversal.checkNextChunk("VOCL"))
		{
			// Sets the next track to whatever next valid track comes first, if any exist

			const unsigned char* const inst = traversal.findNextChunk("INST");
			const unsigned char* const vocl = traversal.findNextChunk("VOCL");
			if (inst && (!vocl || inst < vocl))
				traversal.setNextTrack(inst);
			else
				traversal.setNextTrack(vocl);
		}
		traversal.skipTrack();
	}
}

template<int numTracks>
inline void VocalTrack<numTracks>::scan_bch(BCHTraversal& traversal, std::unique_ptr<NoteTrack_Scan>& track) const
{
	if (track == nullptr)
		track = std::make_unique<VocalTrack_Scan<numTracks>>();
	track->scan_bch(traversal);
}

template<int numTracks>
inline void VocalTrack<numTracks>::load_bch(BCHTraversal& traversal)
{
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

	uint32_t vocalPhraseEnd[2] = { 0, 0 };
	uint32_t starPowerEnd = 0;
	uint32_t soloEnd = 0;
	uint32_t rangeShiftEnd = 0;

	if (!traversal.validateChunk("LYRC"))
		goto ValidateAnim;
	else if (traversal.doesNextTrackExist() && !traversal.checkNextChunk("ANIM") && !traversal.checkNextChunk("INST") && !traversal.checkNextChunk("VOCL"))
	{
		// Sets the next track to whatever next valid track comes first, if any exist

		const unsigned char* const anim = traversal.findNextChunk("ANIM");
		const unsigned char* const inst = traversal.findNextChunk("INST");
		const unsigned char* const vocl = traversal.findNextChunk("VOCL");
		if (anim && (!inst || anim < inst) && (!vocl || anim < vocl))
			traversal.setNextTrack(anim);
		else if (inst && (!vocl || inst < vocl))
			traversal.setNextTrack(inst);
		else
			traversal.setNextTrack(vocl);
	}

	clear();
	for (auto& track : m_vocals)
		track.reserve(1000);
	m_percussion.reserve(200);

	traversal.move(1);
	while (traversal.next())
	{
		try
		{
			switch (traversal.getEventType())
			{
			case 9:
			{
				unsigned char lane = traversal.extractChar();
				if (lane > numTracks)
					throw InvalidNoteException(lane);

				if (lane == 0)
				{
					if (m_percussion.empty() || m_percussion.back().first != traversal.getPosition())
						m_percussion.emplace_back(traversal.getPosition(), percNode);

					if (traversal.extract(lane))
						m_percussion.back().second.modify_binary(lane);
				}
				else
				{
					--lane;
					if (m_vocals[lane].empty() || m_vocals[lane].back().first != traversal.getPosition())
						m_vocals[lane].emplace_back(traversal.getPosition(), vocalNode);

					m_vocals[lane].back().second.init(traversal);
				}
				break;
			}
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

ValidateAnim:
	if (traversal.validateChunk("ANIM"))
	{
		if (traversal.doesNextTrackExist() && !traversal.checkNextChunk("INST") && !traversal.checkNextChunk("VOCL"))
		{
			// Sets the next track to whatever next valid track comes first, if any exist

			const unsigned char* const inst = traversal.findNextChunk("INST");
			const unsigned char* const vocl = traversal.findNextChunk("VOCL");
			if (inst && (!vocl || inst < vocl))
				traversal.setNextTrack(inst);
			else
				traversal.setNextTrack(vocl);
		}
		traversal.skipTrack();
	}
}

template <int numTracks>
inline bool VocalTrack<numTracks>::save_bch(std::fstream& outFile) const
{
	if (!occupied())
		return false;

	outFile.write("VOCL", 4);

	auto start = outFile.tellp();
	uint32_t length = 0;
	outFile.write((char*)&length, 4);
	outFile.put(m_instrumentID);

	outFile.write("LYRC", 4);
	auto lyrcStart = outFile.tellp();
	outFile.write((char*)&length, 4);

	unsigned char scanValue = 0;
	outFile.put(scanValue);

	uint32_t numEvents = 0;
	outFile.write((char*)&numEvents, 4);

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

	static char buffer[1030];
	bool scanWasChecked[numTracks]{};

	uint32_t prevPosition = 0;
	while (effectValid || checkVocals() || eventValid)
	{
		while (effectValid &&
			comparePosition_pre(effectIter->first) &&
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

		for (int i = 0; i < numTracks; ++i)
		{
			while (checkVocal(i))
			{
				if (!scanWasChecked[i] && vocalIters[i]->second.isPitched())
				{
					scanValue |= 1 << i;
					scanWasChecked[i] = true;
				}

				WebType(vocalIters[i]->first - prevPosition).writeToFile(outFile);

				char* current = buffer;
				vocalIters[i]->second.save_bch(i + 1, current);

				const uint32_t length(uint32_t(current - buffer));
				// Event type - Single (Lyric)
				outFile.put(9);
				WebType::writeToFile(length, outFile);
				outFile.write(buffer, length);
				++numEvents;

				prevPosition = vocalIters[i]->first;
				vocalValidations[i] = ++vocalIters[i] != m_vocals[i].end();
			}
		}
		

		while (percValid &&
			(!effectValid || percIter->first < effectIter->first) &&
			comparePosition_post(percIter->first) &&
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
			comparePosition_post(eventIter->first) &&
			(!percValid || eventIter->first < percIter->first))
		{
			WebType delta(eventIter->first - prevPosition);
			for (const auto& str : eventIter->second)
			{
				delta.writeToFile(outFile);
				outFile.put(3);
				UnicodeString::U32ToBCH(str, outFile);
				delta = 0;
			}
			numEvents += (uint32_t)eventIter->second.size();
			prevPosition = eventIter->first;
			eventValid = ++eventIter != m_events.end();
		}
	}

	auto end = outFile.tellp();
	length = uint32_t(end - lyrcStart) - 4;
	outFile.seekp(lyrcStart);
	outFile.write((char*)&length, 4);
	outFile.put(scanValue);
	outFile.write((char*)&numEvents, 4);
	outFile.seekp(end);

	outFile.write("ANIM", 4);
	length = 4;
	outFile.write((char*)&length, 4);
	numEvents = 0;
	outFile.write((char*)&numEvents, 4);

	end = outFile.tellp();
	length = uint32_t(end - start) - 4;
	outFile.seekp(start);
	outFile.write((char*)&length, 4);
	outFile.seekp(end);
	return true;
}
