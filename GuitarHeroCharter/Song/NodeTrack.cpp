#include "NodeTrack.h"
using namespace MidiFile;

template<>
void NodeTrack<GuitarNote<5>>::load_midi(const MidiChunk_Track& track)
{
	struct
	{
		bool greenToOpen = false;
		bool sliderNotes = false;
		uint32_t notes[6] = { UINT32_MAX };
		bool hopoOn = false;
		bool hopoOff = false;
	} difficultyTracker[5];
	// Diff 5 = BRE

	uint32_t solo = UINT32_MAX;
	uint32_t starPower = UINT32_MAX;

	for (auto& vec : track.m_events)
	{
		for (auto ptr : vec.second)
		{
			switch (ptr->m_syntax)
			{
			case 0xF0:
			{
				MidiChunk_Track::SysexEvent* sysex = static_cast<MidiChunk_Track::SysexEvent*>(ptr);
				if (sysex->m_data[4] == 0xFF)
				{
					switch (sysex->m_data[5])
					{
					case 1:
						for (auto& diff : difficultyTracker)
							diff.greenToOpen = sysex->m_data[6];
						break;
					case 4:
						for (auto& diff : difficultyTracker)
							diff.sliderNotes = sysex->m_data[6];
						break;
					}
				}
				else
				{
					switch (sysex->m_data[5])
					{
					case 1:
						difficultyTracker[3 - sysex->m_data[4]].greenToOpen = sysex->m_data[6];
						break;
					case 4:
						difficultyTracker[3 - sysex->m_data[4]].sliderNotes = sysex->m_data[6];
						break;
					}
				}
				break;
			}
			case 0xFF:
			{
				MidiChunk_Track::MetaEvent* ev = static_cast<MidiChunk_Track::MetaEvent*>(ptr);
				if (ev->m_type == 0x01)
				{
					MidiChunk_Track::MetaEvent_Text* text = static_cast<MidiChunk_Track::MetaEvent_Text*>(ptr);
					if (text->m_text != "[ENHANCED_OPENS]")
						m_difficulties[0].addEvent(vec.first, std::string(text->m_text));
				}
				break;
			}
			case 0x90:
			case 0x80:
			{
				MidiChunk_Track::MidiEvent_Note* note = static_cast<MidiChunk_Track::MidiEvent_Note*>(ptr);
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
				*	120 - 125 = fill(BRE)
				*	126 = tremolo
				*	127 = trill
				*/

				// Notes
				if (59 <= note->m_note && note->m_note < 103)
				{
					int noteValue = note->m_note - 59;
					int diff = 3 - (noteValue / 12);
					int lane = noteValue % 12;
					// HopoON marker
					if (lane == 6)
					{
						difficultyTracker[diff].hopoOn = note->m_syntax == 0x90 && note->m_velocity > 0;
						if (difficultyTracker[diff].hopoOn)
							m_difficulties[diff].modifyNote(vec.first, '<');
					}
					// HopoOff marker
					else if (lane == 7)
					{
						difficultyTracker[diff].hopoOff = note->m_syntax == 0x90 && note->m_velocity > 0;
						if (difficultyTracker[diff].hopoOff)
							m_difficulties[diff].modifyNote(vec.first, '>');
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

						if (note->m_syntax == 0x90 && note->m_velocity > 0)
						{
							difficultyTracker[diff].notes[lane] = vec.first;
							if (difficultyTracker[diff].sliderNotes)
								m_difficulties[diff].modifyNote(difficultyTracker[diff].notes[lane], 'T', false);

							if (difficultyTracker[diff].hopoOn)
								m_difficulties[diff].modifyNote(difficultyTracker[diff].notes[lane], '<');

							if (difficultyTracker[diff].hopoOff)
								m_difficulties[diff].modifyNote(difficultyTracker[diff].notes[lane], '>');
						}
						else
						{
							m_difficulties[diff].addNoteFromMid(lane, difficultyTracker[diff].notes[lane], vec.first - difficultyTracker[diff].notes[lane]);
							difficultyTracker[diff].notes[lane] = UINT32_MAX;
						}
					}
				}
				// Slider/Tap
				else if (note->m_note == 104)
				{
					bool active = note->m_syntax == 0x90 && note->m_velocity > 0;
					for (auto& diff : difficultyTracker)
						diff.sliderNotes = active;
				}
				// Star Power
				else if (note->m_note == 116)
				{
					if (note->m_syntax == 0x90 && note->m_velocity > 0)
						starPower = vec.first;
					else
						m_difficulties[0].addEffect(starPower, new StarPowerPhrase(vec.first - starPower));
				}
				// Soloes
				if (note->m_note == 103)
				{
					if (note->m_syntax == 0x90 && note->m_velocity > 0)
						solo = vec.first;
					else
						m_difficulties[0].addSolo(solo, vec.first - solo);
				}
				break;
			}
			}
		}
	}
}

template<>
void NodeTrack<GuitarNote<6>>::load_midi(const MidiChunk_Track& track)
{
	struct
	{
		bool sliderNotes = false;
		uint32_t notes[7] = { UINT32_MAX };
		bool hopoOn = false;
		bool hopoOff = false;
	} difficultyTracker[5];
	// Diff 5 = BRE

	uint32_t solo = UINT32_MAX;
	uint32_t starPower = UINT32_MAX;

	for (auto& vec : track.m_events)
	{
		for (auto ptr : vec.second)
		{
			switch (ptr->m_syntax)
			{
			case 0xF0:
			{
				MidiChunk_Track::SysexEvent* sysex = static_cast<MidiChunk_Track::SysexEvent*>(ptr);
				if (sysex->m_data[5] == 4)
				{
					if (sysex->m_data[4] == 0xFF)
					{
						for (auto& diff : difficultyTracker)
							diff.sliderNotes = sysex->m_data[6];
					}
					else
						difficultyTracker[3 - sysex->m_data[4]].sliderNotes = sysex->m_data[6];
				}
				break;
			}
			case 0xFF:
			{
				MidiChunk_Track::MetaEvent* ev = static_cast<MidiChunk_Track::MetaEvent*>(ptr);
				if (ev->m_type == 0x01)
					m_difficulties[0].addEvent(vec.first, std::string(static_cast<MidiChunk_Track::MetaEvent_Text*>(ptr)->m_text));
				break;
			}
			case 0x90:
			case 0x80:
			{
				MidiChunk_Track::MidiEvent_Note* note = static_cast<MidiChunk_Track::MidiEvent_Note*>(ptr);
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
				*	120 - 125 = fill(BRE)
				*	126 = tremolo
				*	127 = trill
				*/

				// Notes
				if (58 <= note->m_note && note->m_note < 103)
				{
					int noteValue = note->m_note - 59;
					int diff = 3 - (noteValue / 12);
					int lane = noteValue % 12;
					if (lane == 7)
					{
						difficultyTracker[diff].hopoOn = note->m_syntax == 0x90 && note->m_velocity > 0;
						if (difficultyTracker[diff].hopoOn)
							m_difficulties[diff].modifyNote(vec.first, '<');
					}
					else if (lane == 8)
					{
						difficultyTracker[diff].hopoOff = note->m_syntax == 0x90 && note->m_velocity > 0;
						if (difficultyTracker[diff].hopoOff)
							m_difficulties[diff].modifyNote(vec.first, '>');
					}
					else if (lane < 7)
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

						if (note->m_syntax == 0x90 && note->m_velocity > 0)
						{
							difficultyTracker[diff].notes[lane] = vec.first;
							if (difficultyTracker[diff].sliderNotes)
								m_difficulties[diff].modifyNote(difficultyTracker[diff].notes[lane], 'T', false);

							if (difficultyTracker[diff].hopoOn)
								m_difficulties[diff].modifyNote(difficultyTracker[diff].notes[lane], '<');

							if (difficultyTracker[diff].hopoOff)
								m_difficulties[diff].modifyNote(difficultyTracker[diff].notes[lane], '>');
						}
						else
						{
							m_difficulties[diff].addNoteFromMid(lane, difficultyTracker[diff].notes[lane], vec.first - difficultyTracker[diff].notes[lane]);
							difficultyTracker[diff].notes[lane] = UINT32_MAX;
						}
					}
				}
				// Slider/Tap
				else if (note->m_note == 104)
				{
					bool active = note->m_syntax == 0x90 && note->m_velocity > 0;
					for (auto& diff : difficultyTracker)
						diff.sliderNotes = active;
				}
				// Star Power
				else if (note->m_note == 116)
				{
					if (note->m_syntax == 0x90 && note->m_velocity > 0)
						starPower = vec.first;
					else
						m_difficulties[0].addEffect(starPower, new StarPowerPhrase(vec.first - starPower));
				}
				// Soloes
				else if (note->m_note == 103)
				{
					if (note->m_syntax == 0x90 && note->m_velocity > 0)
						solo = vec.first;
					else
						m_difficulties[0].addSolo(solo, vec.first - solo);
				}
				break;
			}
			}
		}
	}
}

template<>
void NodeTrack<DrumNote>::load_midi(const MidiChunk_Track& track)
{
	struct
	{
		uint32_t notes[6] = { UINT32_MAX };
		uint32_t flam = UINT32_MAX;
	} difficultyTracker[5];
	// Diff 5 = BRE

	uint32_t solo = UINT32_MAX;
	uint32_t starPower = UINT32_MAX;
	uint32_t fill = UINT32_MAX;
	uint32_t tremolo = UINT32_MAX;
	uint32_t trill = UINT32_MAX;
	bool toms[3] = { false };

	for (auto& vec : track.m_events)
	{
		for (auto ptr : vec.second)
		{
			switch (ptr->m_syntax)
			{
			case 0xF0:
				break;
			case 0xFF:
			{
				MidiChunk_Track::MetaEvent* ev = static_cast<MidiChunk_Track::MetaEvent*>(ptr);
				if (ev->m_type == 0x01)
					m_difficulties[0].addEvent(vec.first, std::string(static_cast<MidiChunk_Track::MetaEvent_Text*>(ptr)->m_text));
				break;
			}
			case 0x90:
			case 0x80:
			{
				MidiChunk_Track::MidiEvent_Note* note = static_cast<MidiChunk_Track::MidiEvent_Note*>(ptr);
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
				*	120 - 125 = fill(BRE)
				*	126 = tremolo
				*	127 = trill
				*/

				// Expert+
				if (note->m_note == 95)
				{
					if (note->m_syntax == 0x90 && note->m_velocity > 0)
						difficultyTracker[0].notes[0] = vec.first;
					else
						m_difficulties[0].modifyColor(0, difficultyTracker[0].notes[0], 'X');
					difficultyTracker[0].notes[0] = UINT32_MAX;
				}
				// Notes
				else if (60 <= note->m_note && note->m_note < 102)
				{
					int noteValue = note->m_note - 60;
					int diff = 3 - (noteValue / 12);
					int lane = noteValue % 12;
					if (lane < 6)
					{
						if (note->m_syntax == 0x90 && note->m_velocity > 0)
						{
							difficultyTracker[diff].notes[lane] = vec.first;

							if (2 <= lane && lane < 5 && !toms[lane - 2])
								m_difficulties[diff].modifyColor(lane, difficultyTracker[diff].notes[lane], 'C');

							if (note->m_velocity > 100)
								m_difficulties[diff].modifyColor(lane, difficultyTracker[diff].notes[lane], 'A');
							else if (note->m_velocity < 100)
								m_difficulties[diff].modifyColor(lane, difficultyTracker[diff].notes[lane], 'G');
						}
						else
						{
							m_difficulties[diff].addNoteFromMid(lane, difficultyTracker[diff].notes[lane], vec.first - difficultyTracker[diff].notes[lane]);
							difficultyTracker[diff].notes[lane] = UINT32_MAX;
						}
					}
				}
				// Tom markers
				else if (110 <= note->m_note && note->m_note <= 112)
					toms[note->m_note - 110] = note->m_syntax == 0x90 && note->m_velocity > 0;
				// Fill
				else if (note->m_note == 124)
				{
					if (note->m_syntax == 0x90 && note->m_velocity > 0)
						fill = vec.first;
					else
						m_difficulties[0].addEffect(vec.first, new StarPowerActivation(vec.first - fill));
				}
				// Star Power
				else if (note->m_note == 116)
				{
					if (note->m_syntax == 0x90 && note->m_velocity > 0)
						starPower = vec.first;
					else
						m_difficulties[0].addEffect(starPower, new StarPowerPhrase(vec.first - starPower));
				}
				// Soloes
				else if (note->m_note == 103)
				{
					if (note->m_syntax == 0x90 && note->m_velocity > 0)
						solo = vec.first;
					else
						m_difficulties[0].addSolo(solo, vec.first - solo);
				}
				// Flams
				else if (note->m_note == 109)
				{
					if (note->m_syntax == 0x90 && note->m_velocity > 0)
						difficultyTracker[0].flam = vec.first;
					else
						m_difficulties[0].modifyNote(difficultyTracker[0].flam, 'F');
				}
				// Tremolo (or single drum roll)
				else if (note->m_note == 126)
				{
					if (note->m_syntax == 0x90 && note->m_velocity > 0)
						tremolo = vec.first;
					else
						m_difficulties[0].addEffect(tremolo, new Tremolo(vec.first - tremolo));
				}
				// Trill (or special drum roll)
				else if (note->m_note == 127)
				{
					if (note->m_syntax == 0x90 && note->m_velocity > 0)
						trill = vec.first;
					else
						m_difficulties[0].addEffect(trill, new Trill(vec.first - trill));
				}
				break;
			}
			}
		}
	}
}
