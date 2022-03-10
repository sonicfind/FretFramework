#pragma once
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
	void save_chart(uint32_t position, int lane, std::fstream& outFile, char type = 'N') const;
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
	void save_chart(uint32_t position, int lane, std::fstream& outFile) const;
};

class DrumPad_Bass : public Hittable
{
public:
	Toggleable m_isDoubleBass;
	void save_chart(uint32_t position, int lane, std::fstream& outFile) const;
};

template <size_t numColors, class NoteType, class OpenType>
class Note
{
public:
	NoteType m_colors[numColors];
	OpenType m_open;

	virtual bool init_chart(size_t lane, uint32_t sustain) = 0;

	bool init_chart2(size_t lane, uint32_t sustain)
	{
		if (lane > numColors)
			return false;

		if (lane == 0)
			m_open.init(sustain);
		else
			m_colors[lane - 1].init(sustain);

		return true;
	}

	virtual bool init_chart2_modifier(std::stringstream& ss) = 0;

	void save_chart(uint32_t position, std::fstream& outFile) const
	{
		if (m_open)
			m_open.save_chart(position, 0, outFile);

		for (int lane = 0; lane < numColors; ++lane)
			if (m_colors[lane])
				m_colors[lane].save_chart(position, lane + 1, outFile);
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

	bool init_chart2(size_t lane, uint32_t sustain)
	{
		if (!Note<numColors, Fret, Fret>::init_chart2(lane, sustain))
			return false;

		// A colored fret can't exist alongside the open note and vice versa
		if (lane == 0)
		{
			static const Fret replacement[numColors];
			memcpy(m_colors, replacement, sizeof(Fret) * numColors);
		}
		else
			m_open = Fret();
		return true;
	}

	bool init_chart2_modifier(std::stringstream& ss)
	{
		char modifier;
		ss >> modifier;
		if (modifier == 'F')
			m_isForced = true;
		else if (modifier == 'T' && !m_open)
			m_isTap = true;
		else
			return false;
		return true;
	}

	// write values to a V2 .chart file
	void save_chart(const uint32_t position, std::fstream& outFile) const
	{
		Note<numColors, Fret, Fret>::save_chart(position, outFile);
		if (m_isForced)
			outFile << "  " << position << " = M F\n";

		if (m_isTap && !m_open)
			outFile << "  " << position << " = M T\n";
	}
};

class GuitarNote_5Fret : public GuitarNote<5>
{
public:
	bool init_chart(size_t lane, uint32_t sustain);
};

class GuitarNote_6Fret : public GuitarNote<6>
{
public:
	bool init_chart(size_t lane, uint32_t sustain);
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

	bool init_chart2_modifier(std::stringstream& ss)
	{
		char modifier;
		ss >> modifier;
		switch (modifier)
		{
		case 'k':
		case 'K':
			m_open.m_isActive = true;
			return true;
		case 'x':
		case 'X':
			m_open.m_isActive = true;
			m_open.m_isDoubleBass = true;
			return true;
		case 'a':
		case 'A':
		{
			uint32_t sustain;
			ss >> sustain;
			m_fill.init(sustain);
			return true;
		}
		default:
		{
			int lane;
			ss >> lane;
			return ss && m_colors[lane - 1].activateModifier(modifier);
		}
		}
	}

	void save_chart(const uint32_t position, std::fstream& outFile) const
	{
		Note<numColors, DrumType, DrumPad_Bass>::save_chart(position, outFile);
		if (m_fill)
			outFile << "  " << position << " = M A " << m_fill.getSustain() << '\n';
	}
};
