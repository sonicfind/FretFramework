#include "../InstrumentalTrack_Scan.h"
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

	while (m_scanValue != 15 && traversal.next<false>())
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
