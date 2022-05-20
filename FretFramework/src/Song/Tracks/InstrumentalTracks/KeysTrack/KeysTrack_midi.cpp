#include "../InstrumentalTrack.h"
#include "Chords\Keys.h"
#include "Song/Midi/MidiFile.h"
using namespace MidiFile;

template<>
void InstrumentalTrack<Keys<5>>::load_midi(const unsigned char* current, const unsigned char* const end)
{
	struct
	{
		uint32_t notes[5] = { UINT32_MAX };
		int numActive = 0;
		int numAdded = 0;
		uint32_t position = UINT32_MAX;
	} difficultyTracker[5];
	// Diff 5 = BRE

	for (auto& diff : m_difficulties)
		diff.m_notes.reserve(5000);

	uint32_t solo = UINT32_MAX;
	uint32_t starPower = UINT32_MAX;
	uint32_t trill = UINT32_MAX;
	bool doBRE = false;

	unsigned char syntax = 0xFF;
	uint32_t position = 0;
	while (current < end)
	{
		position += VariableLengthQuantity(current);
		unsigned char tmpSyntax = *current++;
		unsigned char note = 0;
		if (tmpSyntax & 0b10000000)
		{
			syntax = tmpSyntax;
			if (syntax == 0x80 || syntax == 0x90)
				note = *current++;
			else
			{
				if (syntax == 0xFF)
				{
					unsigned char type = *current++;
					VariableLengthQuantity length(current);
					if (type < 16)
					{
						if (m_difficulties[3].m_events.empty() || m_difficulties[3].m_events.back().first < position)
						{
							static std::pair<uint32_t, std::vector<std::string>> pairNode;
							pairNode.first = position;
							m_difficulties[3].m_events.push_back(pairNode);
						}

						m_difficulties[3].m_events.back().second.push_back(std::string((char*)current, length));
					}

					if (type == 0x2F)
						break;

					current += length;
				}
				else if (syntax == 0xF0 || syntax == 0xF7)
					current += VariableLengthQuantity(current);
				else
				{
					switch (syntax)
					{
					case 0xB0:
					case 0xA0:
					case 0xE0:
					case 0xF2:
						current += 2;
						break;
					case 0xC0:
					case 0xD0:
					case 0xF3:
						++current;
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
			unsigned char velocity = *current++;
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
			if (60 <= note && note < 100)
			{
				int noteValue = note - 60;
				int diff = noteValue / 12;
				int lane = noteValue % 12;

				if (lane < 5)
				{
					// 0 - Open
					// 1 - Green
					// 2 - Red
					// 3 - Yellow
					// 4 - Blue
					// 5 - Orange

					if (syntax == 0x90 && velocity > 0)
					{
						if (difficultyTracker[diff].position == UINT32_MAX || difficultyTracker[diff].position < position)
						{
							static std::pair<uint32_t, Keys<5>> pairNode;
							pairNode.first = position;
							m_difficulties[diff].m_notes.push_back(std::move(pairNode));

							difficultyTracker[diff].position = position;
							++difficultyTracker[diff].numAdded;
						}

						++difficultyTracker[diff].numActive;
						difficultyTracker[diff].notes[lane] = position;
					}
					else
					{
						m_difficulties[diff].addNoteFromMid(difficultyTracker[diff].notes[lane], lane + 1, difficultyTracker[diff].numAdded, position - difficultyTracker[diff].notes[lane]);
						--difficultyTracker[diff].numActive;
						if (difficultyTracker[diff].numActive == 0)
							difficultyTracker[diff].numAdded = 0;
					}
				}
			}
			// BRE
			else if (120 <= note && note <= 124)
			{
				int lane = note - 120;
				if (syntax == 0x90 && velocity > 0)
				{
					if (difficultyTracker[4].position == UINT32_MAX || difficultyTracker[4].position < position)
					{
						static std::pair<uint32_t, Keys<5>> pairNode;
						pairNode.first = position;
						m_difficulties[4].m_notes.push_back(pairNode);

						difficultyTracker[4].position = position;
						++difficultyTracker[4].numAdded;
					}

					++difficultyTracker[4].numActive;
					difficultyTracker[4].notes[lane + 1] = position;

					if (lane == 4)
					{
						int i = 1;
						while (i < 5 && difficultyTracker[4].notes[i] == position)
							++i;

						if (i == 5)
						{
							m_difficulties[4].m_notes.pop_back();
							doBRE = true;
						}
					}
				}
				else
				{
					--difficultyTracker[4].numActive;
					if (doBRE)
					{
						if (lane == 4)
						{
							m_difficulties[3].addPhrase(position, new StarPowerActivation(position - difficultyTracker[4].notes[lane + 1]));
							doBRE = false;
						}
					}
					else
					{
						m_difficulties[4].addNoteFromMid(difficultyTracker[4].notes[lane + 1], lane + 1, difficultyTracker[4].numAdded, position - difficultyTracker[4].notes[lane + 1]);
					}

					if (difficultyTracker[4].numActive == 0)
						difficultyTracker[4].numAdded = 0;
				}
			}
			else
			{
				switch (note)
				{
				// Star Power
				case 116:
					if (syntax == 0x90 && velocity > 0)
						starPower = position;
					else
						m_difficulties[3].addPhrase(starPower, new StarPowerPhrase(position - starPower));
					break;
				// Soloes
				case 103:
					if (syntax == 0x90 && velocity > 0)
						solo = position;
					else
						m_difficulties[3].addPhrase(solo, new Solo(position - solo));
					break;
				// Trill
				case 127:
					if (syntax == 0x90 && velocity > 0)
						trill = position;
					else
						m_difficulties[3].addPhrase(trill, new Trill(position - trill));
					break;
				}
			}
			break;
		}
		case 0xB0:
		case 0xA0:
		case 0xE0:
		case 0xF2:
			++current;
		}
	}

	for (auto& diff : m_difficulties)
		if (diff.m_notes.size() < diff.m_notes.capacity())
			diff.m_notes.shrink_to_fit();
}

template<>
void InstrumentalTrack<Keys<5>>::save_midi(const char* const name, std::fstream& outFile) const
{
	MidiFile::MidiChunk_Track events(name);
	for (const auto& vec : m_difficulties[3].m_events)
		for (const auto& ev : vec.second)
			events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MetaEvent_Text(1, ev));

	for (const auto& vec : m_difficulties[3].m_effects)
		for (const auto& effect : vec.second)
			if (effect->getMidiNote() != -1)
			{
				events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote()));
				events.addEvent(vec.first + effect->getDuration(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote(), 0));
			}
			else
				for (int lane = 120; lane < 125; ++lane)
				{
					events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, lane));
					events.addEvent(vec.first + effect->getDuration(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, lane, 0));
				}

	if (hasNotes())
	{
		auto processNote = [&](const std::pair<uint32_t, Keys<5>>& note, char baseMidiNote)
		{
			auto placeNote = [&](char midiNote, uint32_t sustain)
			{
				events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote));
				if (sustain == 0)
					events.addEvent(note.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote, 0));
				else
					events.addEvent(note.first + sustain, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote, 0));
			};

			for (char col = 0; col < 5; ++col)
				if (note.second.m_colors[col])
					placeNote(baseMidiNote + col + 1, note.second.m_colors[col].getSustain());
		};

		auto expertIter = m_difficulties[3].m_notes.begin();
		auto hardIter = m_difficulties[2].m_notes.begin();
		auto mediumIter = m_difficulties[1].m_notes.begin();
		auto easyIter = m_difficulties[0].m_notes.begin();
		bool expertValid = expertIter != m_difficulties[3].m_notes.end();
		bool hardValid = hardIter != m_difficulties[2].m_notes.end();
		bool mediumValid = mediumIter != m_difficulties[1].m_notes.end();
		bool easyValid = easyIter != m_difficulties[0].m_notes.end();

		while (expertValid || hardValid || mediumValid || easyValid)
		{
			while (expertValid &&
				(!hardValid || expertIter->first <= hardIter->first) &&
				(!mediumValid || expertIter->first <= mediumIter->first) &&
				(!easyValid || expertIter->first <= easyIter->first))
			{
				processNote(*expertIter, 96);
				expertValid = ++expertIter != m_difficulties[3].m_notes.end();
			}

			while (hardValid &&
				(!expertValid || hardIter->first < expertIter->first) &&
				(!mediumValid || hardIter->first <= mediumIter->first) &&
				(!easyValid || hardIter->first <= easyIter->first))
			{
				processNote(*hardIter, 84);
				hardValid = ++hardIter != m_difficulties[2].m_notes.end();
			}

			while (mediumValid &&
				(!expertValid || mediumIter->first < expertIter->first) &&
				(!hardValid || mediumIter->first < hardIter->first) &&
				(!easyValid || mediumIter->first <= easyIter->first))
			{
				processNote(*mediumIter, 72);
				mediumValid = ++mediumIter != m_difficulties[1].m_notes.end();
			}

			while (easyValid &&
				(!expertValid || easyIter->first < expertIter->first) &&
				(!hardValid || easyIter->first < hardIter->first) &&
				(!mediumValid || easyIter->first < mediumIter->first))
			{
				processNote(*easyIter, 60);
				easyValid = ++easyIter != m_difficulties[0].m_notes.end();
			}
		}
	}

	events.writeToFile(outFile);
}
