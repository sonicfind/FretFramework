#include "VocalTrack_cht.hpp"
#include "VocalTrack_bch.hpp"

template <>
void VocalTrack_Scan<1>::scan_midi(int index, MidiTraversal& traversal)
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

template <>
void VocalTrack_Scan<3>::scan_midi(int index, MidiTraversal& traversal)
{
	uint32_t lyric = UINT32_MAX;
	bool vocalActive = false;

	if (index == 0)
	{
		for (auto& vec : m_effects)
			for (Phrase* phr : vec.second)
				delete phr;
		m_effects.clear();
		std::vector<Phrase*> phraseNode;

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
						if (m_effects.empty() || m_effects.back().first != phrasePosition)
						{
							phraseNode.push_back(new LyricLine(position - phrasePosition));
							m_effects.emplace_back(phrasePosition, std::move(phraseNode));
						}
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
					else if (note == 96)
					{
						if (percActive)
							m_scanValue = 1;
						else if (phrasePosition != UINT32_MAX && type == 0x90 && velocity > 0)
							percActive = true;
					}
				}
			}
			else if (m_scanValue == 0 && type < 16)
			{
				if (traversal[0] != '[')
					lyric = position;
			}
		}
	}
	else if (!m_effects.empty())
	{
		auto phraseIter = m_effects.begin();
		// Harmonies are able to early exit
		const int finalValue = 1 << index;
		while (traversal.next() && traversal.getEventType() != 0x2F && m_scanValue < finalValue)
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
				const unsigned char note = traversal.getMidiNote();
				if (36 <= note && note < 85)
				{
					if (vocalActive)
						m_scanValue |= finalValue;
					else
					{
						const unsigned char velocity = traversal.getVelocity();
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
}
