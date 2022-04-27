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
	bool fill = false;

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
					if (type == 1)
					{
						std::string ev((char*)currPtr, length);
						if (ev != "[ENHANCED_OPENS]")
							m_difficulties[0].addEvent(position, std::move(ev));
					}
					else if (type != 0x2F)
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
							difficultyTracker[3 - currPtr[4]].greenToOpen = currPtr[6];
							break;
						case 4:
						{
							difficultyTracker[3 - currPtr[4]].sliderNotes = currPtr[6];
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
		else
			note = tmpSyntax;

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
			*	120 - 125 = fill(BRE)
			*	126 = tremolo
			*	127 = trill
			*/

			// Notes
			if (59 <= note && note < 103)
			{
				int noteValue = note - 59;
				int diff = 3 - (noteValue / 12);
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
			// Slider/Tap
			else if (note == 104)
			{
				bool active = syntax == 0x90 && velocity > 0;
				for (auto& diff : difficultyTracker)
					diff.sliderNotes = active;
			}
			// Star Power
			else if (note == 116)
			{
				if (syntax == 0x90 && velocity > 0)
					starPower = position;
				else
					m_difficulties[0].addEffect(starPower, new StarPowerPhrase(position - starPower));
			}
			// Soloes
			else if (note == 103)
			{
				if (syntax == 0x90 && velocity > 0)
					solo = position;
				else
					m_difficulties[0].addEffect(solo, new Solo(position - solo));
			}
			// Fill
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
						fill = i == 5;
					}
				}
				else if (fill)
				{
					m_difficulties[0].addEffect(position, new StarPowerActivation(position - difficultyTracker[4].notes[lane]));
					fill = false;
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
	bool fill = false;

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
					if (type == 1)
						m_difficulties[0].addEvent(position, std::string((char*)currPtr, length));
					else if (type != 0x2F)
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
						difficultyTracker[3 - currPtr[4]].sliderNotes = currPtr[6];
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
			note = tmpSyntax;

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
			*	120 - 125 = fill(BRE)
			*	126 = tremolo
			*	127 = trill
			*/

			// Notes
			if (58 <= note && note < 103)
			{
				int noteValue = note - 59;
				int diff = 3 - (noteValue / 12);
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
			// Slider/Tap
			else if (note == 104)
			{
				bool active = syntax == 0x90 && velocity > 0;
				for (auto& diff : difficultyTracker)
					diff.sliderNotes = active;
			}
			// Star Power
			else if (note == 116)
			{
				if (syntax == 0x90 && velocity > 0)
					starPower = position;
				else
					m_difficulties[0].addEffect(starPower, new StarPowerPhrase(position - starPower));
			}
			// Soloes
			else if (note == 103)
			{
				if (syntax == 0x90 && velocity > 0)
					solo = position;
				else
					m_difficulties[0].addEffect(solo, new Solo(position - solo));
			}
			// Fill
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
						fill = i == 5;
					}
				}
				else if (fill)
				{
					m_difficulties[0].addEffect(position, new StarPowerActivation(position - difficultyTracker[4].notes[lane]));
					fill = false;
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
	bool fill = false;
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
					if (type == 1)
						m_difficulties[0].addEvent(position, std::string((char*)currPtr, length));
					else if (type != 0x2F)
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
		else
			note = tmpSyntax;

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
			*	120 - 125 = fill(BRE)
			*	126 = tremolo
			*	127 = trill
			*/

			// Expert+
			if (note == 95)
			{
				if (syntax == 0x90 && velocity > 0)
				{
					if (difficultyTracker[0].position != position)
					{
						static std::pair<uint32_t, DrumNote> pairNode;
						pairNode.first = position;

						m_difficulties[0].m_notes.push_back(pairNode);
						difficultyTracker[0].position = position;
					}

					m_difficulties[0].m_notes.back().second.modifyColor(0, '+');

					++difficultyTracker[0].numActive;
					difficultyTracker[0].notes[0] = position;
				}
				else
				{
					m_difficulties[0].addNoteFromMid(difficultyTracker[0].notes[0], 0, difficultyTracker[0].numAdded, position - difficultyTracker[0].notes[0]);
					--difficultyTracker[0].numActive;
					if (difficultyTracker[0].numActive == 0)
						difficultyTracker[0].numAdded = 0;
				}
			}
			// Notes
			else if (60 <= note && note < 102)
			{
				int noteValue = note - 60;
				int diff = 3 - (noteValue / 12);
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

						if (difficultyTracker[0].flam && diff == 0)
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
			// Fill
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
						fill = i == 5;
					}
				}
				else if (fill)
				{
					m_difficulties[0].addEffect(position, new StarPowerActivation(position - difficultyTracker[4].notes[lane]));
					fill = false;
				}
			}
			// Star Power
			else if (note == 116)
			{
				if (syntax == 0x90 && velocity > 0)
					starPower = position;
				else
					m_difficulties[0].addEffect(starPower, new StarPowerPhrase(position - starPower));
			}
			// Soloes
			else if (note == 103)
			{
				if (syntax == 0x90 && velocity > 0)
					solo = position;
				else
					m_difficulties[0].addEffect(solo, new Solo(position - solo));
			}
			// Flams
			else if (note == 109)
			{
				difficultyTracker[0].flam = syntax == 0x90 && velocity > 0;
				if (difficultyTracker[0].flam && difficultyTracker[0].position == position)
					m_difficulties[0].m_notes.back().second.modify('F');
			}
			// Tremolo (or single drum roll)
			else if (note == 126)
			{
				if (syntax == 0x90 && velocity > 0)
					tremolo = position;
				else
					m_difficulties[0].addEffect(tremolo, new Tremolo(position - tremolo));
			}
			// Trill (or special drum roll)
			else if (note == 127)
			{
				if (syntax == 0x90 && velocity > 0)
					trill = position;
				else
					m_difficulties[0].addEffect(trill, new Trill(position - trill));
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
}
