#pragma once
#include <cstdio>
#include <stdint.h>
#include <fstream>
#include <sstream>
class Toggleable
{
	bool m_state = false;
public:
	void toggle() { m_state = !m_state; }
	void operator=(bool state) { m_state = state; }
	operator bool() const { return m_state; }
};

class Hittable
{
public:
	Toggleable m_isActive;
	void init(uint32_t sustain) { m_isActive = true; }
	uint32_t getSustain() const { return 0; }
	void setSustain(uint32_t sustain) {}
	operator bool() const { return m_isActive; }
};

class Fret : public Hittable
{
protected:
	// Must take sustain gap into account
	uint32_t m_sustain = 0;
public:
	void init(uint32_t sustain);
	uint32_t getSustain() const { return m_sustain; }
	void setSustain(uint32_t sustain) { if (m_isActive) m_sustain = sustain; }
	void write_chart(uint32_t position, int lane, std::ofstream& outFile, char type = 'N') const;
};

class DrumPad : public Fret
{
public:
	Toggleable m_isAccented;
	Toggleable m_isGhosted;
	Toggleable m_isFlamed;
	Fret m_lane;

	bool activateModifier(char modifier);
};

class DrumPad_Pro : public DrumPad
{
public:
	Toggleable m_isCymbal;

	bool activateModifier(char modifier);
	void write_chart(uint32_t position, int lane, std::ofstream& outFile) const;
};

class DrumPad_Bass : public Hittable
{
public:
	Toggleable m_isDoubleBass;
	void write_chart(uint32_t position, int lane, std::ofstream& outFile) const;
};

template <size_t numColors, class NoteType, class OpenType>
class Note
{
public:
	NoteType m_colors[numColors];
	OpenType m_open;

	virtual bool init_chart(size_t lane, uint32_t sustain) = 0;

	void write_chart(uint32_t position, std::ofstream& outFile) const
	{
		if (m_open)
			m_open.write_chart(position, 0, outFile);

		for (int lane = 0; lane < numColors; ++lane)
			if (m_colors[lane])
				m_colors[lane].write_chart(position, lane + 1, outFile);
	}
};

template <size_t numColors>
class GuitarNote : public Note<numColors, Fret, Fret>
{
public:
	using Note<numColors, Fret, Fret>::m_colors;
	using Note<numColors, Fret, Fret>::m_open;
	Toggleable m_isForced;
	Toggleable m_isTap;

	// Pulls values from a V1 .chart file
	// Returns whether a valid value could be utilized
	bool init_chart(size_t lane, uint32_t sustain)
	{
		switch (lane)
		{
		case 5:
			m_isForced = true;
			return true;
		case 6:
			m_isTap = true;
			return true;
		case 7:
			m_open.init(sustain);
			return true;
		default:
			return false;
		}
	}

	// write values to a .chart file
	void write_chart(const uint32_t position, std::ofstream& outFile) const
	{
		if (m_open)
			m_open.write_chart(position, 7, outFile);

		if (m_isForced)
			outFile << "  " << position << " = N 5 0\n";

		if (m_isTap && !m_open)
			outFile << "  " << position << " = N 6 0\n";
	}
};

class GuitarNote_5Fret : public GuitarNote<5>
{
public:
	bool init_chart(size_t lane, uint32_t sustain);
	void write_chart(const uint32_t position, std::ofstream& outFile) const;
};

class GuitarNote_6Fret : public GuitarNote<6>
{
public:
	bool init_chart(size_t lane, uint32_t sustain);
	void write_chart(const uint32_t position, std::ofstream& outFile) const;
};

template <size_t numColors, class DrumType>
class DrumNote : public Note<numColors, DrumType, DrumPad_Bass>
{
public:
	using Note<numColors, DrumType, DrumPad_Bass>::m_colors;
	using Note<numColors, DrumType, DrumPad_Bass>::m_open;
	Fret m_fill;

	// Pulls values from a V1 .chart file
	// Returns whether a valid value could be utilized
	bool init_chart(size_t lane, uint32_t sustain)
	{
		if (lane == 0)
			m_open.init(sustain);
		else if (lane - 1 < numColors)
			m_colors[lane - 1].init(sustain);
		else if (lane == 32)
			m_open.m_isDoubleBass = true;
		else if (lane >= 66 && lane <= 68)
			m_colors[lane - 65].activateModifier('C');
		else
			return false;
		return true;
	}
};
