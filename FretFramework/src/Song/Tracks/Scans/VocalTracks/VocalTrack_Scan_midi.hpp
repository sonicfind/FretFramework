#pragma once
#include "VocalTrack_Scan.h"
#include "../Midi/MidiFile.h"

template <>
template<int index>
void VocalTrack_Scan<1>::scan_midi(MidiTraversal& traversal)
{
	static_assert(index == 0, "Main Vocal midi scan index cannot exceed 0");

	bool lyricFound = false;
	uint32_t lyric = UINT32_MAX;
	bool phraseActive = false;
	bool vocalActive = false;
	while (m_scanValue == 0 && traversal.next<false>())
	{
		const uint32_t position = traversal.getPosition();
		const unsigned char type = traversal.getEventType();

		if (type == 0x90 || type == 0x80)
		{
			const unsigned char note = traversal.getMidiNote();
			const unsigned char velocity = traversal.getVelocity();

			if (note == 105 || note == 106)
				phraseActive = type == 0x90 && velocity > 0;
			else if (36 <= note && note < 85)
			{
				if (vocalActive)
					m_scanValue = 1;
				else if (phraseActive && type == 0x90 && velocity > 0 && lyric == position)
					vocalActive = true;
			}
		}
		else if (type < 16 && traversal.getText()[0] != '[')
		{
			lyric = position;
			lyricFound = true;
		}
	}

	if (m_scanValue == 0 && lyricFound)
		m_scanValue = 8;
}

template <>
template<int index>
void VocalTrack_Scan<3>::scan_midi(MidiTraversal& traversal)
{
	static_assert(0 <= index && index < 3, "Harmony midi scan index cannot exceed 3");
	// All over-extended vocal notes should be handled in gameplay.
	// It is thereby not necessary to check for that at scan time.
	
	bool lyricFound = false;
	uint32_t lyric = UINT32_MAX;
	bool vocalActive = false;

	if constexpr (index == 0)
	{
		m_effects.clear();

		// Only HARM1 will have to manage with polling all lyric line phrases
		uint32_t phrasePosition = UINT32_MAX;

		while (traversal.next<false>())
		{
			const uint32_t position = traversal.getPosition();
			const unsigned char type = traversal.getEventType();

			if (type == 0x90 || type == 0x80)
			{
				const unsigned char note = traversal.getMidiNote();
				const unsigned char velocity = traversal.getVelocity();

				if (note == 105 || note == 106)
				{
					if (type == 0x90 && velocity > 0)
					{
						if (phrasePosition == UINT32_MAX)
							phrasePosition = position;
					}
					else if (phrasePosition != UINT32_MAX)
					{
						if (m_effects.empty() || m_effects.back().first < phrasePosition)
							m_effects.emplace_back(phrasePosition, LyricLine(position - phrasePosition));

						phrasePosition = UINT32_MAX;
					}
				}
				else if (m_scanValue == 0)
				{
					if (36 <= note && note < 85)
					{
						if (vocalActive)
							m_scanValue = 1;
						else if (phrasePosition != UINT32_MAX && type == 0x90 && velocity > 0 && lyric == position)
							vocalActive = true;
					}
				}
			}
			else if (m_scanValue == 0 && type < 16 && traversal.getText()[0] != '[')
			{
				lyric = position;
				lyricFound = true;
			}
		}
	}
	else if (!m_effects.empty())
	{
		auto phraseIter = m_effects.begin();
		// Harmonies are able to early exit
		constexpr int finalValue = 1 << index;
		while (m_scanValue < finalValue && traversal.next<false>())
		{
			const uint32_t position = traversal.getPosition();

			while (phraseIter != m_effects.end() && position > phraseIter->first + phraseIter->second.getDuration())
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
				const unsigned char note = traversal.getMidiNote();
				if (36 <= note && note < 85)
				{
					if (vocalActive)
						m_scanValue |= finalValue;
					else if (type == 0x90 && traversal.getVelocity() > 0 && lyric == position)
						vocalActive = true;

				}
			}
			else if (type < 16 && traversal.getText()[0] != '[')
				lyric = position;
		}
	}
}
