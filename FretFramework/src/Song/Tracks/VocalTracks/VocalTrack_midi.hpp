#pragma once
#include "VocalTrack.h"
#include "../Midi/MidiFile.h"

template<int numTracks>
inline void VocalTrack<numTracks>::scan_midi(int index, MidiTraversal& traversal, std::unique_ptr<NoteTrack_Scan>& track) const
{
	if (track == nullptr)
		track = std::make_unique<VocalTrack_Scan<numTracks>>();
	reinterpret_cast<VocalTrack_Scan<numTracks>*>(track.get())->scan_midi(index, traversal);
}

template <int numTracks>
inline void VocalTrack<numTracks>::load_midi(int index, MidiTraversal& traversal)
{
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
	while (traversal.next() && traversal.getEventType() != 0x2F)
	{
		const uint32_t position = traversal.getPosition();
		const unsigned char type = traversal.getEventType();

		if (type == 0x90 || type == 0x80)
		{
			const unsigned char note = traversal.extractChar();
			const unsigned char velocity = traversal.extractChar();

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
						auto& pair = m_vocals[index].back();
						if (pair.first == vocal)
						{
							const uint32_t sustain = position - vocal;
							if (sustain > 240)
								pair.second.init(sustain - 120);
							else
								pair.second.init(sustain / 2);

							pair.second.setPitch(note);
						}
					}

					vocal = position;
					pitch = note;
				}
				else if (note == pitch && !m_vocals[index].empty() && vocal != UINT32_MAX)
				{
					auto& pair = m_vocals[index].back();
					if (pair.first == vocal)
					{
						pair.second.init(position - vocal);
						pair.second.setPitch(note);
					}
					vocal = UINT32_MAX;
				}
			}
			// Star Power
			else if (note == s_starPowerReadNote)
			{
				if (type == 0x90 && velocity > 0)
					starPower = position;
				else if (starPower != UINT32_MAX)
				{
					addPhrase(starPower, new StarPowerPhrase(position - starPower));
					starPower = UINT32_MAX;
				}
			}
			else
			{
				switch (note)
				{
					// Percussion
				case 96:
				case 97:
					if (type == 0x90 && velocity > 0)
						perc = position;
					else
					{
						if (m_percussion.empty() || m_percussion.back().first != perc)
						{
							static std::pair<uint32_t, VocalPercussion> pairNode;
							pairNode.first = perc;
							m_percussion.push_back(pairNode);
						}

						if (note == 97)
							m_percussion.back().second.modify('N');
					}
					break;
					// Lyric Line/Phrase
				case 105:
				case 106:
					if (index == 2)
						break;

					if (type == 0x90 && velocity > 0)
					{
						if (phrase == UINT32_MAX)
							phrase = position;
					}
					else if (phrase != UINT32_MAX)
					{
						if (index == 0)
							addPhrase(phrase, new LyricLine(position - phrase));
						else
							addPhrase(phrase, new HarmonyLine(position - phrase));
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
		else if (type < 16)
		{
			if (traversal[0] == '[')
			{
				if (m_events.empty() || m_events.back().first < position)
				{
					static std::pair<uint32_t, std::vector<std::string>> pairNode;
					pairNode.first = position;
					m_events.push_back(pairNode);
				}

				m_events.back().second.push_back(traversal.extractText());
			}
			else
			{
				if (m_vocals[index].empty() || m_vocals[index].back().first != position)
				{
					static std::pair<uint32_t, Vocal> pairNode;
					pairNode.first = position;
					m_vocals[index].push_back(std::move(pairNode));
				}

				m_vocals[index].back().second.setLyric(traversal.extractText());
			}
		}
	}

	if ((m_vocals[index].size() < 100 || 2000 <= m_vocals[index].size()) && m_vocals[index].size() < m_vocals[index].capacity())
		m_vocals[index].shrink_to_fit();

	if (index == 0 && (m_percussion.size() < 20 || 400 <= m_percussion.size()) && m_percussion.size() < m_percussion.capacity())
		m_percussion.shrink_to_fit();
}

using namespace MidiFile;

template <int numTracks>
inline void VocalTrack<numTracks>::save_midi(const std::string& name, int trackIndex, std::fstream& outFile) const
{
	MidiFile::MidiChunk_Track events(name);
	if (trackIndex == 0)
	{
		for (const auto& vec : m_events)
			for (const auto& ev : vec.second)
				events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MetaEvent_Text(1, ev));
	}

	if (trackIndex < 2)
	{
		for (const auto& vec : m_effects)
			for (const auto& effect : vec.second)
				// Must be both true OR both false
				if ((trackIndex == 0) == (effect->getMidiNote() != 106))
				{
					events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote()));
					events.addEvent(vec.first + effect->getDuration(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote(), 0));
				}
	}

	auto vocalIter = m_vocals[trackIndex].begin();
	auto percIter = trackIndex == 0 ? m_percussion.begin() : m_percussion.end();
	bool vocalValid = vocalIter != m_vocals[trackIndex].end();
	auto percValid = percIter != m_percussion.end();

	while (vocalValid || percValid)
	{
		while (vocalValid && (!percValid || vocalIter->first <= percIter->first))
		{
			events.addEvent(vocalIter->first, new MidiFile::MidiChunk_Track::MetaEvent_Text(5, vocalIter->second.getLyric()));

			if (vocalIter->second.m_isPitched)
			{
				events.addEvent(vocalIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, vocalIter->second.getPitch()));

				uint32_t sustain = vocalIter->second.getSustain();
				if (sustain == 0)
					events.addEvent(vocalIter->first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, vocalIter->second.getPitch(), 0));
				else
					events.addEvent(vocalIter->first + sustain, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, vocalIter->second.getPitch(), 0));
			}
			vocalValid = ++vocalIter != m_vocals[trackIndex].end();
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
	int numWritten = 0;
	if (occupied())
	{
		numWritten = 1;
		if (numTracks > 1)
		{
			save_midi("HARM1", 0, outFile);
			// If either of the two following conditions are met,
			// then Harm2 MUST be written
			if (!m_vocals[1].empty())
				goto WriteHarm2;

			for (const auto& vec : m_effects)
				for (const auto& effect : vec.second)
					if (effect->getMidiNote() == 106)
						goto WriteHarm2;

			goto WriteHarm3;
		WriteHarm2:
			save_midi("HARM2", 1, outFile);
			++numWritten;

		WriteHarm3:
			if (!m_vocals[2].empty())
			{
				save_midi("HARM3", 2, outFile);
				++numWritten;
			}
		}
		else
			save_midi("PART VOCALS", 0, outFile);
	}
	return numWritten;
}
