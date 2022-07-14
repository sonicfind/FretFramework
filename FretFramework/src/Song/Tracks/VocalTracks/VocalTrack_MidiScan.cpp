#include "VocalTrack_cht.hpp"
#include "VocalTrack_bch.hpp"

template <>
template <>
void VocalTrack_Scan<1>::scan_midi<0>(MidiTraversal& traversal)
{
	uint32_t lyric = UINT32_MAX;
	bool phraseActive = false;
	bool percActive = false;
	bool vocalActive = false;
	while (traversal.next() && traversal.getEventType() != 0x2F && m_scanValue == 0)
	{
		const uint32_t position = traversal.getPosition();
		const unsigned char type = traversal.getEventType();

		if (type == 0x90 || type == 0x80)
		{
			const unsigned char note = traversal.getMidiNote();
			const unsigned char velocity = traversal.getVelocity();

			if (note == 105 || note == 106)
				phraseActive = type == 0x90 && velocity > 0;
			else if (36 <= note && note < 85 && !percActive)
			{
				if (vocalActive)
					m_scanValue = 1;
				else if (phraseActive && type == 0x90 && velocity > 0 && lyric == position)
					vocalActive = true;
			}
			else if (note == 96 && !vocalActive)
			{
				if (percActive)
					m_scanValue = 1;
				else if (phraseActive && type == 0x90 && velocity > 0)
					percActive = true;
			}
		}
		else if (type < 16)
		{
			if (traversal[0] != '[')
				lyric = position;
		}
	}
}
