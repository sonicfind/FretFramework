#pragma once
#include <iostream>
#include "../NoteTrack.h"
#include "Difficulty/Difficulty.h"

template <class T>
class InstrumentalTrack : public NoteTrack
{
	friend class DrumTrackConverter;
protected:
	Difficulty<T> m_difficulties[5] = { { "[Easy]", 0 }, { "[Medium]", 1 }, { "[Hard]", 2 }, { "[Expert]", 3 }, { "[BRE]", 4 } };

public:
	using NoteTrack::NoteTrack;

	void load_chart_V1(int diff, TextTraversal& traversal);
	void load_cht(TextTraversal& traversal);
	void save_cht(std::fstream& outFile) const;

	void load_bch(BinaryTraversal& traversal);
	bool save_bch(std::fstream& outFile) const;

	void load_midi(const unsigned char* current, const unsigned char* const end);
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