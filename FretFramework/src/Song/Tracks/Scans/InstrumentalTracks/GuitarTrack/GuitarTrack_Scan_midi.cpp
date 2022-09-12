#include "../InstrumentalTrack_Scan.h"
#include "Chords\GuitarNote\GuitarNote_bch.hpp"
#include "Chords\GuitarNote\GuitarNote_cht.hpp"
#include "Song/Midi/MidiFile.h"
using namespace MidiFile;

template<>
void InstrumentalTrack_Scan<GuitarNote<5>>::scan_midi(MidiTraversal& traversal)
{
	struct
	{
		bool activeNote = false;
		bool validated = false;
	} difficulties[4];

	bool enhancedForEasy = false;
	while (m_scanValue != 15 && traversal.next())
	{
		const unsigned char type = traversal.getEventType();

		if (type == 0x90 || type == 0x80)
		{
			const unsigned char note = traversal.getMidiNote();
			const unsigned char velocity = traversal.getVelocity();

			// Notes
			if (59 <= note && note <= 100)
			{
				int noteValue = note - 59;
				int diff = noteValue / 12;
				// Animation
				if (note == 59 && !enhancedForEasy)
					continue;

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
		else if (type < 16 && !enhancedForEasy && (traversal.getText() == U"[ENHANCED_OPENS]" || traversal.getText() == U"ENHANCED_OPENS"))
			enhancedForEasy = true;
	}
}

template<>
void InstrumentalTrack_Scan<GuitarNote<6>>::scan_midi(MidiTraversal& traversal)
{
	struct
	{
		bool activeNote = false;
		bool validated = false;
	} difficulties[4];

	while (m_scanValue != 15 && traversal.next<false>())
	{
		const unsigned char type = traversal.getEventType();

		if (type == 0x90 || type == 0x80)
		{
			const unsigned char note = traversal.getMidiNote();
			const unsigned char velocity = traversal.getVelocity();

			// Notes
			if (58 <= note && note <= 100)
			{
				int noteValue = note - 58;
				int diff = noteValue / 12;

				if (!difficulties[diff].validated)
				{
					if (noteValue % 12 < 7)
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