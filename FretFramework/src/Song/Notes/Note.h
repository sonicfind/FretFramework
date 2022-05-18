#pragma once
#include <stdint.h>
#include "NoteExceptions.h"
#include "../TextFileTraversal.h"
#include "../BinaryFileTraversal.h"
#include <fstream>

template <int numColors, class NoteType, class SpecialType>
class Note
{
public:
	NoteType m_colors[numColors];
	SpecialType m_special;

	virtual void init(unsigned char lane, uint32_t sustain = 0)
	{
		if (lane > numColors)
			throw InvalidNoteException(lane);

		if (lane == 0)
			m_special.init(sustain);
		else
			m_colors[lane - 1].init(sustain);
	}

	void init_single(TextTraversal& traversal);
	void init_chord(TextTraversal& traversal);
	void init_single(BinaryTraversal& traversal);
	void init_chord(BinaryTraversal& traversal);

	virtual void modify(char modifier, unsigned char lane = 0) {}
	virtual void modify_binary(char modifier, unsigned char lane = 0) {}
	void modify(TextTraversal& traversal);
	void modify(BinaryTraversal& traversal);

protected:
	uint32_t write_notes(uint32_t position, std::fstream& outFile, const char* const tabs = "\t\t") const;
	char write_notes(char*& dataPtr) const;

public:
	virtual void save_cht(uint32_t position, std::fstream& outFile) const = 0;
	virtual uint32_t save_bch(uint32_t position, std::fstream& outFile) const = 0;

	virtual uint32_t getNumActive() const
	{
		uint32_t num = m_special ? 1 : 0;
		for (int lane = 0; lane < numColors; ++lane)
			if (m_colors[lane])
				++num;
		return num;
	}

	bool operator==(const Note& note) const
	{
		if (m_special != note.m_special)
			return false;
		else
		{
			for (int i = 0; i < numColors; ++i)
				if (m_colors[i] != note.m_colors[i])
					return false;
			return true;
		}
	}

	bool operator!=(const Note& note) const
	{
		return !operator==(note);
	}

	void operator*=(float multiplier)
	{
		for (auto& col : m_colors)
			col *= multiplier;

		m_special *= multiplier;
	}

	static uint32_t getLaneSize()
	{
		return numColors;
	}
};
