#pragma once
#include <iostream>
#include "Difficulty/Difficulty.h"

template <class T>
class BasicTrack
{
public:
	const char* const m_name;
	Difficulty<T> m_difficulties[5] = { { "[Easy]" }, { "[Medium]" }, { "[Hard]" }, { "[Expert]" }, { "[BRE]" } };

	BasicTrack(const char* name)
		: m_name(name) {}

	Difficulty<T>& operator[](size_t i)
	{
		if (i >= 5)
			throw std::out_of_range("Max difficulty index is 4");
		return m_difficulties[i];
	}

	const Difficulty<T>& operator[](size_t i) const
	{
		if (i >= 5)
			throw std::out_of_range("Max difficulty index is 4");
		return m_difficulties[i];
	}

	void load_cht(std::fstream& inFile);
	void save_cht(std::fstream& outFile) const;
	void load_midi(const unsigned char* currPtr, const unsigned char* const end);
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
