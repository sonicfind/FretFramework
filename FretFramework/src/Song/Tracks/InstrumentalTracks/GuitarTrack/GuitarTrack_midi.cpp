#include "../InstrumentalTrack.h"
#include "Chords\GuitarNote\GuitarNote_bch.hpp"
#include "Chords\GuitarNote\GuitarNote_cht.hpp"
#include "Song/Midi/MidiFile.h"
using namespace MidiFile;

template<>
void InstrumentalTrack<GuitarNote<5>>::load_midi(const unsigned char* current, const unsigned char* const end)
{
	struct
	{
		bool greenToOpen = false;
		bool sliderNotes = false;
		bool hopoOn = false;
		bool hopoOff = false;

		uint32_t notes[6] = { UINT32_MAX };
		int numActive = 0;
		int numAdded = 0;
		uint32_t position = UINT32_MAX;
	} difficultyTracker[5];
	// Diff 5 = BRE

	for (auto& diff : m_difficulties)
		diff.m_notes.reserve(5000);

	uint32_t solo = UINT32_MAX;
	uint32_t starPower = UINT32_MAX;
	bool enhancedForEasy = false;
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
						std::string ev((char*)current, length);
						if (ev != "[ENHANCED_OPENS]")
						{
							if (m_difficulties[3].m_events.empty() || m_difficulties[3].m_events.back().first < position)
							{
								static std::pair<uint32_t, std::vector<std::string>> pairNode;
								pairNode.first = position;
								m_difficulties[3].m_events.push_back(pairNode);
							}

							m_difficulties[3].m_events.back().second.push_back(std::move(ev));
						}
						else
							enhancedForEasy = true;
					}

					if (type == 0x2F)
						break;

					current += length;
				}
				else if (syntax == 0xF0 || syntax == 0xF7)
				{
					VariableLengthQuantity length(current);
					if (strncmp((const char*)current, "PS", 2) == 0)
					{
						if (current[4] == 0xFF)
						{
							switch (current[5])
							{
							case 1:
								for (auto& diff : difficultyTracker)
									diff.greenToOpen = current[6];
								break;
							case 4:
								for (auto& diff : difficultyTracker)
									diff.sliderNotes = current[6];
								break;
							}
						}
						else
						{
							switch (current[5])
							{
							case 1:
								difficultyTracker[current[4]].greenToOpen = current[6];
								break;
							case 4:
								difficultyTracker[current[4]].sliderNotes = current[6];
								break;
							}
						}
					}
					current += length;
				}
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
			if (59 <= note && note < 103)
			{
				int noteValue = note - 59;
				int diff = noteValue / 12;
				int lane = noteValue % 12;
				// Animation
				if (note == 59 && !enhancedForEasy)
					break;

				// HopoON marker
				if (lane == 6)
				{
					difficultyTracker[diff].hopoOn = syntax == 0x90 && velocity > 0;
					if (difficultyTracker[diff].hopoOn && difficultyTracker[diff].position == position)
						m_difficulties[diff].m_notes.back().second.modify('<');
				}
				// HopoOff marker
				else if (lane == 7)
				{
					difficultyTracker[diff].hopoOff = syntax == 0x90 && velocity > 0;
					if (difficultyTracker[diff].hopoOff && difficultyTracker[diff].position == position)
						m_difficulties[diff].m_notes.back().second.modify('>');
				}
				else if (lane < 6)
				{
					if (difficultyTracker[diff].greenToOpen && lane == 1)
						lane = 0;
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
							static std::pair<uint32_t, GuitarNote<5>> pairNode;
							pairNode.first = position;
							m_difficulties[diff].m_notes.push_back(pairNode);

							if (difficultyTracker[diff].sliderNotes)
								m_difficulties[diff].m_notes.back().second.modify('T', false);

							if (difficultyTracker[diff].hopoOn)
								m_difficulties[diff].m_notes.back().second.modify('<');
							else if (difficultyTracker[diff].hopoOff)
								m_difficulties[diff].m_notes.back().second.modify('>');

							difficultyTracker[diff].position = position;
							++difficultyTracker[diff].numAdded;
						}

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
			// BRE
			else if (120 <= note && note <= 124)
			{
				int lane = note - 120;
				if (syntax == 0x90 && velocity > 0)
				{
					if (difficultyTracker[4].position == UINT32_MAX || difficultyTracker[4].position < position)
					{
						static std::pair<uint32_t, GuitarNote<5>> pairNode;
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
					// Slider/Tap
				case 104:
				{
					bool active = syntax == 0x90 && velocity > 0;
					for (auto& diff : difficultyTracker)
						diff.sliderNotes = active;
					break;
				}
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
void InstrumentalTrack<GuitarNote<6>>::load_midi(const unsigned char* current, const unsigned char* const end)
{
	struct
	{
		bool sliderNotes = false;
		bool hopoOn = false;
		bool hopoOff = false;

		uint32_t notes[7] = { UINT32_MAX };
		int numActive = 0;
		int numAdded = 0;
		uint32_t position = UINT32_MAX;
	} difficultyTracker[5];
	// Diff 5 = BRE

	for (auto& diff : m_difficulties)
		diff.m_notes.reserve(5000);

	uint32_t solo = UINT32_MAX;
	uint32_t starPower = UINT32_MAX;
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

						m_difficulties[3].m_events.back().second.emplace_back((char*)current, length);
					}

					if (type == 0x2F)
						break;

					current += length;
				}
				else if (syntax == 0xF0 || syntax == 0xF7)
				{
					VariableLengthQuantity length(current);
					if (strncmp((const char*)current, "PS", 2) == 0)
					{
						if (current[4] == 0xFF)
						{
							if (current[5] == 4)
								for (auto& diff : difficultyTracker)
									diff.sliderNotes = current[6];
						}
						else if (current[5] == 4)
							difficultyTracker[current[4]].sliderNotes = current[6];
					}
					current += length;
				}
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
			if (58 <= note && note < 103)
			{
				int noteValue = note - 59;
				int diff = noteValue / 12;
				int lane = noteValue % 12;
				// HopoON marker
				if (lane == 7)
				{
					difficultyTracker[diff].hopoOn = syntax == 0x90 && velocity > 0;
					if (difficultyTracker[diff].hopoOn && difficultyTracker[diff].position == position)
						m_difficulties[diff].m_notes.back().second.modify('<');
				}
				// HopoOff marker
				else if (lane == 8)
				{
					difficultyTracker[diff].hopoOff = syntax == 0x90 && velocity > 0;
					if (difficultyTracker[diff].hopoOff && difficultyTracker[diff].position == position)
						m_difficulties[diff].m_notes.back().second.modify('>');
				}
				else if (lane < 6)
				{
					// 0 = Open
					if (lane != 0)
					{
						// White and black notes are swapped in the file for some reason
						if (lane < 4)
							lane += 3;
						else
							lane -= 3;
					}

					if (syntax == 0x90 && velocity > 0)
					{
						if (difficultyTracker[diff].position == UINT32_MAX || difficultyTracker[diff].position < position)
						{
							static std::pair<uint32_t, GuitarNote<6>> pairNode;
							pairNode.first = position;
							m_difficulties[diff].m_notes.push_back(pairNode);

							if (difficultyTracker[diff].sliderNotes)
								m_difficulties[diff].m_notes.back().second.modify('T', false);

							if (difficultyTracker[diff].hopoOn)
								m_difficulties[diff].m_notes.back().second.modify('<');
							else if (difficultyTracker[diff].hopoOff)
								m_difficulties[diff].m_notes.back().second.modify('>');

							difficultyTracker[diff].position = position;
							++difficultyTracker[diff].numAdded;
						}

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
			// BRE
			else if (120 <= note && note <= 124)
			{
				int lane = note - 120;
				if (syntax == 0x90 && velocity > 0)
				{
					if (difficultyTracker[4].position == UINT32_MAX || difficultyTracker[4].position < position)
					{
						static std::pair<uint32_t, GuitarNote<6>> pairNode;
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
					// Slider/Tap
				case 104:
				{
					bool active = syntax == 0x90 && velocity > 0;
					for (auto& diff : difficultyTracker)
						diff.sliderNotes = active;
					break;
				}
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
void InstrumentalTrack<GuitarNote<5>>::save_midi(const char* const name, std::fstream& outFile) const
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

		events.addEvent(0, new MidiFile::MidiChunk_Track::MetaEvent_Text(1, "[ENHANCED_OPENS]"));
		uint32_t sliderNotes = UINT32_MAX;
		GuitarNote<5>::ForceStatus currStatus[4] = { GuitarNote<5>::ForceStatus::UNFORCED };
		auto processNote = [&](const std::pair<uint32_t, GuitarNote<5>>& note,
			char baseMidiNote,
			int difficulty,
			const std::pair<uint32_t, GuitarNote<5>>* const prev)
		{
			auto placeNote = [&](char midiNote, uint32_t sustain)
			{
				events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote));
				if (sustain == 0)
					events.addEvent(note.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote, 0));
				else
					events.addEvent(note.first + sustain, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote, 0));
			};

			if (note.second.m_special)
				placeNote(baseMidiNote, note.second.m_special.getSustain());
			else
			{
				for (char col = 0; col < 5; ++col)
					if (note.second.m_colors[col])
						placeNote(baseMidiNote + col + 1, note.second.m_colors[col].getSustain());
			}

			if (currStatus[difficulty] != note.second.m_isForced)
			{
				switch (note.second.m_isForced)
				{
				case GuitarNote<5>::ForceStatus::HOPO_ON:
					if (currStatus[difficulty] == GuitarNote<5>::ForceStatus::HOPO_OFF)
						// Disable the hopo off status note
						events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7, 0));
					events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6));
					currStatus[difficulty] = GuitarNote<5>::ForceStatus::HOPO_ON;
					break;
				case GuitarNote<5>::ForceStatus::HOPO_OFF:
					if (currStatus[difficulty] == GuitarNote<5>::ForceStatus::HOPO_ON)
						// Disable the hopo on status note
						events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6, 0));
					events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7));
					currStatus[difficulty] = GuitarNote<5>::ForceStatus::HOPO_OFF;
					break;
				case GuitarNote<5>::ForceStatus::FORCED:
					// Naturally a hopo, so add Forced HOPO Off
					if (note.second.getNumActive() < 2 &&
						prev &&
						note.first <= prev->first + Sustainable::getForceThreshold())
					{
						if (currStatus[difficulty] == GuitarNote<5>::ForceStatus::HOPO_ON)
							// Disable the hopo on status note
							events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6, 0));
						if (currStatus[difficulty] != GuitarNote<5>::ForceStatus::HOPO_OFF)
						{
							events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7));
							currStatus[difficulty] = GuitarNote<5>::ForceStatus::HOPO_OFF;
						}
					}
					// Naturally a strum, so add Forced HOPO On
					else
					{
						if (currStatus[difficulty] == GuitarNote<5>::ForceStatus::HOPO_OFF)
							// Disable the hopo on status note
							events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7, 0));
						if (currStatus[difficulty] != GuitarNote<5>::ForceStatus::HOPO_ON)
						{
							events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6));
							currStatus[difficulty] = GuitarNote<5>::ForceStatus::HOPO_ON;
						}
					}
					break;
				case GuitarNote<5>::ForceStatus::UNFORCED:
					if (currStatus[difficulty] == GuitarNote<5>::ForceStatus::HOPO_OFF)
						// Disable the hopo off status note
						events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7, 0));
					else
						// Disable the hopo on status note
						events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6, 0));
					currStatus[difficulty] = GuitarNote<5>::ForceStatus::UNFORCED;
				}
			}

			// To properly place the NoteOff for a slider event, we need to know
			// what the longest sustain in this note is
			// 
			// Notice: while only notes in the highest track can notate when the actual event turns off or on,
			// any note on any track can determine what tick it ends on
			if (sliderNotes != UINT32_MAX)
			{
				uint32_t sustain = 0;
				if (note.second.m_special)
					sustain = note.second.m_special.getSustain();
				else
				{
					for (const auto& color : note.second.m_colors)
						if (color && color.getSustain() > sustain)
							sustain = color.getSustain();
				}

				if (sustain == 0)
					sustain = 1;

				if (sliderNotes < note.first + sustain)
					sliderNotes = note.first + sustain;
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

		int sliderDifficulty = 3;
		if (!expertValid)
		{
			--sliderDifficulty;
			if (!hardValid)
			{
				--sliderDifficulty;
				if (!mediumValid)
					--sliderDifficulty;
			}
		}
		
		auto adjustSlider = [&](const std::pair<uint32_t, GuitarNote<5>>& pair)
		{
			if (pair.second.m_isTap)
			{
				// NoteOn
				if (sliderNotes == UINT32_MAX)
					events.addEvent(expertIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104));
				sliderNotes = pair.first;
			}
			else if (sliderNotes != UINT32_MAX)
			{
				if (sliderNotes <= pair.first)
					// The previous note ended, so we can attach the NoteOff to its end
					events.addEvent(sliderNotes, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104, 0));
				else
					// This note cuts off the slider event earlier than expected
					events.addEvent(pair.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104, 0));
				sliderNotes = UINT32_MAX;
			}
		};

		while (expertValid || hardValid || mediumValid || easyValid)
		{
			while (expertValid &&
				(!hardValid || expertIter->first <= hardIter->first) &&
				(!mediumValid || expertIter->first <= mediumIter->first) &&
				(!easyValid || expertIter->first <= easyIter->first))
			{
				if (sliderDifficulty == 3)
					adjustSlider(*expertIter);

				if (expertIter != m_difficulties[3].m_notes.begin())
					processNote(*expertIter, 95, 3, (expertIter - 1)._Ptr);
				else
					processNote(*expertIter, 95, 3, nullptr);
				expertValid = ++expertIter != m_difficulties[3].m_notes.end();
			}

			while (hardValid &&
				(!expertValid || hardIter->first < expertIter->first) &&
				(!mediumValid || hardIter->first <= mediumIter->first) &&
				(!easyValid || hardIter->first <= easyIter->first))
			{
				if (sliderDifficulty == 2)
					adjustSlider(*hardIter);

				if (hardIter != m_difficulties[2].m_notes.begin())
					processNote(*hardIter, 83, 2, (hardIter - 1)._Ptr);
				else
					processNote(*hardIter, 83, 2, nullptr);
				hardValid = ++hardIter != m_difficulties[2].m_notes.end();
			}

			while (mediumValid &&
				(!expertValid || mediumIter->first < expertIter->first) &&
				(!hardValid || mediumIter->first < hardIter->first) &&
				(!easyValid || mediumIter->first <= easyIter->first))
			{
				if (sliderDifficulty == 1)
					adjustSlider(*mediumIter);

				if (mediumIter != m_difficulties[1].m_notes.begin())
					processNote(*mediumIter, 71, 1, (mediumIter - 1)._Ptr);
				else
					processNote(*mediumIter, 71, 1, nullptr);
				mediumValid = ++mediumIter != m_difficulties[1].m_notes.end();
			}

			while (easyValid &&
				(!expertValid || easyIter->first < expertIter->first) &&
				(!hardValid || easyIter->first < hardIter->first) &&
				(!mediumValid || easyIter->first < mediumIter->first))
			{
				if (sliderDifficulty == 0)
					adjustSlider(*easyIter);

				if (easyIter != m_difficulties[0].m_notes.begin())
					processNote(*easyIter, 59, 0, (easyIter - 1)._Ptr);
				else
					processNote(*easyIter, 59, 0, nullptr);
				easyValid = ++easyIter != m_difficulties[0].m_notes.end();
			}
		}

		for (int i = 3; i >= 0; --i)
		{
			const unsigned char base = 59 + 12 * i;
			switch (currStatus[i])
			{
			case GuitarNote<5>::ForceStatus::HOPO_ON:
				// Disable the hopo on status note
				events.addEvent(m_difficulties[i].m_notes.back().first + m_difficulties[i].m_notes.back().second.getLongestSustain(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 6, 0));
				break;
			case GuitarNote<5>::ForceStatus::HOPO_OFF:
				// Disable the hopo off status note
				events.addEvent(m_difficulties[i].m_notes.back().first + m_difficulties[i].m_notes.back().second.getLongestSustain(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 7, 0));
				break;
			}
		}

		// Add the NoteOff event for a remaining slider phrase
		if (sliderNotes != UINT32_MAX)
			events.addEvent(sliderNotes, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104, 0));
	}

	events.writeToFile(outFile);
}

template<>
void InstrumentalTrack<GuitarNote<6>>::save_midi(const char* const name, std::fstream& outFile) const
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
		uint32_t sliderNotes = UINT32_MAX;
		GuitarNote<6>::ForceStatus currStatus[4] = { GuitarNote<6>::ForceStatus::UNFORCED };
		auto processNote = [&](const std::pair<uint32_t, GuitarNote<6>>& note,
			char baseMidiNote,
			int difficulty,
			const std::pair<uint32_t, GuitarNote<6>>* const prev)
		{
			auto placeNote = [&](char midiNote, uint32_t sustain)
			{
				events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote));
				if (sustain == 0)
					events.addEvent(note.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote, 0));
				else
					events.addEvent(note.first + sustain, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote, 0));
			};

			if (note.second.m_special)
				placeNote(baseMidiNote, note.second.m_special.getSustain());
			else
			{
				for (char col = 0; col < 5; ++col)
					if (note.second.m_colors[col])
						placeNote(baseMidiNote + col + 1, note.second.m_colors[col].getSustain());
			}

			if (currStatus[difficulty] != note.second.m_isForced)
			{
				switch (note.second.m_isForced)
				{
				case GuitarNote<6>::ForceStatus::HOPO_ON:
					if (currStatus[difficulty] == GuitarNote<6>::ForceStatus::HOPO_OFF)
						// Disable the hopo off status note
						events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7, 0));
					events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6));
					currStatus[difficulty] = GuitarNote<6>::ForceStatus::HOPO_ON;
					break;
				case GuitarNote<6>::ForceStatus::HOPO_OFF:
					if (currStatus[difficulty] == GuitarNote<6>::ForceStatus::HOPO_ON)
						// Disable the hopo on status note
						events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6, 0));
					events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7));
					currStatus[difficulty] = GuitarNote<6>::ForceStatus::HOPO_OFF;
					break;
				case GuitarNote<6>::ForceStatus::FORCED:
					// Naturally a hopo, so add Forced HOPO Off
					if (note.second.getNumActive() < 2 &&
						prev &&
						note.first <= prev->first + Sustainable::getForceThreshold())
					{
						if (currStatus[difficulty] == GuitarNote<6>::ForceStatus::HOPO_ON)
							// Disable the hopo on status note
							events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6, 0));
						if (currStatus[difficulty] != GuitarNote<6>::ForceStatus::HOPO_OFF)
						{
							events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7));
							currStatus[difficulty] = GuitarNote<6>::ForceStatus::HOPO_OFF;
						}
					}
					// Naturally a strum, so add Forced HOPO On
					else
					{
						if (currStatus[difficulty] == GuitarNote<6>::ForceStatus::HOPO_OFF)
							// Disable the hopo on status note
							events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7, 0));
						if (currStatus[difficulty] != GuitarNote<6>::ForceStatus::HOPO_ON)
						{
							events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6));
							currStatus[difficulty] = GuitarNote<6>::ForceStatus::HOPO_ON;
						}
					}
					break;
				case GuitarNote<6>::ForceStatus::UNFORCED:
					if (currStatus[difficulty] == GuitarNote<6>::ForceStatus::HOPO_OFF)
						// Disable the hopo off status note
						events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7, 0));
					else
						// Disable the hopo on status note
						events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6, 0));
					currStatus[difficulty] = GuitarNote<6>::ForceStatus::UNFORCED;
				}
			}

			// To properly place the NoteOff for a slider event, we need to know
			// what the longest sustain in this note is
			// 
			// Notice: while the actual event turning off and on is notated only by the highest track,
			// *when* it ends can be determined by any note on any track
			if (sliderNotes != UINT32_MAX)
			{
				uint32_t sustain = 0;
				if (note.second.m_special)
					sustain = note.second.m_special.getSustain();
				else
				{
					for (const auto& color : note.second.m_colors)
						if (color && color.getSustain() > sustain)
							sustain = color.getSustain();
				}

				if (sustain == 0)
					sustain = 1;

				if (sliderNotes < note.first + sustain)
					sliderNotes = note.first + sustain;
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

		int sliderDifficulty = 3;
		if (!expertValid)
		{
			--sliderDifficulty;
			if (!hardValid)
			{
				--sliderDifficulty;
				if (!mediumValid)
					--sliderDifficulty;
			}
		}

		auto adjustSlider = [&](const std::pair<uint32_t, GuitarNote<6>>& pair)
		{
			if (pair.second.m_isTap)
			{
				// NoteOn
				if (sliderNotes == UINT32_MAX)
					events.addEvent(expertIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104));
				sliderNotes = pair.first;
			}
			else if (sliderNotes != UINT32_MAX)
			{
				if (sliderNotes <= pair.first)
					// The previous note ended, so we can attach the NoteOff to its end
					events.addEvent(sliderNotes, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104, 0));
				else
					// This note cuts off the slider event earlier than expected
					events.addEvent(pair.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104, 0));
				sliderNotes = UINT32_MAX;
			}
		};

		while (expertValid || hardValid || mediumValid || easyValid)
		{
			while (expertValid &&
				(!hardValid || expertIter->first <= hardIter->first) &&
				(!mediumValid || expertIter->first <= mediumIter->first) &&
				(!easyValid || expertIter->first <= easyIter->first))
			{
				if (sliderDifficulty == 3)
					adjustSlider(*expertIter);

				if (expertIter != m_difficulties[3].m_notes.begin())
					processNote(*expertIter, 94, 3, (expertIter - 1)._Ptr);
				else
					processNote(*expertIter, 94, 3, nullptr);
				expertValid = ++expertIter != m_difficulties[3].m_notes.end();
			}

			while (hardValid &&
				(!expertValid || hardIter->first < expertIter->first) &&
				(!mediumValid || hardIter->first <= mediumIter->first) &&
				(!easyValid || hardIter->first <= easyIter->first))
			{
				if (sliderDifficulty == 2)
					adjustSlider(*hardIter);

				if (hardIter != m_difficulties[2].m_notes.begin())
					processNote(*hardIter, 82, 2, (hardIter - 1)._Ptr);
				else
					processNote(*hardIter, 82, 2, nullptr);
				hardValid = ++hardIter != m_difficulties[2].m_notes.end();
			}

			while (mediumValid &&
				(!expertValid || mediumIter->first < expertIter->first) &&
				(!hardValid || mediumIter->first < hardIter->first) &&
				(!easyValid || mediumIter->first <= easyIter->first))
			{
				if (sliderDifficulty == 1)
					adjustSlider(*mediumIter);

				if (mediumIter != m_difficulties[1].m_notes.begin())
					processNote(*mediumIter, 70, 1, (mediumIter - 1)._Ptr);
				else
					processNote(*mediumIter, 70, 1, nullptr);
				mediumValid = ++mediumIter != m_difficulties[1].m_notes.end();
			}

			while (easyValid &&
				(!expertValid || easyIter->first < expertIter->first) &&
				(!hardValid || easyIter->first < hardIter->first) &&
				(!mediumValid || easyIter->first < mediumIter->first))
			{
				if (sliderDifficulty == 0)
					adjustSlider(*easyIter);

				if (easyIter != m_difficulties[0].m_notes.begin())
					processNote(*easyIter, 58, 0, (easyIter - 1)._Ptr);
				else
					processNote(*easyIter, 58, 0, nullptr);
				easyValid = ++easyIter != m_difficulties[0].m_notes.end();
			}
		}

		for (int i = 3; i >= 0; --i)
		{
			const unsigned char base = 59 + 12 * i;
			switch (currStatus[i])
			{
			case GuitarNote<6>::ForceStatus::HOPO_ON:
				// Disable the hopo on status note
				events.addEvent(m_difficulties[i].m_notes.back().first + m_difficulties[i].m_notes.back().second.getLongestSustain(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 6, 0));
				break;
			case GuitarNote<6>::ForceStatus::HOPO_OFF:
				// Disable the hopo off status note
				events.addEvent(m_difficulties[i].m_notes.back().first + m_difficulties[i].m_notes.back().second.getLongestSustain(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 7, 0));
				break;
			}
		}

		// Add the NoteOff event for a remaining slider phrase
		if (sliderNotes != UINT32_MAX)
		{
			events.addEvent(sliderNotes, new MidiFile::MidiChunk_Track::SysexEvent(0xFF, 4, 0));
			sliderNotes = UINT32_MAX;
		}
	}

	events.writeToFile(outFile);
}