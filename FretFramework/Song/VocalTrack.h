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
	const char* const m_name;
	std::vector<std::pair<uint32_t, Vocal>> m_vocals[numTracks];
	std::vector<std::pair<uint32_t, VocalPercussion>> m_percussion;
	std::vector<std::pair<uint32_t, std::vector<SustainableEffect*>>> m_effects;
	std::vector<std::pair<uint32_t, std::vector<std::string>>> m_events;

	void init_cht_single(uint32_t position, const char* str)
	{
		int lane, count;
		if (sscanf_s(str, " %i%n", &lane, &count) != 1)
			throw EndofLineException();

		if (lane > numTracks)
			throw InvalidNoteException(lane);

		str += count;
		if (lane == 0)
		{
			if (m_percussion.empty() || m_percussion.back().first != position)
			{
				static std::pair<uint32_t, VocalPercussion> pairNode;
				pairNode.first = position;
				m_percussion.push_back(pairNode);
			}

			char disable;
			if (sscanf_s(str, " %c", &disable, 1) == 1)
				m_percussion.back().second.modify(disable);
		}
		else
		{
			str += 2;
			char strBuf[256] = { 0 };
			if (sscanf_s(str, "%[^\"]%n", &strBuf, 256, &count) == EOF)
				throw EndofLineException();

			if (m_vocals[lane - 1].empty() || m_vocals[lane - 1].back().first != position)
			{
				static std::pair<uint32_t, Vocal> pairNode;
				pairNode.first = position;
				m_vocals[lane - 1].push_back(pairNode);
			}

			m_vocals[lane - 1].back().second.setLyric(strBuf);
			str += count + 1;

			// Read pitch if found
			int pitch = 0;
			uint32_t sustain = 0;
			switch (sscanf_s(str, " %i %lu", &pitch, &sustain))
			{
			case 2:
				m_vocals[lane - 1].back().second.setPitch(pitch);
				m_vocals[lane - 1].back().second.init(sustain);
				break;
			case 1:
				m_vocals[lane - 1].pop_back();
				throw EndofLineException();
			}
		}
	}

	void init_cht_chord(uint32_t position, const char* str)
	{
		int colors;
		int count;
		if (sscanf_s(str, " %i%n", &colors, &count) != 1)
			throw EndofLineException();

		str += count;
		int i = 0;
		int numAdded = 0;
		int lane;
		while (i < colors && sscanf_s(str, " %i%n", &lane, &count) == 1)
		{
			if (lane > numTracks)
				throw InvalidNoteException(lane);

			if (lane == 0)
			{
				if (m_percussion.empty() || m_percussion.back().first != position)
				{
					static std::pair<uint32_t, VocalPercussion> pairNode;
					pairNode.first = position;
					m_percussion.push_back(pairNode);
					numAdded = 1;

					for (auto& track : m_vocals)
						if (!track.empty() && track.back().first == position)
							track.pop_back();
				}
				str += count;
			}
			else
			{
				str += count + 2;
				char strBuf[256] = { 0 };
				if (sscanf_s(str, "%[^\"]%n", &strBuf, 256, &count) == EOF)
					throw EndofLineException();

				if (m_vocals[lane - 1].empty() || m_vocals[lane - 1].back().first != position)
				{
					static std::pair<uint32_t, Vocal> pairNode;
					pairNode.first = position;
					m_vocals[lane - 1].push_back(pairNode);
				}

				m_vocals[lane - 1].back().second.setLyric(strBuf);

				if (m_percussion.empty() || m_percussion.back().first != position)
					++numAdded;
				else
					m_percussion.pop_back();
				str += count + 1;
			}
			++i;
		}

		if (numAdded == 0)
			throw InvalidNoteException();
	}

	void modify_cht(uint32_t position, const char* str)
	{
		int numMods;
		int count;
		if (sscanf_s(str, " %i%n", &numMods, &count) == 1)
		{
			str += count;
			char modifier;
			for (int i = 0;
				i < numMods && sscanf_s(str, " %c%n", &modifier, 1, &count) == 1;
				++i)
			{
				str += count;
				switch (modifier)
				{
				case 'n':
				case 'N':
					if (!m_percussion.empty() && m_percussion.back().first == position)
						m_percussion.back().second.modify('N');
				}
			}
		}
	}

	void vocalize_cht(uint32_t position, const char* str)
	{
		int numPitches;
		int count;
		if (sscanf_s(str, " %i%n", &numPitches, &count) == 1)
		{
			str += count;
			int lane;
			int pitch;
			uint32_t sustain;

			int i = 0;
			int numVocalized = 0;
			while (i < numPitches && sscanf_s(str, " %i %i %lu%n", &lane, &pitch, &sustain, &count) == 3)
			{
				str += count;
				if (0 < lane && lane <= numTracks &&
					!m_vocals[lane - 1].empty() && m_vocals[lane - 1].back().first == position)
				{
					m_vocals[lane - 1].back().second.setPitch(pitch);
					m_vocals[lane - 1].back().second.init(sustain);
					++numVocalized;
				}
				++i;
			}

			if (numVocalized == 0)
				std::cout << "Unable to vocalize lyrics at position " << std::endl;

			if (i < numPitches)
				throw EndofLineException();
		}
	}

	uint32_t getLongestSustain(uint32_t position) const
	{
		// If percussion is the only note at this position, then sustain should end up as 0
		uint32_t sustain = 0;
		for (const auto& track : m_vocals)
		{
			if (!track.empty())
			{
				try
				{
					const Vocal& vocal = VectorIteration::getIterator(track, position);
					if (vocal.getSustain() > sustain)
						sustain = vocal.getSustain();
				}
				catch (...) {}
			}
		}
		return sustain;
	}

public:
	VocalTrack(const char* name) : m_name(name) {}

	// Returns whether this track contains any notes
	// ONLY checks for notes
	bool hasNotes() const
	{
		for (const auto& track : m_vocals)
			if (track.size())
				return true;

		return m_percussion.size();
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
		VectorIteration::try_emplace(m_vocals[lane], position).setLyric(lyric);
	}

	void addPercussion(uint32_t position, bool playable)
	{
		VocalPercussion& perc = VectorIteration::try_emplace(m_percussion, position);
		if (!playable)
			perc.modify('N');
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
		try
		{
			VectorIteration::getIterator(m_vocals[lane], position).setPitch(pitch);
		}
		catch (...)
		{
			return false;
		}
	}

	void clear()
	{
		for (auto& track : m_vocals)
			track.clear();
		m_percussion.clear();
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
		if (numTracks == 1)
		{
			if (sscanf_s(buffer, "\tLyrics = %lu", &numNotes) == 1)
				m_vocals[0].reserve(numNotes);
		}
		else
		{
			if (sscanf_s(buffer, "\tHarm1 = %lu", &numNotes) == 1)
				m_vocals[0].reserve(numNotes);

			inFile.getline(buffer, 512);
			if (sscanf_s(buffer, "\tHarm2 = %lu", &numNotes) == 1)
				m_vocals[1].reserve(numNotes);

			inFile.getline(buffer, 512);
			if (sscanf_s(buffer, "\tHarm3 = %lu", &numNotes) == 1)
				m_vocals[2].reserve(numNotes);
		}

		inFile.getline(buffer, 512);
		if (sscanf_s(buffer, "\tPercussion = %lu", &numNotes) == 1)
			m_percussion.reserve(numNotes);
		
		struct
		{
			uint32_t position;
			bool active = false;
		} phrases[2];
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
					try
					{
						vocalize_cht(position, str);
					}
					catch (EndofLineException EoL)
					{
						std::cout << "Unable to parse full list of pitches at position " << position << " (\"" << str << "\")" << std::endl;
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
						if (phrases[index].active)
						{
							if (index == 0)
								addEffect(phrases[0].position, new LyricLine(position - phrases[0].position));
							else
								addEffect(phrases[1].position, new HarmonyLine(position - phrases[1].position));
						}
						phrases[index].position = position;
						phrases[index].active = true;
						break;
					// Phrase End
					case 'e':
					case 'E':
						if (index == 0)
							addEffect(phrases[0].position, new LyricLine(position - phrases[0].position));
						else
							addEffect(phrases[1].position, new HarmonyLine(position - phrases[1].position));
						phrases[index].active = false;
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
					init_cht_single(position, str);
					break;
				case 'c':
				case 'C':
					init_cht_chord(position, str);
					break;
				case 'm':
				case 'M':
					modify_cht(position, str);
					break;
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
							m_effects.back().second.push_back(new HarmonyLine(duration));
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

		for (auto& track : m_vocals)
			if (track.size() < track.capacity())
				track.shrink_to_fit();

		if (m_percussion.size() < m_percussion.capacity())
			m_percussion.shrink_to_fit();
	}

	void load_midi(int index, const unsigned char* currPtr, const unsigned char* const end)
	{
		uint32_t starPower = UINT32_MAX;
		uint32_t rangeShift = UINT32_MAX;
		uint32_t lyricShift = UINT32_MAX;
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
								addEffect(phrase, new LyricLine(position - phrase));
							break;
						case 1:
							if (syntax == 0x90 && velocity > 0)
								phrase = position;
							else
								addEffect(phrase, new HarmonyLine(position - phrase));
						}
						break;
						// Range Shift
					case 0:
						if (syntax == 0x90 && velocity > 0)
							rangeShift = position;
						else
							addEffect(rangeShift, new RangeShift(position - rangeShift));
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

	void save_cht(std::fstream& outFile) const;

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

	~VocalTrack()
	{
		for (auto& vec : m_effects)
			for (auto& eff : vec.second)
				delete eff;
	}
};

template<>
void VocalTrack<1>::save_cht(std::fstream& outFile) const;

template<>
void VocalTrack<3>::save_cht(std::fstream& outFile) const;
