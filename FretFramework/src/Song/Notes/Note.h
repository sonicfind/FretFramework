#pragma once
#include <stdint.h>
#include "NoteExceptions.h"
#include "../TextFileManip.h"

template <size_t numColors, class NoteType, class SpecialType>
class Note
{
public:
	NoteType m_colors[numColors];
	SpecialType m_special;

	bool init(size_t lane, uint32_t sustain = 0)
	{
		if (lane > numColors)
			return false;

		if (lane == 0)
			m_special.init(sustain);
		else
			m_colors[lane - 1].init(sustain);

		return true;
	}

	void init_chartV1(int lane, uint32_t sustain) {}
	virtual void init_cht_single(TextTraversal& traversal) = 0;
	void init_cht_chord(TextTraversal& traversal);

	virtual bool modify(char modifier, bool toggle = true) = 0;
	bool modifyColor(int lane, char modifier) { return false; }
	virtual void modify_cht(TextTraversal& traversal) = 0;

	virtual uint32_t getNumActiveColors() const
	{
		uint32_t num = m_special ? 1 : 0;
		for (int lane = 0; lane < numColors; ++lane)
			if (m_colors[lane])
				++num;
		return num;
	}

	virtual void save_cht(uint32_t position, std::fstream& outFile) const = 0;

protected:
	uint32_t write_notes_cht(uint32_t position, std::fstream& outFile, const char* const tabs = "\t\t") const;

public:
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
