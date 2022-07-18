#pragma once
#include "VocalTrack.h"
#include "../Midi/MidiFile.h"

template <>
template<int index>
void VocalTrack_Scan<3>::scan_midi(MidiTraversal& traversal)
{
	uint32_t lyric = UINT32_MAX;
	bool vocalActive = false;

	if constexpr (index == 0)
	{
		for (auto& vec : m_effects)
			for (Phrase* phr : vec.second)
				delete phr;
		m_effects.clear();
#ifndef _DEBUG
		static constexpr std::vector<Phrase*> phraseNode;
#else
		static const std::vector<Phrase*> phraseNode;
#endif // !_DEBUG

		// Only HARM1 will have to manage with polling all lyric line phrases
		uint32_t phrasePosition = UINT32_MAX;
		// Percussion notes are only valid in HARM1
		bool percActive = false;

		while (traversal.next())
		{
			const uint32_t position = traversal.getPosition();
			const unsigned char type = traversal.getEventType();

			if (type == 0x90 || type == 0x80)
			{
				const unsigned char note = traversal.getMidiNote();
				const unsigned char velocity = traversal.getVelocity();

				if (note == 105 || note == 106)
				{
					if (type == 0x90 && velocity > 0)
					{
						if (phrasePosition == UINT32_MAX)
							phrasePosition = position;
					}
					else if (phrasePosition != UINT32_MAX)
					{
						if (m_effects.empty() || m_effects.back().first != phrasePosition)
						{
							m_effects.emplace_back(phrasePosition, phraseNode);
							m_effects.back().second.push_back(new LyricLine(position - phrasePosition));
						}
						phrasePosition = UINT32_MAX;
					}
				}
				else if (m_scanValue == 0)
				{
					if (36 <= note && note < 85)
					{
						if (vocalActive)
							m_scanValue = 1;
						else if (phrasePosition != UINT32_MAX && type == 0x90 && velocity > 0 && lyric == position)
							vocalActive = true;
					}
					else if (note == 96)
					{
						if (percActive)
							m_scanValue = 1;
						else if (phrasePosition != UINT32_MAX && type == 0x90 && velocity > 0)
							percActive = true;
					}
				}
			}
			else if (type < 16)
			{
				if (m_scanValue == 0 && traversal[0] != '[')
					lyric = position;
			}
			else if (type == 0x2F)
				break;
		}
	}
	else if (!m_effects.empty())
	{
		auto phraseIter = m_effects.begin();
		// Harmonies are able to early exit
		const int finalValue = 1 << index;
		while (traversal.next() && m_scanValue < finalValue)
		{
			const uint32_t position = traversal.getPosition();

			while (phraseIter != m_effects.end() && position > phraseIter->first + phraseIter->second.front()->getDuration())
				++phraseIter;

			if (!vocalActive)
			{
				if (phraseIter == m_effects.end())
					break;
				else if (position < phraseIter->first)
					continue;
			}

			const unsigned char type = traversal.getEventType();
			if (type == 0x90 || type == 0x80)
			{
				const unsigned char note = traversal.getMidiNote();
				if (36 <= note && note < 85)
				{
					if (vocalActive)
						m_scanValue |= finalValue;
					else
					{
						const unsigned char velocity = traversal.getVelocity();
						if (type == 0x90 && velocity > 0 && lyric == position)
							vocalActive = true;
					}

				}
			}
			else if (type < 16)
			{
				if (traversal[0] != '[')
					lyric = position;
			}
			else if (type == 0x2F)
				break;
		}
	}
}

template<int numTracks>
template<int index>
inline void VocalTrack<numTracks>::scan_midi(MidiTraversal& traversal, std::unique_ptr<NoteTrack_Scan>& track) const
{
	if (track == nullptr)
		track = std::make_unique<VocalTrack_Scan<numTracks>>();
	reinterpret_cast<VocalTrack_Scan<numTracks>*>(track.get())->scan_midi<index>(traversal);
}

template<int numTracks>
template<int index>
inline void VocalTrack<numTracks>::load_midi(MidiTraversal& traversal)
{
#ifndef _DEBUG
	static constexpr std::vector<UnicodeString> eventNode;
	static constexpr std::vector<Phrase*> phraseNode;
	static const Vocal vocalNode;
	static constexpr VocalPercussion percNode;
#else
	static const std::vector<UnicodeString> eventNode;
	static const std::vector<Phrase*> phraseNode;
	static const Vocal vocalNode;
	static constexpr VocalPercussion percNode;
#endif // !_DEBUG

	uint32_t starPower = UINT32_MAX;
	uint32_t rangeShift = UINT32_MAX;
	uint32_t phrase = UINT32_MAX;
	uint32_t vocal = UINT32_MAX;
	uint32_t perc = UINT32_MAX;
	unsigned char pitch = 0;

	if (index == 0)
		clear();
	else if (!m_vocals[index].empty())
		m_vocals[index].clear();

	m_vocals[index].reserve(500);
	while (traversal.next())
	{
		const uint32_t position = traversal.getPosition();
		const unsigned char type = traversal.getEventType();

		if (type == 0x90 || type == 0x80)
		{
			const unsigned char note = traversal.getMidiNote();
			const unsigned char velocity = traversal.getVelocity();

			/*
			* Special values:
			*
			*	95 (drums) = Expert+ Double Bass
			*	103 = Solo
			*	104 = New Tap note
			*	105/106 = vocals
			*	108 = pro guitar
			*	109 = Drum flam
			*	115 = Pro guitar solo
			*	116 (default) = star power/overdrive
			*	120 - 125 = BRE
			*	126 = tremolo
			*	127 = trill
			*/

			// Notes
			if (36 <= note && note < 85)
			{
				if (type == 0x90 && velocity > 0)
				{
					// This is a security put in place to handle poor GH rips
					if (vocal != UINT32_MAX && !m_vocals[index].empty())
					{
						if (m_vocals[index].back().first == vocal)
						{
							uint32_t sustain = position - vocal;
							if (sustain > 240)
								sustain -= 120;
							else
								sustain /= 2;

							m_vocals[index].back().second.init(note, sustain);
						}
					}

					vocal = position;
					pitch = note;
				}
				else if (note == pitch && !m_vocals[index].empty() && vocal != UINT32_MAX)
				{
					if (m_vocals[index].back().first == vocal)
						m_vocals[index].back().second.init(note, position - vocal);
					vocal = UINT32_MAX;
				}
			}
			else if constexpr (index == 0)
			{
				// Star Power
				if (note == s_starPowerReadNote)
				{
					if (type == 0x90 && velocity > 0)
						starPower = position;
					else if (starPower != UINT32_MAX)
					{
						addPhrase(starPower, new StarPowerPhrase(position - starPower));
						starPower = UINT32_MAX;
					}
				}
				else if (note == 96 || note == 97)
				{
					if (type == 0x90 && velocity > 0)
						perc = position;
					else
					{
						if (m_percussion.empty() || m_percussion.back().first != perc)
							m_percussion.emplace_back(position, percNode);

						if (note == 97)
							m_percussion.back().second.modify('N');
					}
				}
				else
				{
					switch (note)
					{
						// Lyric Line/Phrase
					case 105:
					case 106:
						if (type == 0x90 && velocity > 0)
						{
							if (phrase == UINT32_MAX)
								phrase = position;
						}
						else if (phrase != UINT32_MAX)
						{
							addPhrase(phrase, new LyricLine(position - phrase));
							phrase = UINT32_MAX;
						}
						break;
						// Range Shift
					case 0:
						if (type == 0x90 && velocity > 0)
							rangeShift = position;
						else if (rangeShift != UINT32_MAX)
						{
							addPhrase(rangeShift, new RangeShift(position - rangeShift));
							rangeShift = UINT32_MAX;
						}
						break;
						// Lyric Shift
					case 1:
						if (type == 0x90 && velocity > 0)
							addPhrase(position, new LyricShift());
						break;
					}
				}
			}
			else if constexpr (index == 1)
			{
				if (note == 105 || note == 106)
				{
					if (type == 0x90 && velocity > 0)
					{
						if (phrase == UINT32_MAX)
							phrase = position;
					}
					else if (phrase != UINT32_MAX)
					{
						addPhrase(phrase, new HarmonyLine(position - phrase));
						phrase = UINT32_MAX;
					}
				}
			}
		}
		else if (type < 16)
		{
			if (traversal[0] == '[')
			{
				if (m_events.empty() || m_events.back().first < position)
					m_events.emplace_back(position, eventNode);

				m_events.back().second.push_back(traversal.extractText());
			}
			else
			{
				if (m_vocals[index].empty() || m_vocals[index].back().first != position)
					m_vocals[index].emplace_back(position, vocalNode);

				m_vocals[index].back().second.setLyric(traversal.extractLyric());
			}
		}
		else if (type == 0x2F)
			break;
	}

	if ((m_vocals[index].size() < 100 || 2000 <= m_vocals[index].size()) && m_vocals[index].size() < m_vocals[index].capacity())
		m_vocals[index].shrink_to_fit();

	if (index == 0 && (m_percussion.size() < 20 || 400 <= m_percussion.size()) && m_percussion.size() < m_percussion.capacity())
		m_percussion.shrink_to_fit();
}

using namespace MidiFile;

template <int numTracks>
template <int index>
inline void VocalTrack<numTracks>::save_midi(const std::string& name, std::fstream& outFile) const
{
	MidiFile::MidiChunk_Track events(name);
	if constexpr (index == 0)
	{
		for (const auto& vec : m_events)
			for (const auto& ev : vec.second)
				events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MetaEvent_Text(1, ev));

		for (const auto& vec : m_effects)
			for (const auto& effect : vec.second)
				if (effect->getMidiNote() != 106)
				{
					events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote()));
					events.addEvent(vec.first + effect->getDuration(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote(), 0));
				}
	}
	else if constexpr (index == 1)
	{
		for (const auto& vec : m_effects)
			for (const auto& effect : vec.second)
				if (effect->getMidiNote() == 106)
				{
					events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote()));
					events.addEvent(vec.first + effect->getDuration(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote(), 0));
				}
	}

	auto vocalIter = m_vocals[index].begin();
	auto percIter = index == 0 ? m_percussion.begin() : m_percussion.end();
	bool vocalValid = vocalIter != m_vocals[index].end();
	auto percValid = percIter != m_percussion.end();

	while (vocalValid || percValid)
	{
		while (vocalValid && (!percValid || vocalIter->first <= percIter->first))
		{
			events.addEvent(vocalIter->first, new MidiFile::MidiChunk_Track::MetaEvent_Text(5, vocalIter->second.getLyric()));

			if (vocalIter->second.isPitched())
			{
				events.addEvent(vocalIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, vocalIter->second.getPitch()));

				uint32_t sustain = vocalIter->second.getDuration();
				if (sustain == 0)
					events.addEvent(vocalIter->first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, vocalIter->second.getPitch(), 0));
				else
					events.addEvent(vocalIter->first + sustain, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, vocalIter->second.getPitch(), 0));
			}
			vocalValid = ++vocalIter != m_vocals[index].end();
		}

		while (percValid && (!vocalValid || percIter->first < vocalIter->first))
		{
			if (percIter->second.m_isPlayable)
			{
				events.addEvent(percIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 96));
				events.addEvent(percIter->first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 96, 0));
			}
			else
			{
				events.addEvent(percIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 97));
				events.addEvent(percIter->first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 97, 0));
			}
			percValid = ++percIter != m_percussion.end();
		}
	}

	events.writeToFile(outFile);
}

template <int numTracks>
inline int VocalTrack<numTracks>::save_midi(std::fstream& outFile) const
{
	if (occupied())
	{
		if constexpr (numTracks == 1)
			save_midi<0>("PART VOCALS", outFile);
		else
		{
			save_midi<0>("HARM1", outFile);
			save_midi<1>("HARM2", outFile);
			save_midi<2>("HARM3", outFile);
		}
		return numTracks;
	}
	return 0;
}
