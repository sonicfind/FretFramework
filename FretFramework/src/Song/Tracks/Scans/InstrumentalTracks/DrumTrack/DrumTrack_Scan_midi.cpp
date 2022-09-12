#include "DrumTrack_Scan_Legacy.h"
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

	while (m_scanValue != 15 && traversal.next<false>())
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
	}
}

template<>
void InstrumentalTrack_Scan<DrumNote<5, DrumPad>>::scan_midi(MidiTraversal& traversal)
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
	}
}

void InstrumentalTrack_Scan<DrumNote_Legacy>::scan_midi(MidiTraversal& traversal)
{
	struct
	{
		bool activeNote = false;
		bool validated = false;
	} difficulties[4];

	while ((m_scanValue != 15 || m_drumType == UNKNOWN) && traversal.next<false>())
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
					m_drumType = FIVELANE;

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
			else if (110 <= note && note <= 112)
				m_drumType = FOURLANE_PRO;
		}
	}
}
