#include "BasicTrack.h"
#include "Drums/DrumNote.h"
#include "Midi/MidiFile.h"
using namespace MidiFile;

template<>
void BasicTrack<DrumNote>::load_midi(const unsigned char* currPtr, const unsigned char* const end)
{
	struct
	{
		bool flam = false;
		uint32_t notes[6] = { UINT32_MAX };
		int numActive = 0;
		int numAdded = 0;
		uint32_t position = UINT32_MAX;
	} difficultyTracker[5];
	// Diff 5 = BRE

	uint32_t solo = UINT32_MAX;
	uint32_t starPower = UINT32_MAX;
	bool doBRE = false;
	uint32_t tremolo = UINT32_MAX;
	uint32_t trill = UINT32_MAX;
	bool toms[3] = { false };

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
						if (m_difficulties[3].m_events.empty() || m_difficulties[3].m_events.back().first < position)
						{
							static std::pair<uint32_t, std::vector<std::string>> pairNode;
							pairNode.first = position;
							m_difficulties[3].m_events.push_back(pairNode);
						}

						m_difficulties[3].m_events.back().second.emplace_back((char*)currPtr, length);
					}

					if (type == 0x2F)
						break;

					currPtr += length;
				}
				else if (syntax == 0xF0 || syntax == 0xF7)
				{
					VariableLengthQuantity length(currPtr);
					currPtr += length;
				}
				else
				{
					switch (syntax)
					{
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
			*	120 - 125 = fill/BRE
			*	126 = tremolo
			*	127 = trill
			*/

			// Expert+
			if (note == 95)
			{
				if (syntax == 0x90 && velocity > 0)
				{
					if (difficultyTracker[3].position == UINT32_MAX || difficultyTracker[3].position < position)
					{
						static std::pair<uint32_t, DrumNote> pairNode;
						pairNode.first = position;

						m_difficulties[3].m_notes.push_back(pairNode);
						difficultyTracker[3].position = position;
						++difficultyTracker[3].numAdded;
					}

					m_difficulties[3].m_notes.back().second.modifyColor(0, '+');

					++difficultyTracker[3].numActive;
					difficultyTracker[3].notes[0] = position;
				}
				else
				{
					m_difficulties[3].addNoteFromMid(difficultyTracker[3].notes[0], 0, difficultyTracker[3].numAdded, position - difficultyTracker[3].notes[0]);
					--difficultyTracker[3].numActive;
					if (difficultyTracker[3].numActive == 0)
						difficultyTracker[3].numAdded = 0;
				}
			}
			// Notes
			else if (60 <= note && note < 102)
			{
				int noteValue = note - 60;
				int diff = noteValue / 12;
				int lane = noteValue % 12;
				if (lane < 6)
				{
					if (syntax == 0x90 && velocity > 0)
					{
						if (difficultyTracker[diff].position == UINT32_MAX || difficultyTracker[diff].position < position)
						{
							static std::pair<uint32_t, DrumNote> pairNode;
							pairNode.first = position;
							m_difficulties[diff].m_notes.push_back(pairNode);

							difficultyTracker[diff].position = position;
							++difficultyTracker[diff].numAdded;
						}
						difficultyTracker[diff].notes[lane] = position;

						if (difficultyTracker[3].flam && diff == 0)
							m_difficulties[diff].m_notes.back().second.modify('F');

						if (2 <= lane && lane < 5 && !toms[lane - 2])
							m_difficulties[diff].m_notes.back().second.modifyColor(lane, 'C');

						if (velocity > 100)
							m_difficulties[diff].m_notes.back().second.modifyColor(lane, 'A');
						else if (velocity < 100)
							m_difficulties[diff].m_notes.back().second.modifyColor(lane, 'G');

						++difficultyTracker[diff].numActive;
						difficultyTracker[diff].notes[lane] = position;
					}
					else
					{
						m_difficulties[diff].addNoteFromMid(difficultyTracker[diff].notes[lane], lane, difficultyTracker[diff].numAdded, position - difficultyTracker[diff].notes[lane]);
						--difficultyTracker[diff].numActive;
						if (difficultyTracker[diff].numActive == 0)
							difficultyTracker[diff].numAdded = 0;
					}
				}
			}
			// Tom markers
			else if (110 <= note && note <= 112)
				toms[note - 110] = syntax == 0x90 && velocity > 0;
			// Fill/BRE
			else if (120 <= note && note <= 124)
			{
				int lane = note - 120;
				if (syntax == 0x90 && velocity > 0)
				{
					if (difficultyTracker[4].position == UINT32_MAX || difficultyTracker[4].position < position)
					{
						static std::pair<uint32_t, DrumNote> pairNode;
						pairNode.first = position;
						m_difficulties[4].m_notes.push_back(pairNode);

						difficultyTracker[4].position = position;
						++difficultyTracker[4].numAdded;
					}

					++difficultyTracker[4].numActive;
					difficultyTracker[4].notes[lane] = position;
					if (lane == 4)
					{
						int i = 0;
						while (i < 4 && difficultyTracker[4].notes[i] == position)
							++i;

						if (i == 4)
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
							m_difficulties[3].addPhrase(position, new StarPowerActivation(position - difficultyTracker[4].notes[lane]));
							doBRE = false;
						}
					}
					else
					{
						m_difficulties[4].addNoteFromMid(difficultyTracker[4].notes[lane], lane, difficultyTracker[4].numAdded, position - difficultyTracker[4].notes[lane]);
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
					// Flams
				case 109:
					difficultyTracker[3].flam = syntax == 0x90 && velocity > 0;
					if (difficultyTracker[3].flam && difficultyTracker[3].position == position)
						m_difficulties[3].m_notes.back().second.modify('F');
					break;
					// Tremolo (or single drum roll)
				case 126:
					if (syntax == 0x90 && velocity > 0)
						tremolo = position;
					else
						m_difficulties[3].addPhrase(tremolo, new Tremolo(position - tremolo));
					break;
					// Trill (or special drum roll)
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
			++currPtr;
			break;
		}
	}

	for (auto& diff : m_difficulties)
		if (diff.m_notes.size() < diff.m_notes.capacity())
			diff.m_notes.shrink_to_fit();
}

template<>
void BasicTrack<DrumNote>::save_midi(const char* const name, std::fstream& outFile) const
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

		bool useDynamics = false;
		auto processNote = [&](const std::pair<uint32_t, DrumNote>& note,
			char baseMidiNote,
			const std::pair<uint32_t, DrumNote>* const prev)
		{
			auto placeNote = [&](char midiNote, char velocity)
			{
				events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote, velocity));
				events.addEvent(note.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote, 0));
			};

			if (note.second.m_special)
			{
				if (note.second.m_special.m_isDoubleBass)
					placeNote(95, 100);
				else
					placeNote(baseMidiNote, 100);
			}

			for (char col = 0; col < 4; ++col)
				if (note.second.m_colors[col])
				{
					if (note.second.m_colors[col].m_isAccented)
					{
						// For now, use 127 as the default accent velocity
						placeNote(baseMidiNote + col + 1, 127);
						useDynamics = true;
					}
					else if (note.second.m_colors[col].m_isGhosted)
					{
						// For now, use 1 as the default accent velocity
						placeNote(baseMidiNote + col + 1, 1);
						useDynamics = true;
					}
					else
						placeNote(baseMidiNote + col + 1, 100);
				}

			if (note.second.m_fifthLane)
			{
				if (note.second.m_fifthLane.m_isAccented)
				{
					// For now, use 127 as the default accent velocity
					placeNote(baseMidiNote + 5, 127);
					useDynamics = true;
				}
				else if (note.second.m_fifthLane.m_isGhosted)
				{
					// For now, use 1 as the default accent velocity
					placeNote(baseMidiNote + 5, 1);
					useDynamics = true;
				}
				else
					placeNote(baseMidiNote + 5, 100);
			}
		};

		auto expertIter = m_difficulties[3].m_notes.begin();
		auto hardIter = m_difficulties[2].m_notes.begin();
		auto mediumIter = m_difficulties[1].m_notes.begin();
		auto easyIter = m_difficulties[0].m_notes.begin();
		bool expertValid = expertIter != m_difficulties[3].m_notes.end();
		bool hardValid = hardIter != m_difficulties[2].m_notes.end();
		bool mediumValid = mediumIter != m_difficulties[1].m_notes.end();
		bool easyValid = easyIter != m_difficulties[0].m_notes.end();

		uint32_t toms[3] = { UINT32_MAX, UINT32_MAX, UINT32_MAX };

		int adjustWithDifficulty = 3;
		if (!expertValid)
		{
			--adjustWithDifficulty;
			if (!hardValid)
			{
				--adjustWithDifficulty;
				if (!mediumValid)
					--adjustWithDifficulty;
			}
		}

		auto adjustToms = [&](const std::pair<uint32_t, DrumNote>& pair)
		{
			for (int lane = 0; lane < 3; ++lane)
			{
				if (pair.second.m_colors[1 + lane])
				{
					if (pair.second.m_colors[1 + lane].m_isCymbal)
					{
						// Ensure that toms are disabled
						if (toms[lane] != UINT32_MAX)
						{
							// Create Note off Event for the tom marker
							events.addEvent(toms[lane], new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 110 + lane, 0));
							toms[lane] = UINT32_MAX;
						}
					}
					else
					{
						// Ensure toms are enabled
						if (toms[lane] == UINT32_MAX)
							// Create Note On Event for the tom marker
							events.addEvent(pair.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 110 + lane));
						toms[lane] = pair.first + 1;
					}
				}
			}
		};

		while (expertValid || hardValid || mediumValid || easyValid)
		{
			while (expertValid &&
				(!hardValid || expertIter->first <= hardIter->first) &&
				(!mediumValid || expertIter->first <= mediumIter->first) &&
				(!easyValid || expertIter->first <= easyIter->first))
			{
				if (adjustWithDifficulty == 3)
					adjustToms(*expertIter);

				if (expertIter != m_difficulties[3].m_notes.begin())
					processNote(*expertIter, 96, (expertIter - 1)._Ptr);
				else
					processNote(*expertIter, 96, nullptr);

				if (adjustWithDifficulty == 3 && expertIter->second.m_isFlamed)
				{
					events.addEvent(expertIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109));
					events.addEvent(expertIter->first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109, 0));
				}

				expertValid = ++expertIter != m_difficulties[3].m_notes.end();
			}

			while (hardValid &&
				(!expertValid || hardIter->first < expertIter->first) &&
				(!mediumValid || hardIter->first <= mediumIter->first) &&
				(!easyValid || hardIter->first <= easyIter->first))
			{
				if (adjustWithDifficulty == 2)
					adjustToms(*hardIter);

				if (hardIter != m_difficulties[2].m_notes.begin())
					processNote(*hardIter, 84, (hardIter - 1)._Ptr);
				else
					processNote(*hardIter, 84, nullptr);

				if (adjustWithDifficulty == 2 && hardIter->second.m_isFlamed)
				{
					events.addEvent(hardIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109));
					events.addEvent(hardIter->first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109, 0));
				}

				hardValid = ++hardIter != m_difficulties[2].m_notes.end();
			}

			while (mediumValid &&
				(!expertValid || mediumIter->first < expertIter->first) &&
				(!hardValid || mediumIter->first < hardIter->first) &&
				(!easyValid || mediumIter->first <= easyIter->first))
			{
				if (adjustWithDifficulty == 1)
					adjustToms(*mediumIter);

				if (mediumIter != m_difficulties[1].m_notes.begin())
					processNote(*mediumIter, 72, (mediumIter - 1)._Ptr);
				else
					processNote(*mediumIter, 72, nullptr);

				if (adjustWithDifficulty == 1 && mediumIter->second.m_isFlamed)
				{
					events.addEvent(mediumIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109));
					events.addEvent(mediumIter->first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109, 0));
				}
				mediumValid = ++mediumIter != m_difficulties[1].m_notes.end();
			}

			while (easyValid &&
				(!expertValid || easyIter->first < expertIter->first) &&
				(!hardValid || easyIter->first < hardIter->first) &&
				(!mediumValid || easyIter->first < mediumIter->first))
			{
				if (adjustWithDifficulty == 0)
					adjustToms(*easyIter);

				if (easyIter != m_difficulties[0].m_notes.begin())
					processNote(*easyIter, 60, (easyIter - 1)._Ptr);
				else
					processNote(*easyIter, 60, nullptr);

				if (adjustWithDifficulty == 0 && easyIter->second.m_isFlamed)
				{
					events.addEvent(easyIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109));
					events.addEvent(easyIter->first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109, 0));
				}
				easyValid = ++easyIter != m_difficulties[0].m_notes.end();
			}
		}

		// Disable any active tom markers
		for (int lane = 0; lane < 3; ++lane)
			if (toms[lane] != UINT32_MAX)
			{
				// Create Note off Event for the tom marker
				events.addEvent(toms[lane], new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 110 + lane, 0));
				toms[lane] = UINT32_MAX;
			}

		if (useDynamics)
			events.addEvent(0, new MidiFile::MidiChunk_Track::MetaEvent_Text(1, "[ENABLE_CHART_DYNAMICS]"));
	}

	events.writeToFile(outFile);
}
