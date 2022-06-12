#include "VocalTrack_cht.hpp"
#include "VocalTrack_bch.hpp"

template <>
int VocalTrack<1>::scan_midi(int index, MidiTraversal& traversal)
{
	uint32_t lyric = UINT32_MAX;
	bool phraseActive = false;
	bool percActive = false;
	bool vocalActive = false;
	while (traversal.next() && traversal.getEventType() != 0x2F)
	{
		const uint32_t position = traversal.getPosition();
		const unsigned char type = traversal.getEventType();

		if (type == 0x90 || type == 0x80)
		{
			const unsigned char note = traversal.extractChar();
			const unsigned char velocity = traversal.extractChar();

			if (note == 105 || note == 106)
				phraseActive = type == 0x90 && velocity > 0;
			else if (36 <= note && note < 85 && !percActive)
			{
				if (vocalActive)
					return 1;
				else if (phraseActive && type == 0x90 && velocity > 0 && lyric == position)
					vocalActive = true;
			}
			else if (note == 96 && !vocalActive)
			{
				if (percActive)
					return 1;
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

	return 0;
}

template <>
int VocalTrack<3>::scan_midi(int index, MidiTraversal& traversal)
{
	uint32_t lyric = UINT32_MAX;
	bool vocalActive = false;

	if (index == 0)
	{
		m_effects.clear();

		int ret = 0;
		// Only HARM1 will have to manage with polling all lyric line phrases
		uint32_t phrasePosition = UINT32_MAX;
		// Percussion notes are only valid in HARM1
		bool percActive = false;

		while (traversal.next() && traversal.getEventType() != 0x2F)
		{
			const uint32_t position = traversal.getPosition();
			const unsigned char type = traversal.getEventType();

			if (type == 0x90 || type == 0x80)
			{
				const unsigned char note = traversal.extractChar();
				const unsigned char velocity = traversal.extractChar();

				if (note == 105 || note == 106)
				{
					if (type == 0x90 && velocity > 0)
					{
						if (phrasePosition == UINT32_MAX)
							phrasePosition = position;
					}
					else if (phrasePosition != UINT32_MAX)
					{
						addPhrase(phrasePosition, new LyricLine(position - phrasePosition));
						phrasePosition = UINT32_MAX;
					}
				}
				else if (ret == 0)
				{
					if (36 <= note && note < 85)
					{
						if (vocalActive)
							ret = 1;
						else if (phrasePosition != UINT32_MAX && type == 0x90 && velocity > 0 && lyric == position)
							vocalActive = true;
					}
					else if (note == 96)
					{
						if (percActive)
							ret = 1;
						else if (phrasePosition != UINT32_MAX && type == 0x90 && velocity > 0)
							percActive = true;
					}
				}
			}
			else if (ret == 0 && type < 16)
			{
				if (traversal[0] != '[')
					lyric = position;
			}
		}
		return ret;
	}
	else if (!m_effects.empty())
	{
		auto phraseIter = m_effects.begin();
		while (traversal.next() && traversal.getEventType() != 0x2F)
		{
			const uint32_t position = traversal.getPosition();

			while (phraseIter != m_effects.end() && position > phraseIter->first + phraseIter->second.front()->getDuration())
				++phraseIter;

			if (!vocalActive)
			{
				if (phraseIter == m_effects.end())
					break;
				else if (position < phraseIter->first)
					continue;
			}

			const unsigned char type = traversal.getEventType();
			if (type == 0x90 || type == 0x80)
			{
				const unsigned char note = traversal.extractChar();
				if (36 <= note && note < 85)
				{
					if (vocalActive)
						// Harmonies are able to early exit
						return 1 << index;
					else
					{
						const unsigned char velocity = traversal.extractChar();
						if (type == 0x90 && velocity > 0 && lyric == position)
							vocalActive = true;
					}
					
				}
			}
			else if (type < 16)
			{
				if (traversal[0] != '[')
					lyric = position;
			}
		}
	}

	return 0;
}
