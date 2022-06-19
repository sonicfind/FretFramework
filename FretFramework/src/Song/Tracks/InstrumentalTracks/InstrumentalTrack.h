#pragma once
#include <iostream>
#include "../NoteTrack.h"
#include "Difficulty/Difficulty.h"
#include "FileTraversal/MidiFileTraversal.h"

template <class T>
class InstrumentalTrack_Scan : public NoteTrack_Scan
{
protected:
	Difficulty_Scan<T> m_difficulties[5] = { { "[Easy]", 0 }, { "[Medium]", 1 }, { "[Hard]", 2 }, { "[Expert]", 3 }, { "[BRE]", 4 } };

public:
	using NoteTrack_Scan::NoteTrack_Scan;
	void scan_chart_V1(int diff, TextTraversal& traversal);
	void scan_cht(TextTraversal& traversal);
	void scan_bch(BCHTraversal& traversal);
	void scan_midi(MidiTraversal& traversal);
};

template <class T>
class InstrumentalTrack : public NoteTrack
{
	friend class DrumTrackConverter;
protected:
	Difficulty<T> m_difficulties[5] = { { "[Easy]", 0 }, { "[Medium]", 1 }, { "[Hard]", 2 }, { "[Expert]", 3 }, { "[BRE]", 4 } };

public:
	using NoteTrack::NoteTrack;

	template <typename U>
	InstrumentalTrack& operator=(const InstrumentalTrack<U>& track) { return *this; }

	void scan_chart_V1(int diff, TextTraversal& traversal, NoteTrack_Scan*& track) const;
	void load_chart_V1(int diff, TextTraversal& traversal);

	void scan_cht(TextTraversal& traversal, NoteTrack_Scan*& track) const;
	void load_cht(TextTraversal& traversal);
	void save_cht(std::fstream& outFile) const;

	void scan_bch(BCHTraversal& traversal, NoteTrack_Scan*& track) const;
	void load_bch(BCHTraversal& traversal);
	bool save_bch(std::fstream& outFile) const;

	void scan_midi(MidiTraversal& traversal, NoteTrack_Scan*& track) const;
	void load_midi(MidiTraversal& traversal);
	void save_midi(const char* const name, std::fstream& outFile) const;
	
	// Returns whether any difficulty in this track contains notes
	// ONLY checks for notes
	bool hasNotes() const
	{
		for (const auto& diff : m_difficulties)
			if (diff.hasNotes())
				return true;
		return false;
	}

	// Returns whether any difficulty in this track contains notes, effects, soloes, or other events
	bool occupied() const
	{
		for (const auto& diff : m_difficulties)
			if (diff.occupied())
				return true;
		return false;
	}

	void clear()
	{
		for (auto& diff : m_difficulties)
			diff.clear();
	}

	void adjustTicks(float multiplier)
	{
		for (auto& diff : m_difficulties)
			if (diff.occupied())
				diff.adjustTicks(multiplier);
	}
};
