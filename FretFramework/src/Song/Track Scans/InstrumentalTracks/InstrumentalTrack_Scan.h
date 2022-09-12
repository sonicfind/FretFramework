#pragma once
#include <iostream>
#include "../NoteTrack_Scan.h"
#include "FileTraversal/MidiFileTraversal.h"

template <class T>
class InstrumentalTrack_Scan : public NoteTrack_Scan
{
	bool scanDifficulty(TextTraversal& traversal);
	bool scanDifficulty(BCHTraversal& traversal);

public:
	using NoteTrack_Scan::NoteTrack_Scan;
	void scan_chart_V1(int diff, TextTraversal& traversal);
	void scan_cht(TextTraversal& traversal);
	void scan_bch(BCHTraversal& traversal);
	void scan_midi(MidiTraversal& traversal);

	std::string toString()
	{
		std::string str;
		int val = m_scanValue;
		if (val >= 8)
		{
			str += "Expert ";
			val -= 8;
		}
		if (val >= 4)
		{
			str += "Hard ";
			val -= 4;
		}
		if (val >= 2)
		{
			str += "Medium ";
			val -= 2;
		}
		if (val == 1)
			str += "Easy";
		return str;
	}
};
