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
};

class DrumPad_Bass : public Hittable
{
public:
	Toggleable m_isDoubleBass;
};

template <size_t numColors, class NoteType, class OpenType>
class Note
{
public:
	NoteType m_colors[numColors];
	OpenType m_open;

	virtual bool init_chart(size_t lane, uint32_t sustain) = 0;
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
};
