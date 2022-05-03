#include "NodeTrack.h"
#include "Midi/MidiFile.h"
using namespace MidiFile;

template<>
void NodeTrack<GuitarNote<5>>::load_midi(const unsigned char* currPtr, const unsigned char* const end)
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

	uint32_t solo = UINT32_MAX;
	uint32_t starPower = UINT32_MAX;
	bool doBRE = false;

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
					if (type == 1)
					{
						std::string ev((char*)currPtr, length);
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
					}

					if (type != 0x2F)
						currPtr += length;
					else
						break;
				}
				else if (syntax == 0xF0 || syntax == 0xF7)
				{
					VariableLengthQuantity length(currPtr);
					if (currPtr[4] == 0xFF)
					{
						switch (currPtr[5])
						{
						case 1:
							for (auto& diff : difficultyTracker)
								diff.greenToOpen = currPtr[6];
							break;
						case 4:
						{
							for (auto& diff : difficultyTracker)
								diff.sliderNotes = currPtr[6];
							break;
						}
						}
					}
					else
					{
						switch (currPtr[5])
						{
						case 1:
							difficultyTracker[currPtr[4]].greenToOpen = currPtr[6];
							break;
						case 4:
						{
							difficultyTracker[currPtr[4]].sliderNotes = currPtr[6];
							break;
						}
						}
					}
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
			if (59 <= note && note < 103)
			{
				int noteValue = note - 59;
				int diff = noteValue / 12;
				int lane = noteValue % 12;
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
						if (difficultyTracker[diff].position != position)
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
					difficultyTracker[4].notes[lane] = position;
					if (lane == 4)
					{
						int i = 0;
						while (i < 5 && difficultyTracker[4].notes[i] == position)
							++i;
						doBRE = i == 5;
					}
				}
				else if (doBRE)
				{
					m_difficulties[3].addEffect(position, new StarPowerActivation(position - difficultyTracker[4].notes[lane]));
					doBRE = false;
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
						m_difficulties[3].addEffect(starPower, new StarPowerPhrase(position - starPower));
					break;
				// Soloes
				case 103:
					if (syntax == 0x90 && velocity > 0)
						solo = position;
					else
						m_difficulties[3].addEffect(solo, new Solo(position - solo));
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

	for (auto& diff : m_difficulties)
		if (diff.m_notes.size() < diff.m_notes.capacity())
			diff.m_notes.shrink_to_fit();
}

template<>
void NodeTrack<GuitarNote<6>>::load_midi(const unsigned char* currPtr, const unsigned char* const end)
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

	uint32_t solo = UINT32_MAX;
	uint32_t starPower = UINT32_MAX;
	bool doBRE = false;

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
					if (type == 1)
					{
						if (m_difficulties[3].m_events.empty() || m_difficulties[3].m_events.back().first < position)
						{
							static std::pair<uint32_t, std::vector<std::string>> pairNode;
							pairNode.first = position;
							m_difficulties[3].m_events.push_back(pairNode);
						}

						m_difficulties[3].m_events.back().second.emplace_back((char*)currPtr, length);
					}

					if (type != 0x2F)
						currPtr += length;
					else
						break;
				}
				else if (syntax == 0xF0 || syntax == 0xF7)
				{
					VariableLengthQuantity length(currPtr);
					if (currPtr[4] == 0xFF)
					{
						if (currPtr[5] == 4)
							for (auto& diff : difficultyTracker)
								diff.sliderNotes = currPtr[6];
					}
					else if (currPtr[5] == 4)
						difficultyTracker[currPtr[4]].sliderNotes = currPtr[6];
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
				else if (lane < 8)
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
						if (difficultyTracker[diff].position != position)
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
					difficultyTracker[4].notes[lane] = position;
					if (lane == 4)
					{
						int i = 0;
						while (i < 5 && difficultyTracker[4].notes[i] == position)
							++i;
						doBRE = i == 5;
					}
				}
				else if (doBRE)
				{
					m_difficulties[3].addEffect(position, new StarPowerActivation(position - difficultyTracker[4].notes[lane]));
					doBRE = false;
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
						m_difficulties[3].addEffect(starPower, new StarPowerPhrase(position - starPower));
					break;
				// Soloes
				case 103:
					if (syntax == 0x90 && velocity > 0)
						solo = position;
					else
						m_difficulties[3].addEffect(solo, new Solo(position - solo));
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

	for (auto& diff : m_difficulties)
		if (diff.m_notes.size() < diff.m_notes.capacity())
			diff.m_notes.shrink_to_fit();
}

template<>
void NodeTrack<DrumNote>::load_midi(const unsigned char* currPtr, const unsigned char* const end)
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
					if (type == 1)
					{
						if (m_difficulties[3].m_events.empty() || m_difficulties[3].m_events.back().first < position)
						{
							static std::pair<uint32_t, std::vector<std::string>> pairNode;
							pairNode.first = position;
							m_difficulties[3].m_events.push_back(pairNode);
						}

						m_difficulties[3].m_events.back().second.emplace_back((char*)currPtr, length);
					}

					if (type != 0x2F)
						currPtr += length;
					else
						break;
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
			*	120 - 125 = fill/BRE
			*	126 = tremolo
			*	127 = trill
			*/

			// Expert+
			if (note == 95)
			{
				if (syntax == 0x90 && velocity > 0)
				{
					if (difficultyTracker[3].position != position)
					{
						static std::pair<uint32_t, DrumNote> pairNode;
						pairNode.first = position;

						m_difficulties[3].m_notes.push_back(pairNode);
						difficultyTracker[3].position = position;
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
						if (difficultyTracker[diff].position != position)
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
					difficultyTracker[4].notes[lane] = position;
					if (lane == 4)
					{
						int i = 0;
						while (i < 5 && difficultyTracker[4].notes[i] == position)
							++i;
						doBRE = i == 5;
					}
				}
				else if (doBRE)
				{
					m_difficulties[3].addEffect(position, new StarPowerActivation(position - difficultyTracker[4].notes[lane]));
					doBRE = false;
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
						m_difficulties[3].addEffect(starPower, new StarPowerPhrase(position - starPower));
					break;
				// Soloes
				case 103:
					if (syntax == 0x90 && velocity > 0)
						solo = position;
					else
						m_difficulties[3].addEffect(solo, new Solo(position - solo));
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
						m_difficulties[3].addEffect(tremolo, new Tremolo(position - tremolo));
					break;
				// Trill (or special drum roll)
				case 127:
					if (syntax == 0x90 && velocity > 0)
						trill = position;
					else
						m_difficulties[3].addEffect(trill, new Trill(position - trill));
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
void NodeTrack<GuitarNote<5>>::convertNotesToMid(MidiFile::MidiChunk_Track& events) const
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
				if (note.second.getNumActiveColors() < 2 &&
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
		--sliderDifficulty;
	if (!hardValid)
		--sliderDifficulty;
	if (!mediumValid)
		--sliderDifficulty;

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

template<>
void NodeTrack<GuitarNote<6>>::convertNotesToMid(MidiFile::MidiChunk_Track& events) const
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
				if (note.second.getNumActiveColors() < 2 &&
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
		--sliderDifficulty;
	if (!hardValid)
		--sliderDifficulty;
	if (!mediumValid)
		--sliderDifficulty;

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

template<>
void NodeTrack<DrumNote>::convertNotesToMid(MidiFile::MidiChunk_Track& events) const
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
		--adjustWithDifficulty;
	if (!hardValid)
		--adjustWithDifficulty;
	if (!mediumValid)
		--adjustWithDifficulty;

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
