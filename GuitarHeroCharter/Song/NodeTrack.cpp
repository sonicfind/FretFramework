#include "NodeTrack.h"
#include "Midi/MidiFile.h"
using namespace MidiFile;

template<>
void NodeTrack<GuitarNote<5>>::load_midi(const unsigned char* currPtr, const unsigned char* const end)
{
	struct
	{
		bool greenToOpen = false;
		uint32_t sliderNotes = UINT32_MAX;
		uint32_t forceNotes = UINT32_MAX;
		bool hopoOn = false;
		bool hopoOff = false;

		uint32_t notes[6] = { UINT32_MAX };
		uint32_t position = UINT32_MAX;
	} difficultyTracker[5];
	// Diff 5 = BRE

	uint32_t solo = UINT32_MAX;
	uint32_t starPower = UINT32_MAX;

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
						switch (currPtr[5])
						{
						case 1:
							for (auto& diff : difficultyTracker)
								diff.greenToOpen = currPtr[6];
							break;
						case 4:
						{
							uint32_t val = currPtr[6] ? 0 : UINT32_MAX;
							for (auto& diff : difficultyTracker)
								diff.sliderNotes = val;
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
							difficultyTracker[3 - currPtr[4]].sliderNotes = currPtr[6] ? 0 : UINT32_MAX;
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
					if (difficultyTracker[diff].hopoOn)
						m_difficulties[diff].modifyNote(position, '<');
				}
				// HopoOff marker
				else if (lane == 7)
				{
					difficultyTracker[diff].hopoOff = syntax == 0x90 && velocity > 0;
					if (difficultyTracker[diff].hopoOff)
						m_difficulties[diff].modifyNote(position, '>');
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

							m_difficulties[diff].m_notes.insert(pairNode);
							difficultyTracker[diff].position = position;
						}
						difficultyTracker[diff].notes[lane] = position;

						if (difficultyTracker[diff].sliderNotes < position)
						{
							if (difficultyTracker[diff].sliderNotes)
								m_difficulties[diff].modifyNote(position, 'T', false);
							difficultyTracker[diff].sliderNotes = position;
						}

						if (difficultyTracker[diff].forceNotes < position)
						{
							if (difficultyTracker[diff].hopoOn)
								m_difficulties[diff].modifyNote(difficultyTracker[diff].notes[lane], '<');
							else if (difficultyTracker[diff].hopoOff)
								m_difficulties[diff].modifyNote(difficultyTracker[diff].notes[lane], '>');

							difficultyTracker[diff].forceNotes = position;
						}
					}
					else
					{
						m_difficulties[diff].addNoteFromMid(lane, difficultyTracker[diff].notes[lane], position - difficultyTracker[diff].notes[lane]);
						difficultyTracker[diff].notes[lane] = UINT32_MAX;
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
			else if (120 <= note && note < 125)
			{
				char lane = note - 120;
				if (syntax == 0x90 && velocity > 0)
					difficultyTracker[4].notes[lane] = position;
				else
				{
					m_difficulties[4].addNoteFromMid(lane, difficultyTracker[4].notes[lane], position - difficultyTracker[4].notes[lane]);
					difficultyTracker[4].notes[lane] = UINT32_MAX;
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

	// Go through each difficulty track and validate forcing markers
	for (auto& diff : m_difficulties)
		if (diff.hasNotes())
		{
			auto iter = diff.m_notes.begin();
			auto prevIter = diff.m_notes.begin();
			iter->second.m_isForced = GuitarNote<5>::ForceStatus::UNFORCED;
			while (++iter != diff.m_notes.end())
			{
				if (iter->second.m_isForced != GuitarNote<5>::ForceStatus::UNFORCED)
				{
					if (iter->second == prevIter->second)
						iter->second.m_isForced = GuitarNote<5>::ForceStatus::UNFORCED;
					// Naturally a strum
					else if (iter->second.getNumActiveColors() >= 2 || iter->first > prevIter->first + Hittable::getForceThreshold())
					{
						if (iter->second.m_isForced == GuitarNote<5>::ForceStatus::HOPO_OFF)
							iter->second.m_isForced = GuitarNote<5>::ForceStatus::UNFORCED;
					}
					// Naturally a hopo
					else if (iter->second.m_isForced == GuitarNote<5>::ForceStatus::HOPO_ON)
						iter->second.m_isForced = GuitarNote<5>::ForceStatus::UNFORCED;
				}
				++prevIter;
			}
		}
}

template<>
void NodeTrack<GuitarNote<6>>::load_midi(const unsigned char* currPtr, const unsigned char* const end)
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
						{
							uint32_t val = currPtr[6] ? 0 : UINT32_MAX;
							for (auto& diff : difficultyTracker)
								diff.sliderNotes = val;
						}
					}
					else if (currPtr[5] == 4)
						difficultyTracker[3 - currPtr[4]].sliderNotes = currPtr[6] ? 0 : UINT32_MAX;
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
				if (lane == 7)
					difficultyTracker[diff].hopoOn = syntax == 0x90 && velocity > 0;
				else if (lane == 8)
					difficultyTracker[diff].hopoOff = syntax == 0x90 && velocity > 0;
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

					if (syntax == 0x90 && velocity > 0)
					{
						difficultyTracker[diff].notes[lane] = position;
							
						if (difficultyTracker[diff].sliderNotes &&
							!m_difficulties[diff].m_notes.contains(position))
							m_difficulties[diff].modifyNote(position, 'T', false);
					}
					else
					{
						if (!m_difficulties[diff].m_notes.contains(difficultyTracker[diff].notes[lane]))
						{
							if (difficultyTracker[diff].hopoOn)
								m_difficulties[diff].modifyNote(difficultyTracker[diff].notes[lane], '<');

							if (difficultyTracker[diff].hopoOff)
								m_difficulties[diff].modifyNote(difficultyTracker[diff].notes[lane], '>');
						}

						m_difficulties[diff].addNoteFromMid(lane, difficultyTracker[diff].notes[lane], position - difficultyTracker[diff].notes[lane]);
						difficultyTracker[diff].notes[lane] = UINT32_MAX;
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
			else if (120 <= note && note < 125)
			{
				char lane = note - 120;
				if (syntax == 0x90 && velocity > 0)
					difficultyTracker[4].notes[lane] = position;
				else
					m_difficulties[4].addNoteFromMid(lane, difficultyTracker[4].notes[lane], position - difficultyTracker[4].notes[lane]);
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

	// Go through each difficulty track and validate forcing markers
	for (auto& diff : m_difficulties)
		if (diff.hasNotes())
		{
			auto iter = diff.m_notes.begin();
			auto prevIter = diff.m_notes.begin();
			iter->second.m_isForced = GuitarNote<6>::ForceStatus::UNFORCED;
			while (++iter != diff.m_notes.end())
			{
				if (iter->second.m_isForced != GuitarNote<6>::ForceStatus::UNFORCED)
				{
					if (iter->second == prevIter->second)
						iter->second.m_isForced = GuitarNote<6>::ForceStatus::UNFORCED;
					// Naturally a strum
					else if (iter->second.getNumActiveColors() >= 2 || iter->first > prevIter->first + Hittable::getForceThreshold())
					{
						if (iter->second.m_isForced == GuitarNote<6>::ForceStatus::HOPO_OFF)
							iter->second.m_isForced = GuitarNote<6>::ForceStatus::UNFORCED;
					}
					// Naturally a hopo
					else if (iter->second.m_isForced == GuitarNote<6>::ForceStatus::HOPO_ON)
						iter->second.m_isForced = GuitarNote<6>::ForceStatus::UNFORCED;
				}
				++prevIter;
			}
		}
}

template<>
void NodeTrack<DrumNote>::load_midi(const unsigned char* currPtr, const unsigned char* const end)
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
					difficultyTracker[0].notes[0] = position;
				else
				{
					m_difficulties[0].addNoteFromMid(0, difficultyTracker[0].notes[0], position - difficultyTracker[0].notes[0]);
					m_difficulties[0].modifyColor(0, difficultyTracker[0].notes[0], '+');
					difficultyTracker[0].notes[0] = UINT32_MAX;
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
						difficultyTracker[diff].notes[lane] = position;

						if (2 <= lane && lane < 5 && !toms[lane - 2])
							m_difficulties[diff].modifyColor(lane, difficultyTracker[diff].notes[lane], 'C');

						if (velocity > 100)
							m_difficulties[diff].modifyColor(lane, difficultyTracker[diff].notes[lane], 'A');
						else if (velocity < 100)
							m_difficulties[diff].modifyColor(lane, difficultyTracker[diff].notes[lane], 'G');
					}
					else
					{
						m_difficulties[diff].addNoteFromMid(lane, difficultyTracker[diff].notes[lane], position - difficultyTracker[diff].notes[lane]);
						difficultyTracker[diff].notes[lane] = UINT32_MAX;
					}
				}
			}
			// Tom markers
			else if (110 <= note && note <= 112)
				toms[note - 110] = syntax == 0x90 && velocity > 0;
			// Fill
			else if (note == 124)
			{
				if (syntax == 0x90 && velocity > 0)
					fill = position;
				else
					m_difficulties[0].addEffect(position, new StarPowerActivation(position - fill));
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
				if (syntax == 0x90 && velocity > 0)
					difficultyTracker[0].flam = position;
				else
					m_difficulties[0].modifyNote(difficultyTracker[0].flam, 'F');
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
