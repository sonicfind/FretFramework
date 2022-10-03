#pragma once
#include "Song/Track Scans/NoteTrack_Scan.h"
#include "FileTraversal/MidiFileTraversal.h"
#include "Vocals/Vocal.h"
#include "Song/Phrases/Phrases.h"

template <int numTracks>
class VocalTrack_Scan : public NoteTrack_Scan
{
	std::vector<std::pair<uint32_t, LyricLine>>* m_lyricLines = nullptr;

public:
	void scan_cht(TextTraversal& traversal);
	void scan_bch(BCHTraversal& traversal);
	template<int index>
	void scan_midi(MidiTraversal& traversal);
	std::string toString()
	{
		if (m_scanValue == 8)
			return "Non-playable lyrics present";
		else if constexpr (numTracks == 1)
			return "Main Vocals";
		else
		{
			std::string str;
			if (m_scanValue & 1)
				str += "Harm1 ";
			if (m_scanValue & 2)
				str += "Harm2 ";
			if (m_scanValue & 4)
				str += "Harm3";
			return str;
		}
	}

	void clearEffects() { delete m_lyricLines; m_lyricLines = nullptr; }
};

template <>
template<int index>
void VocalTrack_Scan<1>::scan_midi(MidiTraversal& traversal);

template <>
template<int index>
void VocalTrack_Scan<3>::scan_midi(MidiTraversal& traversal);
