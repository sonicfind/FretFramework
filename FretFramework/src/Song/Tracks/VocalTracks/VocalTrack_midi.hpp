#pragma once
#include "VocalTrack.h"

template <size_t numTracks>
void VocalTrack<numTracks>::load_midi(int index, const unsigned char* currPtr, const unsigned char* const end)
{
	uint32_t starPower = UINT32_MAX;
	uint32_t rangeShift = UINT32_MAX;
	uint32_t phrase = UINT32_MAX;
	uint32_t vocal = UINT32_MAX;

	unsigned char syntax = 0xFF;
	uint32_t position = 0;
	while (currPtr < end)
	{
		position += VariableLengthQuantity(currPtr);
		unsigned char tmpSyntax = *currPtr++;
		unsigned char note = 0;
		if (tmpSyntax & 0b10000000)
		{
			syntax = tmpSyntax;
			if (syntax == 0x80 || syntax == 0x90)
				note = *currPtr++;
			else
			{
				if (syntax == 0xFF)
				{
					unsigned char type = *currPtr++;
					VariableLengthQuantity length(currPtr);
					if (type < 16)
					{
						std::string ev((char*)currPtr, length);
						if (ev[0] == '[')
						{
							if (m_events.empty() || m_events.back().first < position)
							{
								static std::pair<uint32_t, std::vector<std::string>> pairNode;
								pairNode.first = position;
								m_events.push_back(pairNode);
							}

							m_events.back().second.push_back(std::move(ev));
						}
						else
						{
							if (m_vocals[index].empty() || m_vocals[index].back().first != position)
							{
								static std::pair<uint32_t, Vocal> pairNode;
								pairNode.first = position;
								m_vocals[index].push_back(pairNode);
							}
							m_vocals[index].back().second.setLyric(std::move(ev));
						}
					}

					if (type == 0x2F)
						break;

					currPtr += length;
				}
				else
				{
					switch (syntax)
					{
					case 0xF0:
					case 0xF7:
						currPtr += VariableLengthQuantity(currPtr);
						break;
					case 0xB0:
					case 0xA0:
					case 0xE0:
					case 0xF2:
						currPtr += 2;
						break;
					case 0xC0:
					case 0xD0:
					case 0xF3:
						++currPtr;
					}
				}
				continue;
			}
		}
		else
		{
			switch (syntax)
			{
			case 0xF0:
			case 0xF7:
			case 0xFF:
				throw std::exception();
			default:
				note = tmpSyntax;
			}
		}

		switch (syntax)
		{
		case 0x90:
		case 0x80:
		{
			unsigned char velocity = *currPtr++;
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
			*	116 = star power/overdrive
			*	120 - 125 = BRE
			*	126 = tremolo
			*	127 = trill
			*/

			// Notes
			if (36 <= note && note < 85)
			{
				if (syntax == 0x90 && velocity > 0)
					vocal = position;
				else if (!m_vocals[index].empty())
				{
					auto& pair = m_vocals[index].back();
					if (pair.first == vocal)
					{
						pair.second.init(position - vocal);
						pair.second.setPitch(note);
					}
				}
			}
			else
			{
				switch (note)
				{
					// Percussion
				case 96:
				case 97:
					if (syntax == 0x90 && velocity > 0)
					{
						if (m_percussion.empty() || m_percussion.back().first != position)
						{
							static std::pair<uint32_t, VocalPercussion> pairNode;
							pairNode.first = position;
							m_percussion.push_back(pairNode);
						}

						if (note == 97)
							m_percussion.back().second.modify('N');
					}
					break;
					// Lyric Line/Phrase
				case 105:
				case 106:
					switch (index)
					{
					case 0:
						if (syntax == 0x90 && velocity > 0)
							phrase = position;
						else
							addPhrase(phrase, new LyricLine(position - phrase));
						break;
					case 1:
						if (syntax == 0x90 && velocity > 0)
							phrase = position;
						else
							addPhrase(phrase, new HarmonyLine(position - phrase));
					}
					break;
					// Range Shift
				case 0:
					if (syntax == 0x90 && velocity > 0)
						rangeShift = position;
					else
						addPhrase(rangeShift, new RangeShift(position - rangeShift));
					break;
					// Lyric Shift
				case 1:
					if (syntax == 0x90 && velocity > 0)
						addPhrase(position, new LyricShift());
					break;
					// Star Power
				case 116:
					if (syntax == 0x90 && velocity > 0)
						starPower = position;
					else
						addPhrase(starPower, new StarPowerPhrase(position - starPower));
					break;
				}
			}
			break;
		}
		case 0xB0:
		case 0xA0:
		case 0xE0:
		case 0xF2:
			++currPtr;
		}
	}
}

template <size_t numTracks>
void VocalTrack<numTracks>::save_midi(const std::string& name, int trackIndex, std::fstream& outFile) const
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

			if (vocalIter->second.m_isActive)
			{
				events.addEvent(vocalIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, vocalIter->second.getPitch()));

				uint32_t sustain = vocalIter->second.getSustain();
				if (sustain == 0)
					events.addEvent(vocalIter->first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, vocalIter->second.getPitch(), 0));
				else
					events.addEvent(vocalIter->first + sustain, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, vocalIter->second.getPitch(), 0));
			}
			vocalValid = ++vocalIter != m_vocals[0].end();
		}

		while (percValid && (!vocalValid || percIter->first < vocalIter->first))
		{
			if (percIter->second.m_isActive)
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

template <size_t numTracks>
int VocalTrack<numTracks>::save_midi(std::fstream& outFile) const
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