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

	void scan_chart_V1(int diff, TextTraversal& traversal, std::unique_ptr<NoteTrack_Scan>& track) const;
	void load_chart_V1(int diff, TextTraversal& traversal);

	void scan_cht(TextTraversal& traversal, std::unique_ptr<NoteTrack_Scan>& track) const;
	void load_cht(TextTraversal& traversal);
	void save_cht(std::fstream& outFile) const;

	void scan_bch(BCHTraversal& traversal, std::unique_ptr<NoteTrack_Scan>& track) const;
	void load_bch(BCHTraversal& traversal);
	bool save_bch(std::fstream& outFile) const;

	void scan_midi(MidiTraversal& traversal, std::unique_ptr<NoteTrack_Scan>& track) const
	{
		if (track == nullptr)
			track = std::make_unique<InstrumentalTrack_Scan<T>>();
		reinterpret_cast<InstrumentalTrack_Scan<T>*>(track.get())->scan_midi(traversal);
	}

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
