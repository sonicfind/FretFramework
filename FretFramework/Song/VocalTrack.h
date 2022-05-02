#pragma once
#include <iostream>
#include "TimedNode.h"
#include "Effect.h"
#include "Midi/MidiFile.h"
#include "../VectorIteration.h"
using namespace MidiFile;

template <size_t numTracks>
class VocalTrack
{
protected:
	const char* const m_name;
	std::vector<std::pair<uint32_t, VocalNote<numTracks>>> m_notes;
	std::vector<std::pair<uint32_t, std::vector<SustainableEffect*>>> m_effects;
	std::vector<std::pair<uint32_t, std::vector<std::string>>> m_events;

public:
	VocalTrack(const char* name) : m_name(name) {}

	// Returns whether this track contains any notes
	// ONLY checks for notes
	bool hasNotes() const
	{
		return m_notes.size();
	}

	// Returns whether this track contains any notes, effects, soloes, or other events
	bool occupied() const
	{
		return hasNotes() ||
			m_effects.size() ||
			m_events.size();
	}

	void addLyric(int lane, uint32_t position, const std::string& lyric)
	{
		VectorIteration::try_emplace(m_notes, position).setLyric(lane, lyric);
	}

	void addPercussion(uint32_t position, bool playable)
	{
		VocalNote<numTracks>& vocal = VectorIteration::try_emplace(m_notes, position);
		vocal.init(0);
		if (!playable)
			vocal.modify('N');
	}

	void addEffect(uint32_t position, SustainableEffect* effect)
	{
		VectorIteration::try_emplace(m_effects, position).push_back(effect);
	}

	void addEvent(uint32_t position, const std::string& ev)
	{
		VectorIteration::try_emplace(m_events, position).push_back(ev);
	}

	void setPitch(int lane, uint32_t position, char pitch, uint32_t sustain = 0)
	{
		if (lane == 0)
			return false;

		try
		{
			VocalNote<numTracks>& vocal = VectorIteration::getIterator(m_notes, position);
			vocal.init(sustain);
			vocal.setPitch(pitch);
		}
		catch (...)
		{
			return false;
		}
	}

	void clear()
	{
		m_notes.clear();
		m_events.clear();
		for (auto& vec : m_effects)
			for (auto& eff : vec.second)
				delete eff;
		m_effects.clear();
	}

	void load_cht(std::fstream& inFile)
	{
		clear();
		static char buffer[512] = { 0 };
		inFile.getline(buffer, 512);

		int numNotes = 0;
		if (sscanf_s(buffer, "\tNotes = %lu", &numNotes) == 1)
			m_notes.reserve(numNotes);

		uint32_t phrase = 0;
		bool phraseActive = false;
		uint32_t prevPosition = 0;
		while (inFile.getline(buffer, 512) && !memchr(buffer, '}', 2))
		{
			const char* str = buffer;
			uint32_t position;
			char type[3] = { 0 };
			int count;
			int numRead = sscanf_s(str, " %lu = %[^ ]%n", &position, type, 3, &count);
			if (numRead == 2 && prevPosition <= position)
			{
				prevPosition = position;
				str += count;
				switch (type[0])
				{
				case 'v':
				case 'V':
					if (!m_notes.empty() && m_notes.back().first == position)
					{
						try
						{
							m_notes.back().second.vocalize_cht(str);
						}
						catch (EndofLineException EoL)
						{
							std::cout << "Unable to parse full list of pitches at " << position << " (\"" << str << "\")" << std::endl;
						}
					}
					break;
				case 'p':
				case 'P':
				{
					int index = 0;
					if (numTracks > 1)
					{
						char phraseIndex;
						if (sscanf_s(str, " %c", &phraseIndex, 1) != 0 && (phraseIndex == 'h' || phraseIndex == 'H'))
							index = 1;
					}

					switch (type[1])
					{
					// Phrase Start
					case 0:
						if (phraseActive)
						{
							if (index == 0)
								addEffect(phrase, new LyricLine(position - phrase));
							else
								addEffect(phrase, new HarmonyPhrase(position - phrase));
						}
						phrase = position;
						phraseActive = true;
						break;
					// Phrase End
					case 'e':
					case 'E':
						if (index == 0)
							addEffect(phrase, new LyricLine(position - phrase));
						else
							addEffect(phrase, new HarmonyPhrase(position - phrase));
						phraseActive = false;
					}
					break;
				}
				case 'e':
				case 'E':
				{
					++str;
					if (*str == '\"')
						++str;

					char strBuf[256] = { 0 };
					sscanf_s(str, "%[^\"]s", &strBuf, 256);
					if (m_events.empty() || m_events.back().first < position)
					{
						static std::pair<uint32_t, std::vector<std::string>> pairNode;
						pairNode.first = position;
						m_events.push_back(pairNode);
					}

					m_events.back().second.push_back(strBuf);
					break;
				}
				case 'n':
				case 'N':
				case 'c':
				case 'C':
				{
					if (m_notes.empty() || m_notes.back().first != position)
					{
						static std::pair<uint32_t, VocalNote<numTracks>> pairNode;
						pairNode.first = position;
						m_notes.push_back(pairNode);
					}

					switch (type[0])
					{
					case 'n':
					case 'N':
						m_notes.back().second.init_cht_single(str);
						break;
					default:
						m_notes.back().second.init_cht_chord(str);
					}
					break;
				}
				case 's':
				case 'S':
				{
					int phrase;
					uint32_t duration = 0;
					if (sscanf_s(str, " %i %lu", &phrase, &duration) != 0)
					{
						auto check = [&]()
						{
							if (m_effects.empty() || m_effects.back().first < position)
							{
								static std::pair<uint32_t, std::vector<SustainableEffect*>> pairNode;
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
							m_effects.back().second.push_back(new HarmonyPhrase(duration));
							break;
						case 64:
						case 65:
						case 66:
							break;
						case 67:
							check();
							m_effects.back().second.push_back(new LyricShift(duration));
							break;
						
						default:
							std::cout << "Error at position " << position << ": unrecognized special phrase type (" << phrase << ')' << std::endl;
						}
					}
					break;
				}
				default:
					// Need to add a line count tracking variable for easy debugging by the end-user
					std::cout << "Error at position: unrecognized node type (" << type << ')' << std::endl;
				}
			}
			else
			{
				// Need to add a line count tracking variable for easy debugging by the end-user
				std::cout << "Error reading line: ";
				if (numRead != 2)
					std::cout << "Improper node setup (\"" << str << "\")" << std::endl;
				else
					std::cout << "Node position out of order (" << position << ')' << std::endl;
			}
		}

		if (!inFile)
		{
			std::cout << "Error in track Vocals" << std::endl;
			throw EndofFileException();
		}

		if (m_notes.size() < m_notes.capacity())
			m_notes.shrink_to_fit();
	}

	void load_midi(int index, const unsigned char* currPtr, const unsigned char* const end)
	{
		uint32_t starPower = UINT32_MAX;
		uint32_t pitchShift = UINT32_MAX;
		uint32_t lyricShift = UINT32_MAX;
		uint32_t phrase = UINT32_MAX;
		uint32_t vocal = UINT32_MAX;

		unsigned char syntax = 0;
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
						if (type == 1 || type == 5)
						{
							std::string ev((char*)currPtr, length);
							if (type == 1 && ev[0] == '[')
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
								VectorIteration::try_emplace(m_notes, position).setLyric(index + 1, std::move(ev));
						}

						if (type != 0x2F)
							currPtr += length;
						else
							break;
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
			else if (syntax & 0b10000000)
				note = tmpSyntax;
			else
				throw std::exception();

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
					else
					{
						auto& vocalNote = VectorIteration::getIterator(m_notes, vocal);
						vocalNote.init(index + 1, position - vocal);
						vocalNote.setPitch(index + 1, note);
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
							auto& vocalNote = VectorIteration::try_emplace(m_notes, position);
							vocalNote.init(0);
							if (note == 97)
								vocalNote.modify('N');
						}
						break;
						// Lyric Line/Phrase
						// For index == 1, harmony lyric shift
					case 105:
					case 106:
						switch (index)
						{
						case 0:
							if (syntax == 0x90 && velocity > 0)
								phrase = position;
							else
								addEffect(phrase, new LyricLine(position - phrase));
							break;
						case 1:
							if (syntax == 0x90 && velocity > 0)
								phrase = position;
							else
								addEffect(phrase, new HarmonyPhrase(position - phrase));
						}
						break;
						// Range Shift
					case 0:
						if (syntax == 0x90 && velocity > 0)
							pitchShift = position;
						else
							addEffect(pitchShift, new RangeShift(position - pitchShift));
						break;
						// Lyric Shift
					case 1:
						if (syntax == 0x90 && velocity > 0)
							lyricShift = position;
						else
							addEffect(lyricShift, new LyricShift(position - lyricShift));
						break;
						// Star Power
					case 116:
						if (syntax == 0x90 && velocity > 0)
							starPower = position;
						else
							addEffect(starPower, new StarPowerPhrase(position - starPower));
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

	void save_cht(std::fstream& outFile) const
	{
		if (!occupied())
			return;

		outFile << "[" << m_name << "]\n{\n";
		outFile << "\tNotes = " << m_notes.size() << '\n';

		auto vocalIter = m_notes.begin();
		auto effectIter = m_effects.begin();
		auto eventIter = m_events.begin();
		bool vocalValid = vocalIter != m_notes.end();
		bool effectValid = effectIter != m_effects.end();
		bool eventValid = eventIter != m_events.end();

		while (effectValid || vocalValid || eventValid)
		{
			while (effectValid &&
				(!vocalValid || effectIter->first <= vocalIter->first) &&
				(!eventValid || effectIter->first <= eventIter->first))
			{
				for (const auto& eff : effectIter->second)
					eff->save_cht(effectIter->first, outFile, "\t");
				effectValid = ++effectIter != m_effects.end();
			}

			while (vocalValid &&
				(!effectValid || vocalIter->first < effectIter->first) &&
				(!eventValid || vocalIter->first <= eventIter->first))
			{
				vocalIter->second.save_cht(vocalIter->first, outFile);
				vocalValid = ++vocalIter != m_notes.end();
			}

			while (eventValid &&
				(!effectValid || eventIter->first < effectIter->first) &&
				(!vocalValid || eventIter->first < vocalIter->first))
			{
				for (const auto& str : eventIter->second)
					outFile << "\t" << eventIter->first << " = E \"" << str << "\"\n";
				eventValid = ++eventIter != m_events.end();
			}
		}

		outFile << "}\n";
		outFile.flush();
	}

protected:

	void save_midi(const std::string& name, int trackIndex, std::fstream& outFile) const
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
				if (!((trackIndex == 0) ^ (effect->getMidiNote() != 106)))
				{
					events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote()));
					events.addEvent(vec.first + effect->getDuration(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote(), 0));
				}
		}

		for (const std::pair<uint32_t, VocalNote<numTracks>>& pair : m_notes)
		{
			if (pair.second.m_special)
			{
				if (trackIndex == 0)
				{
					int midiNote = 96;
					if (pair.second.m_special.m_noiseOnly)
						++midiNote;

					events.addEvent(pair.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote));
					events.addEvent(pair.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote, 0));
				}
			}
			else
			{
				const Vocal& vocal = pair.second.m_colors[trackIndex];
				// Checks if a pitch needs to be added
				if (vocal)
				{
					events.addEvent(pair.first, new MidiFile::MidiChunk_Track::MetaEvent_Text(5, vocal.getLyric()));
					if (vocal.isSung())
					{
						events.addEvent(pair.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, vocal.getPitch()));
						uint32_t sustain = vocal.getSustain();
						if (sustain == 0)
							events.addEvent(pair.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, vocal.getPitch(), 0));
						else
							events.addEvent(pair.first + sustain, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, vocal.getPitch(), 0));
					}
				}
			}
		}

		events.writeToFile(outFile);
	}

public:
	int save_midi(std::fstream& outFile) const
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
				for (const auto& vec : m_effects)
					for (const auto& effect : vec.second)
						if (effect->getMidiNote() == 106)
							goto WriteHarm2;

				for (const auto& vocal : m_notes)
					if (vocal.second.m_colors[1])
						goto WriteHarm2;

				goto WriteHarm3;
			WriteHarm2:
				save_midi("HARM2", 1, outFile);
				++numWritten;

			WriteHarm3:
				for (const auto& vocal : m_notes)
					if (vocal.second.m_colors[2])
					{
						save_midi("HARM3", 2, outFile);
						++numWritten;
						break;
					}
			}
			else
				save_midi("PART VOCALS", 0, outFile);
		}
		return numWritten;
	}

	~VocalTrack()
	{
		for (auto& vec : m_effects)
			for (auto& eff : vec.second)
				delete eff;
	}
};
