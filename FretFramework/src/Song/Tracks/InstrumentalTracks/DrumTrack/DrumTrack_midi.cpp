#include "DrumTrack_Legacy.h"
#include "Drums/DrumNote_bch.hpp"
#include "Drums/DrumNote_cht.hpp"
#include "Song/Midi/MidiFile.h"
using namespace MidiFile;

template<>
void InstrumentalTrack_Scan<DrumNote<4, DrumPad_Pro>>::scan_midi(MidiTraversal& traversal)
{
	struct
	{
		bool activeNote = false;
		bool validated = false;
	} difficulties[4];

	while (traversal.next() && m_scanValue != 15)
	{
		const unsigned char type = traversal.getEventType();

		if (type == 0x90 || type == 0x80)
		{
			const unsigned char note = traversal.getMidiNote();
			const unsigned char velocity = traversal.getVelocity();

			if (note == 95)
			{
				if (m_scanValue < 8)
				{
					if (type == 0x90 && velocity > 0)
						difficulties[3].activeNote = true;
					else if (difficulties[3].activeNote)
					{
						difficulties[3].validated = true;
						m_scanValue |= 8;
					}
				}
			}
			// Notes
			else if (60 <= note && note <= 100)
			{
				int noteValue = note - 60;
				int diff = noteValue / 12;

				if (!difficulties[diff].validated)
				{
					if (noteValue % 12 < 5)
					{
						if (type == 0x90 && velocity > 0)
							difficulties[diff].activeNote = true;
						else if (difficulties[diff].activeNote)
						{
							difficulties[diff].validated = true;
							m_scanValue |= 1 << diff;
						}
					}
				}
			}
		}
		else if (type == 0x2F)
			break;
	}
}

template<>
void InstrumentalTrack<DrumNote<4, DrumPad_Pro>>::load_midi(MidiTraversal& traversal)
{
	struct
	{
		bool flam = false;
		uint32_t notes[5] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
		int numActive = 0;
		int numAdded = 0;
	} difficultyTracker[5];
	// Diff 5 = BRE

	uint32_t solo = UINT32_MAX;
	uint32_t starPower = UINT32_MAX;
	bool doBRE = false;
	uint32_t tremolo = UINT32_MAX;
	uint32_t trill = UINT32_MAX;
	bool toms[3] = { false };
	bool enableDynamics = false;

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
			*	120 - 125 = fill/BRE
			*	126 = tremolo
			*	127 = trill
			*/

			static constexpr int diffValues[48] =
			{
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
				3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
			};

			static constexpr int laneValues[48] =
			{
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
			};

			// Notes
			if (60 <= note && note <= 100)
			{
				const int noteValue = note - 60;
				const int diff = diffValues[noteValue];
				const int lane = laneValues[noteValue];

				if (lane < 5)
				{
					if (type == 0x90 && velocity > 0)
					{
						if (m_difficulties[diff].m_notes.empty() || m_difficulties[diff].m_notes.back().first < position)
						{
							if (m_difficulties[diff].m_notes.capacity() == 0)
								m_difficulties[diff].m_notes.reserve(5000);

							static std::pair<uint32_t, DrumNote<4, DrumPad_Pro>> pairNode;
							pairNode.first = position;
							m_difficulties[diff].m_notes.push_back(pairNode);

							++difficultyTracker[diff].numAdded;
						}
						difficultyTracker[diff].notes[lane] = position;

						if (difficultyTracker[3].flam && diff == 0)
							m_difficulties[diff].m_notes.back().second.modify('F');

						if (2 <= lane && lane < 5 && !toms[lane - 2])
							m_difficulties[diff].m_notes.back().second.modify('C', lane);

						if (enableDynamics)
						{
							if (velocity > 100)
								m_difficulties[diff].m_notes.back().second.modify('A', lane);
							else if (velocity < 100)
								m_difficulties[diff].m_notes.back().second.modify('G', lane);
						}

						++difficultyTracker[diff].numActive;
						difficultyTracker[diff].notes[lane] = position;
					}
					else if (difficultyTracker[diff].notes[lane] != UINT32_MAX)
					{
						m_difficulties[diff].addNoteFromMid(difficultyTracker[diff].notes[lane], lane, difficultyTracker[diff].numAdded, position - difficultyTracker[diff].notes[lane]);
						difficultyTracker[diff].notes[lane] = UINT32_MAX;
						--difficultyTracker[diff].numActive;
						if (difficultyTracker[diff].numActive == 0)
							difficultyTracker[diff].numAdded = 0;
					}
				}
				else if (note == 95)
				{
					if (type == 0x90 && velocity > 0)
					{
						if (m_difficulties[3].m_notes.empty() || m_difficulties[3].m_notes.back().first < position)
						{
							if (m_difficulties[3].m_notes.capacity() == 0)
								m_difficulties[3].m_notes.reserve(5000);

							static std::pair<uint32_t, DrumNote<4, DrumPad_Pro>> pairNode;
							pairNode.first = position;

							m_difficulties[3].m_notes.push_back(pairNode);
							++difficultyTracker[3].numAdded;
						}

						m_difficulties[3].m_notes.back().second.modify('+', 0);

						++difficultyTracker[3].numActive;
						difficultyTracker[3].notes[0] = position;
					}
					else if (difficultyTracker[3].notes[0] != UINT32_MAX)
					{
						m_difficulties[3].addNoteFromMid(difficultyTracker[3].notes[0], 0, difficultyTracker[3].numAdded, position - difficultyTracker[3].notes[0]);
						difficultyTracker[3].notes[0] = UINT32_MAX;
						--difficultyTracker[3].numActive;
						if (difficultyTracker[3].numActive == 0)
							difficultyTracker[3].numAdded = 0;
					}
				}
			}
			// Tom markers
			else if (110 <= note && note <= 112)
				toms[note - 110] = type == 0x90 && velocity > 0;
			// Fill/BRE
			else if (120 <= note && note <= 124)
			{
				int lane = note - 120;
				if (type == 0x90 && velocity > 0)
				{
					if (m_difficulties[4].m_notes.empty() || m_difficulties[4].m_notes.back().first < position)
					{
						if (m_difficulties[4].m_notes.capacity() == 0)
							m_difficulties[4].m_notes.reserve(5000);

						static std::pair<uint32_t, DrumNote<4, DrumPad_Pro>> pairNode;
						pairNode.first = position;
						m_difficulties[4].m_notes.push_back(pairNode);

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
				else if (difficultyTracker[4].notes[lane] != UINT32_MAX)
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
						m_difficulties[4].addNoteFromMid(difficultyTracker[4].notes[lane], lane, difficultyTracker[4].numAdded, position - difficultyTracker[4].notes[lane]);
					difficultyTracker[4].notes[lane] = UINT32_MAX;

					if (difficultyTracker[4].numActive == 0)
						difficultyTracker[4].numAdded = 0;
				}
			}
			// Star Power
			else if (note == s_starPowerReadNote)
			{
				if (type == 0x90 && velocity > 0)
					starPower = position;
				else if (starPower != UINT32_MAX)
				{
					m_difficulties[3].addPhrase(starPower, new StarPowerPhrase(position - starPower));
					starPower = UINT32_MAX;
				}
			}
			else
			{
				switch (note)
				{
					// Soloes
				case 103:
					if (type == 0x90 && velocity > 0)
						solo = position;
					else if (solo != UINT32_MAX)
					{
						m_difficulties[3].addPhrase(solo, new Solo(position - solo));
						solo = UINT32_MAX;
					}
					break;
					// Flams
				case 109:
					difficultyTracker[3].flam = type == 0x90 && velocity > 0;
					if (difficultyTracker[3].flam && m_difficulties[3].m_notes.back().first == position)
						m_difficulties[3].m_notes.back().second.modify('F');
					break;
					// Tremolo (or single drum roll)
				case 126:
					if (type == 0x90 && velocity > 0)
						tremolo = position;
					else if (tremolo != UINT32_MAX)
					{
						m_difficulties[3].addPhrase(tremolo, new Tremolo(position - tremolo));
						tremolo = UINT32_MAX;
					}
					break;
					// Trill (or special drum roll)
				case 127:
					if (type == 0x90 && velocity > 0)
						trill = position;
					else if (trill != UINT32_MAX)
					{
						m_difficulties[3].addPhrase(trill, new Trill(position - trill));
						trill = UINT32_MAX;
					}
					break;
				}
			}
		}
		else if (type < 16)
		{
			const std::string str = traversal.extractText();
			if (str == "[ENABLE_CHART_DYNAMICS]")
			{
				if (m_difficulties[3].m_events.empty() || m_difficulties[3].m_events.back().first < position)
				{
					static std::pair<uint32_t, std::vector<UnicodeString>> pairNode;
					pairNode.first = position;
					m_difficulties[3].m_events.push_back(pairNode);
				}

				m_difficulties[3].m_events.back().second.push_back(str);
			}
			else
				enableDynamics = true;
		}
		else if (type == 0x2F)
			break;
	}

	for (auto& diff : m_difficulties)
		if ((diff.m_notes.size() < 500 || 10000 <= diff.m_notes.size()) && diff.m_notes.size() < diff.m_notes.capacity())
			diff.m_notes.shrink_to_fit();
}

template<>
void InstrumentalTrack<DrumNote<4, DrumPad_Pro>>::save_midi(const char* const name, std::fstream& outFile) const
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
		auto processNote = [&](const std::pair<uint32_t, DrumNote<4, DrumPad_Pro>>& note, char baseMidiNote)
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

			for (char col = 0; col < 5; ++col)
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

		auto adjustToms = [&](const std::pair<uint32_t, DrumNote<4, DrumPad_Pro>>& pair)
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
							// Create InstrumentalNote off Event for the tom marker
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

				processNote(*expertIter, 96);

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

				processNote(*hardIter, 84);

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

				processNote(*mediumIter, 72);

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

				processNote(*easyIter, 60);

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
				// Create InstrumentalNote off Event for the tom marker
				events.addEvent(toms[lane], new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 110 + lane, 0));
				toms[lane] = UINT32_MAX;
			}

		if (useDynamics)
			events.addEvent(0, new MidiFile::MidiChunk_Track::MetaEvent_Text(1, "[ENABLE_CHART_DYNAMICS]"));
	}

	events.writeToFile(outFile);
}

template<>
void InstrumentalTrack_Scan<DrumNote<5, DrumPad>>::scan_midi(MidiTraversal& traversal)
{
	struct
	{
		bool activeNote = false;
		bool validated = false;
	} difficulties[4];

	while (traversal.next() && m_scanValue != 15)
	{
		const unsigned char type = traversal.getEventType();

		if (type == 0x90 || type == 0x80)
		{
			const unsigned char note = traversal.getMidiNote();
			const unsigned char velocity = traversal.getVelocity();

			if (note == 95)
			{
				if (m_scanValue < 8)
				{
					if (type == 0x90 && velocity > 0)
						difficulties[3].activeNote = true;
					else if (difficulties[3].activeNote)
					{
						difficulties[3].validated = true;
						m_scanValue |= 8;
					}
				}
			}
			// Notes
			else if (60 <= note && note <= 101)
			{
				int noteValue = note - 60;
				int diff = noteValue / 12;

				if (!difficulties[diff].validated)
				{
					if (noteValue % 12 < 6)
					{
						if (type == 0x90 && velocity > 0)
							difficulties[diff].activeNote = true;
						else if (difficulties[diff].activeNote)
						{
							difficulties[diff].validated = true;
							m_scanValue |= 1 << diff;
						}
					}
				}
			}
		}
		else if (type == 0x2F)
			break;
	}
}

template<>
void InstrumentalTrack<DrumNote<5, DrumPad>>::load_midi(MidiTraversal& traversal)
{
	struct
	{
		bool flam = false;
		uint32_t notes[6] = { UINT32_MAX };
		int numActive = 0;
		int numAdded = 0;
	} difficultyTracker[5];
	// Diff 5 = BRE

	uint32_t solo = UINT32_MAX;
	uint32_t starPower = UINT32_MAX;
	bool doBRE = false;
	uint32_t tremolo = UINT32_MAX;
	uint32_t trill = UINT32_MAX;
	bool enableDynamics = false;

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
			*	120 - 125 = fill/BRE
			*	126 = tremolo
			*	127 = trill
			*/

			static constexpr int diffValues[48] =
			{
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
				3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
			};

			static constexpr int laneValues[48] =
			{
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
			};

			// Notes
			if (60 <= note && note <= 101)
			{
				const int noteValue = note - 60;
				const int diff = diffValues[noteValue];
				const int lane = laneValues[noteValue];

				if (lane < 6)
				{
					if (type == 0x90 && velocity > 0)
					{
						if (m_difficulties[diff].m_notes.empty() || m_difficulties[diff].m_notes.back().first < position)
						{
							if (m_difficulties[diff].m_notes.capacity() == 0)
								m_difficulties[diff].m_notes.reserve(5000);

							static std::pair<uint32_t, DrumNote<5, DrumPad>> pairNode;
							pairNode.first = position;
							m_difficulties[diff].m_notes.push_back(pairNode);

							++difficultyTracker[diff].numAdded;
						}
						difficultyTracker[diff].notes[lane] = position;

						if (difficultyTracker[3].flam && diff == 0)
							m_difficulties[diff].m_notes.back().second.modify('F');

						if (enableDynamics)
						{
							if (velocity > 100)
								m_difficulties[diff].m_notes.back().second.modify('A', lane);
							else if (velocity < 100)
								m_difficulties[diff].m_notes.back().second.modify('G', lane);
						}

						++difficultyTracker[diff].numActive;
						difficultyTracker[diff].notes[lane] = position;
					}
					else if (difficultyTracker[diff].notes[lane] != UINT32_MAX)
					{
						m_difficulties[diff].addNoteFromMid(difficultyTracker[diff].notes[lane], lane, difficultyTracker[diff].numAdded, position - difficultyTracker[diff].notes[lane]);
						difficultyTracker[diff].notes[lane] = UINT32_MAX;
						--difficultyTracker[diff].numActive;

						if (difficultyTracker[diff].numActive == 0)
							difficultyTracker[diff].numAdded = 0;
					}
				}
				else if (note == 95)
				{
					if (type == 0x90 && velocity > 0)
					{
						if (m_difficulties[3].m_notes.empty() || m_difficulties[3].m_notes.back().first < position)
						{
							if (m_difficulties[3].m_notes.capacity() == 0)
								m_difficulties[3].m_notes.reserve(5000);

							static std::pair<uint32_t, DrumNote<5, DrumPad>> pairNode;
							pairNode.first = position;

							m_difficulties[3].m_notes.push_back(pairNode);
							++difficultyTracker[3].numAdded;
						}

						m_difficulties[3].m_notes.back().second.modify('+', 0);

						++difficultyTracker[3].numActive;
						difficultyTracker[3].notes[0] = position;
					}
					else if (difficultyTracker[3].notes[0] != UINT32_MAX)
					{
						m_difficulties[3].addNoteFromMid(difficultyTracker[3].notes[0], 0, difficultyTracker[3].numAdded, position - difficultyTracker[3].notes[0]);
						difficultyTracker[3].notes[0] = UINT32_MAX;
						--difficultyTracker[3].numActive;
						if (difficultyTracker[3].numActive == 0)
							difficultyTracker[3].numAdded = 0;
					}
				}
			}
			// Fill/BRE
			else if (120 <= note && note <= 124)
			{
				int lane = note - 120;
				if (type == 0x90 && velocity > 0)
				{
					if (m_difficulties[4].m_notes.empty() || m_difficulties[4].m_notes.back().first < position)
					{
						if (m_difficulties[4].m_notes.capacity() == 0)
							m_difficulties[4].m_notes.reserve(5000);

						static std::pair<uint32_t, DrumNote<5, DrumPad>> pairNode;
						pairNode.first = position;
						m_difficulties[4].m_notes.push_back(pairNode);

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
				else if (difficultyTracker[4].notes[lane] != UINT32_MAX)
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
						m_difficulties[4].addNoteFromMid(difficultyTracker[4].notes[lane], lane, difficultyTracker[4].numAdded, position - difficultyTracker[4].notes[lane]);
					difficultyTracker[4].notes[lane] = UINT32_MAX;

					if (difficultyTracker[4].numActive == 0)
						difficultyTracker[4].numAdded = 0;
				}
			}
			// Star Power
			else if (note == s_starPowerReadNote)
			{
				if (type == 0x90 && velocity > 0)
					starPower = position;
				else if (starPower != UINT32_MAX)
				{
					m_difficulties[3].addPhrase(starPower, new StarPowerPhrase(position - starPower));
					starPower = UINT32_MAX;
				}
			}
			else
			{
				switch (note)
				{
					// Soloes
				case 103:
					if (type == 0x90 && velocity > 0)
						solo = position;
					else if (solo != UINT32_MAX)
					{
						m_difficulties[3].addPhrase(solo, new Solo(position - solo));
						solo = UINT32_MAX;
					}
					break;
					// Flams
				case 109:
					difficultyTracker[3].flam = type == 0x90 && velocity > 0;
					if (difficultyTracker[3].flam && m_difficulties[3].m_notes.back().first == position)
						m_difficulties[3].m_notes.back().second.modify('F');
					break;
					// Tremolo (or single drum roll)
				case 126:
					if (type == 0x90 && velocity > 0)
						tremolo = position;
					else if (tremolo != UINT32_MAX)
					{
						m_difficulties[3].addPhrase(tremolo, new Tremolo(position - tremolo));
						tremolo = UINT32_MAX;
					}
					break;
					// Trill (or special drum roll)
				case 127:
					if (type == 0x90 && velocity > 0)
						trill = position;
					else if (trill != UINT32_MAX)
					{
						m_difficulties[3].addPhrase(trill, new Trill(position - trill));
						trill = UINT32_MAX;
					}
					break;
				}
			}
		}
		else if (type < 16)
		{
			const std::string str = traversal.extractText();
			if (str == "[ENABLE_CHART_DYNAMICS]")
			{
				if (m_difficulties[3].m_events.empty() || m_difficulties[3].m_events.back().first < position)
				{
					static std::pair<uint32_t, std::vector<UnicodeString>> pairNode;
					pairNode.first = position;
					m_difficulties[3].m_events.push_back(pairNode);
				}

				m_difficulties[3].m_events.back().second.push_back(str);
			}
			else
				enableDynamics = true;
		}
		else if (type == 0x2F)
			break;
	}

	for (auto& diff : m_difficulties)
		if ((diff.m_notes.size() < 500 || 10000 <= diff.m_notes.size()) && diff.m_notes.size() < diff.m_notes.capacity())
			diff.m_notes.shrink_to_fit();
}

template<>
void InstrumentalTrack<DrumNote<5, DrumPad>>::save_midi(const char* const name, std::fstream& outFile) const
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
		auto processNote = [&](const std::pair<uint32_t, DrumNote<5, DrumPad>>& note, char baseMidiNote)
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

			for (char col = 0; col < 5; ++col)
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
		};

		auto expertIter = m_difficulties[3].m_notes.begin();
		auto hardIter = m_difficulties[2].m_notes.begin();
		auto mediumIter = m_difficulties[1].m_notes.begin();
		auto easyIter = m_difficulties[0].m_notes.begin();
		bool expertValid = expertIter != m_difficulties[3].m_notes.end();
		bool hardValid = hardIter != m_difficulties[2].m_notes.end();
		bool mediumValid = mediumIter != m_difficulties[1].m_notes.end();
		bool easyValid = easyIter != m_difficulties[0].m_notes.end();

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

		while (expertValid || hardValid || mediumValid || easyValid)
		{
			while (expertValid &&
				(!hardValid || expertIter->first <= hardIter->first) &&
				(!mediumValid || expertIter->first <= mediumIter->first) &&
				(!easyValid || expertIter->first <= easyIter->first))
			{
				processNote(*expertIter, 96);

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
				processNote(*hardIter, 84);

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
				processNote(*mediumIter, 72);

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
				processNote(*easyIter, 60);

				if (adjustWithDifficulty == 0 && easyIter->second.m_isFlamed)
				{
					events.addEvent(easyIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109));
					events.addEvent(easyIter->first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109, 0));
				}
				easyValid = ++easyIter != m_difficulties[0].m_notes.end();
			}
		}

		if (useDynamics)
			events.addEvent(0, new MidiFile::MidiChunk_Track::MetaEvent_Text(1, "[ENABLE_CHART_DYNAMICS]"));
	}

	events.writeToFile(outFile);
}

void InstrumentalTrack_Scan<DrumNote_Legacy>::scan_midi(MidiTraversal& traversal)
{
	struct
	{
		bool activeNote = false;
		bool validated = false;
	} difficulties[4];

	while (traversal.next() && (m_scanValue != 15 || !m_isFiveLane))
	{
		const unsigned char type = traversal.getEventType();

		if (type == 0x90 || type == 0x80)
		{
			const unsigned char note = traversal.getMidiNote();
			const unsigned char velocity = traversal.getVelocity();

			if (note == 95)
			{
				if (m_scanValue < 8)
				{
					if (type == 0x90 && velocity > 0)
						difficulties[3].activeNote = true;
					else if (difficulties[3].activeNote)
					{
						difficulties[3].validated = true;
						m_scanValue |= 8;
					}
				}
			}
			// Notes
			else if (60 <= note && note <= 101)
			{
				int noteValue = note - 60;
				int diff = noteValue / 12;
				int lane = noteValue % 12;

				if (lane == 5)
					m_isFiveLane = true;

				if (!difficulties[diff].validated)
				{
					if (lane < 6)
					{
						if (type == 0x90 && velocity > 0)
							difficulties[diff].activeNote = true;
						else if (difficulties[diff].activeNote)
						{
							difficulties[diff].validated = true;
							m_scanValue |= 1 << diff;
						}
					}
				}
			}
		}
		else if (type == 0x2F)
			break;
	}
}

void InstrumentalTrack<DrumNote_Legacy>::load_midi(MidiTraversal& traversal)
{
	struct
	{
		bool flam = false;
		uint32_t notes[6] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
		int numActive = 0;
		int numAdded = 0;
	} difficultyTracker[5];
	// Diff 5 = BRE

	uint32_t solo = UINT32_MAX;
	uint32_t starPower = UINT32_MAX;
	bool doBRE = false;
	uint32_t tremolo = UINT32_MAX;
	uint32_t trill = UINT32_MAX;
	bool toms[3] = { false };
	bool enableDynamics = false;

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
			*	120 - 125 = fill/BRE
			*	126 = tremolo
			*	127 = trill
			*/

			static constexpr int diffValues[48] =
			{
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
				3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
			};

			static constexpr int laneValues[48] =
			{
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
			};

			// Notes
			if (60 <= note && note <= 101)
			{
				const int noteValue = note - 60;
				const int diff = diffValues[noteValue];
				const int lane = laneValues[noteValue];

				if (lane < 6)
				{
					if (type == 0x90 && velocity > 0)
					{
						if (m_difficulties[diff].m_notes.empty() || m_difficulties[diff].m_notes.back().first < position)
						{
							if (m_difficulties[diff].m_notes.capacity() == 0)
								m_difficulties[diff].m_notes.reserve(5000);

							static std::pair<uint32_t, DrumNote_Legacy> pairNode;
							pairNode.first = position;
							m_difficulties[diff].m_notes.push_back(pairNode);

							++difficultyTracker[diff].numAdded;
						}
						difficultyTracker[diff].notes[lane] = position;

						if (difficultyTracker[3].flam && diff == 0)
							m_difficulties[diff].m_notes.back().second.modify('F');

						if (2 <= lane && lane < 5 && !toms[lane - 2])
							m_difficulties[diff].m_notes.back().second.modify('C', lane);

						if (enableDynamics)
						{
							if (velocity > 100)
								m_difficulties[diff].m_notes.back().second.modify('A', lane);
							else if (velocity < 100)
								m_difficulties[diff].m_notes.back().second.modify('G', lane);
						}

						++difficultyTracker[diff].numActive;
						difficultyTracker[diff].notes[lane] = position;
					}
					else if (difficultyTracker[diff].notes[lane] != UINT32_MAX)
					{
						m_difficulties[diff].addNoteFromMid(difficultyTracker[diff].notes[lane], lane, difficultyTracker[diff].numAdded, position - difficultyTracker[diff].notes[lane]);
						difficultyTracker[diff].notes[lane] = UINT32_MAX;
						--difficultyTracker[diff].numActive;

						if (difficultyTracker[diff].numActive == 0)
							difficultyTracker[diff].numAdded = 0;
					}
				}
				else if (note == 95)
				{
					if (type == 0x90 && velocity > 0)
					{
						if (m_difficulties[3].m_notes.empty() || m_difficulties[3].m_notes.back().first < position)
						{
							if (m_difficulties[3].m_notes.capacity() == 0)
								m_difficulties[3].m_notes.reserve(5000);

							static std::pair<uint32_t, DrumNote_Legacy> pairNode;
							pairNode.first = position;

							m_difficulties[3].m_notes.push_back(pairNode);
							++difficultyTracker[3].numAdded;
						}

						m_difficulties[3].m_notes.back().second.modify('+', 0);

						++difficultyTracker[3].numActive;
						difficultyTracker[3].notes[0] = position;
					}
					else if (difficultyTracker[3].notes[0] != UINT32_MAX)
					{
						m_difficulties[3].addNoteFromMid(difficultyTracker[3].notes[0], 0, difficultyTracker[3].numAdded, position - difficultyTracker[3].notes[0]);
						difficultyTracker[3].notes[0] = UINT32_MAX;
						--difficultyTracker[3].numActive;

						if (difficultyTracker[3].numActive == 0)
							difficultyTracker[3].numAdded = 0;
					}
				}
			}
			// Tom markers
			else if (110 <= note && note <= 112)
				toms[note - 110] = type == 0x90 && velocity > 0;
			// Fill/BRE
			else if (120 <= note && note <= 124)
			{
				int lane = note - 120;
				if (type == 0x90 && velocity > 0)
				{
					if (m_difficulties[4].m_notes.empty() || m_difficulties[4].m_notes.back().first < position)
					{
						if (m_difficulties[4].m_notes.capacity() == 0)
							m_difficulties[4].m_notes.reserve(5000);

						static std::pair<uint32_t, DrumNote_Legacy> pairNode;
						pairNode.first = position;
						m_difficulties[4].m_notes.push_back(pairNode);

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
				else if (difficultyTracker[4].notes[lane] != UINT32_MAX)
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
						m_difficulties[4].addNoteFromMid(difficultyTracker[4].notes[lane], lane, difficultyTracker[4].numAdded, position - difficultyTracker[4].notes[lane]);
					difficultyTracker[4].notes[lane] = UINT32_MAX;

					if (difficultyTracker[4].numActive == 0)
						difficultyTracker[4].numAdded = 0;
				}
			}
			// Star Power
			else if (note == s_starPowerReadNote)
			{
				if (type == 0x90 && velocity > 0)
					starPower = position;
				else if (starPower != UINT32_MAX)
				{
					m_difficulties[3].addPhrase(starPower, new StarPowerPhrase(position - starPower));
					starPower = UINT32_MAX;
				}
			}
			else
			{
				switch (note)
				{
					// Soloes
				case 103:
					if (type == 0x90 && velocity > 0)
						solo = position;
					else if (solo != UINT32_MAX)
					{
						m_difficulties[3].addPhrase(solo, new Solo(position - solo));
						solo = UINT32_MAX;
					}
					break;
					// Flams
				case 109:
					difficultyTracker[3].flam = type == 0x90 && velocity > 0;
					if (difficultyTracker[3].flam && m_difficulties[3].m_notes.back().first == position)
						m_difficulties[3].m_notes.back().second.modify('F');
					break;
					// Tremolo (or single drum roll)
				case 126:
					if (type == 0x90 && velocity > 0)
						tremolo = position;
					else if (tremolo != UINT32_MAX)
					{
						m_difficulties[3].addPhrase(tremolo, new Tremolo(position - tremolo));
						tremolo = UINT32_MAX;
					}
					break;
					// Trill (or special drum roll)
				case 127:
					if (type == 0x90 && velocity > 0)
						trill = position;
					else if (trill != UINT32_MAX)
					{
						m_difficulties[3].addPhrase(trill, new Trill(position - trill));
						trill = UINT32_MAX;
					}
					break;
				}
			}
		}
		else if (type < 16)
		{
			const std::string str = traversal.extractText();
			if (str == "[ENABLE_CHART_DYNAMICS]")
			{
				if (m_difficulties[3].m_events.empty() || m_difficulties[3].m_events.back().first < position)
				{
					static std::pair<uint32_t, std::vector<UnicodeString>> pairNode;
					pairNode.first = position;
					m_difficulties[3].m_events.push_back(pairNode);
				}

				m_difficulties[3].m_events.back().second.push_back(str);
			}
			else
				enableDynamics = true;
		}
		else if (type == 0x2F)
			break;
	}

	for (auto& diff : m_difficulties)
		if ((diff.m_notes.size() < 500 || 10000 <= diff.m_notes.size()) && diff.m_notes.size() < diff.m_notes.capacity())
			diff.m_notes.shrink_to_fit();
}
