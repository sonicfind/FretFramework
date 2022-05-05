#pragma once
#include <stdint.h>
#include "NoteExceptions.h"

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
	virtual void init_cht_single(const char* str) = 0;
	void init_cht_chord(const char* str)
	{
		int colors;
		int count;
		if (sscanf_s(str, " %i%n", &colors, &count) == 1)
		{
			int numAdded = 0;
			str += count;
			int lane;
			for (int i = 0;
				i < colors && sscanf_s(str, " %i%n", &lane, &count) == 1;
				++i)
			{
				str += count;
				unsigned char color = lane & 127;
				uint32_t sustain = 0;
				if (lane & 128)
				{
					if (sscanf_s(str, " %lu%n", &sustain, &count) != 1)
						throw EndofLineException();
					str += count;
				}

				if (color == 0)
				{
					m_special.init(sustain);
					++numAdded;
				}
				else if (color <= numColors)
				{
					m_colors[color - 1].init(sustain);
					++numAdded;
				}
			}

			if (numAdded == 0)
				throw InvalidNoteException();
		}
		else
			throw EndofLineException();
	}

	virtual bool modify(char modifier, bool toggle = true) = 0;
	bool modifyColor(int lane, char modifier) { return false; }
	virtual void modify_cht(const char* str) = 0;

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
	uint32_t write_notes_cht(uint32_t position, std::fstream& outFile, const char* const tabs = "\t\t") const
	{
		uint32_t numActive = getNumActiveColors();
		if (numActive == 1)
		{
			outFile << tabs << position << " = N";
			if (m_special)
				m_special.save_cht(0, outFile);
			else
			{
				int lane = 0;
				while (!m_colors[lane])
					++lane;

				m_colors[lane].save_cht(lane + 1, outFile);
			}
		}
		else
		{
			outFile << tabs << position << " = C " << numActive;
			if (m_special)
				m_special.save_cht(0, outFile);

			for (int lane = 0; lane < numColors; ++lane)
				if (m_colors[lane])
					m_colors[lane].save_cht(lane + 1, outFile);
		}
		return numActive;
	}

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

	static uint32_t getLaneSize()
	{
		return numColors;
	}
};
