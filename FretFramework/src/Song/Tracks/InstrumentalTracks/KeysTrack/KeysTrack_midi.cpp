#include "../InstrumentalTrack.h"
#include "Chords\Keys.h"
#include "Song/Midi/MidiFile.h"
using namespace MidiFile;

template<>
void InstrumentalTrack_Scan<Keys<5>>::scan_midi(MidiTraversal& traversal)
{
	struct
	{
		bool activeNote = false;
		bool validated = false;
	} difficulties[4];

	while (traversal.scanNext() && m_scanValue != 15)
	{
		const unsigned char type = traversal.getEventType();

		if (type == 0x90 || type == 0x80)
		{
			const unsigned char note = traversal.getMidiNote();
			const unsigned char velocity = traversal.getVelocity();

			// Notes
			if (60 <= note && note < 100)
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
	}
}

template<>
void InstrumentalTrack<Keys<5>>::load_midi(MidiTraversal& traversal)
{
	struct
	{
		uint32_t notes[5] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
	} difficultyTracker[5];
	// Diff 5 = BRE

	uint32_t solo = UINT32_MAX;
	uint32_t starPower = UINT32_MAX;
	uint32_t trill = UINT32_MAX;
	bool doBRE = false;

	static constexpr Keys<5> noteNode;
	static constexpr std::vector<UnicodeString> eventNode;

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
				auto& tracker = difficultyTracker[diff];
				auto& difficulty = m_difficulties[diff];

				const int lane = laneValues[noteValue];

				if (lane < 5)
				{
					// 1 - Green
					// 2 - Red
					// 3 - Yellow
					// 4 - Blue
					// 5 - Orange
					uint32_t& colorPosition = tracker.notes[lane];

					if (type == 0x90 && velocity > 0)
					{
						if (difficulty.m_notes.empty() || difficulty.m_notes.back().first < position)
						{
							if (difficulty.m_notes.capacity() == 0)
								difficulty.m_notes.reserve(5000);

							difficulty.m_notes.push_back({ position, noteNode });
						}

						colorPosition = position;
					}
					else if (colorPosition != UINT32_MAX)
					{
						difficulty.setColor_linear(colorPosition, lane + 1, position - colorPosition);
						colorPosition = UINT32_MAX;
					}
				}
			}
			// BRE
			else if (120 <= note && note <= 124)
			{
				auto& tracker = difficultyTracker[4];
				auto& difficulty = m_difficulties[4];

				const int lane = note - 120;
				uint32_t& colorPosition = tracker.notes[lane];

				if (type == 0x90 && velocity > 0)
				{
					if (difficulty.m_notes.empty() || difficulty.m_notes.back().first < position)
						difficulty.m_notes.push_back({ position, noteNode });

					colorPosition = position;

					if (lane == 4)
					{
						int i = 0;
						while (i < 4 && tracker.notes[i] == position)
							++i;

						if (i == 4)
						{
							difficulty.m_notes.pop_back();
							doBRE = true;
						}
					}
				}
				else if (colorPosition != UINT32_MAX)
				{
					if (doBRE)
					{
						if (lane == 4)
						{
							m_difficulties[3].addPhrase(position, new StarPowerActivation(position - colorPosition));
							doBRE = false;
						}
					}
					else
						difficulty.setColor_linear(colorPosition, lane + 1, position - colorPosition);
					colorPosition = UINT32_MAX;
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
			else if (note == 103)
			{
				if (type == 0x90 && velocity > 0)
					solo = position;
				else if (solo != UINT32_MAX)
				{
					m_difficulties[3].addPhrase(solo, new Solo(position - solo));
					solo = UINT32_MAX;
				}
			}
			else if (note == 127)
			{
				if (type == 0x90 && velocity > 0)
					trill = position;
				else if (trill != UINT32_MAX)
				{
					m_difficulties[3].addPhrase(trill, new Trill(position - trill));
					trill = UINT32_MAX;
				}
			}
		}
		else if (type < 16)
		{
			if (m_difficulties[3].m_events.empty() || m_difficulties[3].m_events.back().first < position)
				m_difficulties[3].m_events.push_back({ position, eventNode });

			m_difficulties[3].m_events.back().second.emplace_back(std::move(traversal.getText()));
		}
	}

	for (auto& diff : m_difficulties)
		if ((diff.m_notes.size() < 500 || 10000 <= diff.m_notes.size()) && diff.m_notes.size() < diff.m_notes.capacity())
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
