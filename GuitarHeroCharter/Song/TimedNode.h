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
	void toggle() { m_isActive.toggle(); }
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
	void save_chart(uint32_t position, int lane, std::fstream& outFile) const
	{
		outFile << "  " << position << " = N " << lane << ' ' << m_sustain << "\n";
	}
};

class Modifiable : public Hittable
{
	virtual bool modify(char modifier) = 0;
public:
	void save_chart(uint32_t position, int lane, std::fstream& outFile) const;
};

class DrumPad : public Modifiable
{
public:
	Toggleable m_isAccented;
	Toggleable m_isGhosted;
	Fret m_lane;

	bool modify(char modifier);
};

class DrumPad_Pro : public DrumPad
{
private:
	// Ensures a pad's tom status is always secured if designated so
	// by a .mid file
	bool m_lockTom = false;

public:
	Toggleable m_isCymbal;

	bool modify(char modifier);
	void save_chart(uint32_t position, int lane, std::fstream& outFile) const;
};

class DrumPad_Bass : public Modifiable
{
public:
	Toggleable m_isDoubleBass;
	bool modify(char modifier);
	void save_chart(uint32_t position, int lane, std::fstream& outFile) const;
};

template <size_t numColors, class NoteType, class OpenType>
class Note
{
public:
	NoteType m_colors[numColors];
	OpenType m_open;

	virtual bool initFromChartV1(size_t lane, uint32_t sustain) = 0;

	virtual bool init(size_t lane, uint32_t sustain)
	{
		if (lane > numColors)
			return false;

		if (lane == 0)
			m_open.init(sustain);
		else
			m_colors[lane - 1].init(sustain);

		return true;
	}

	virtual bool initFromMid(size_t lane, uint32_t sustain)
	{
		return init(lane, sustain);
	}

	virtual bool init_chart2_modifier(std::stringstream& ss) = 0;
	virtual bool modify(char modifier, bool toggle = true) = 0;
	virtual bool modifyColor(int lane, char modifier) { return false; }

	void save_chart(uint32_t position, std::fstream& outFile) const
	{
		if (m_open)
			m_open.save_chart(position, 0, outFile);

		for (int lane = 0; lane < numColors; ++lane)
			if (m_colors[lane])
				m_colors[lane].save_chart(position, lane + 1, outFile);
	}
	constexpr static bool isGHL()
	{
		return false;
	}

	constexpr static bool isDrums()
	{
		return false;
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
	bool initFromChartV1(size_t lane, uint32_t sustain)
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

	bool init(size_t lane, uint32_t sustain = 0)
	{
		if (!Note<numColors, Fret, Fret>::init(lane, sustain))
		{
			// Should only occur if set by hopo_on or hopo_off midi events
			m_isForced = true;
			return false;
		}

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
		return modify(modifier);
	}

	bool modify(char modifier, bool toggle = true)
	{
		if (modifier == 'F')
		{
			if (toggle)
				m_isForced.toggle();
			else
				m_isForced = true;
		}	
		else if (modifier == 'T' && !m_open)
		{
			if (toggle)
				m_isTap.toggle();
			else
				m_isTap = true;
		}
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
	bool initFromChartV1(size_t lane, uint32_t sustain);
};

class GuitarNote_6Fret : public GuitarNote<6>
{
public:
	bool initFromChartV1(size_t lane, uint32_t sustain);

	constexpr static bool isGHL()
	{
		return true;
	}
};

template <size_t numColors, class DrumType>
class DrumNote : public Note<numColors, DrumType, DrumPad_Bass>
{
public:
	using Note<numColors, DrumType, DrumPad_Bass>::m_colors;
	using Note<numColors, DrumType, DrumPad_Bass>::m_open;
	Toggleable m_isFlamed;

	// Pulls values from a V1 .chart file
	// Returns whether a valid value could be utilized
	bool initFromChartV1(size_t lane, uint32_t sustain)
	{
		if (lane == 0)
			m_open.init(sustain);
		else if (lane - 1 < numColors)
			m_colors[lane - 1].init(sustain);
		else if (lane == 32)
			m_open.m_isDoubleBass = true;
		else if (lane >= 66 && lane <= 68)
			m_colors[lane - 65].modify('C');
		else
			return false;
		return true;
	}

	bool init_chart2_modifier(std::stringstream& ss)
	{
		char modifier;
		ss >> modifier;
		if (!modify(modifier))
		{
			int lane = 0;
			switch (modifier)
			{
			case 'a':
			case 'A':
			case 'g':
			case 'G':
				ss >> lane;
				if (!ss)
					return false;
				__fallthrough;
			default:
				return m_colors[lane].modify(modifier);
			}
		}
		return true;
	}

	bool initFromMid(size_t lane, uint32_t sustain)
	{
		if (Note<numColors, DrumType, DrumPad_Bass>::init(lane, sustain))
		{
			if (lane >= 2)
				m_colors[lane - 1].modify('C');
			return true;
		}
		else
			return false;
	}

	bool modify(char modifier, bool toggle = true)
	{
		if (modifier == 'F')
		{
			if (!m_isFlamed || !toggle)
			{
				int numActive = 0;
				for (int i = 0; i < numColors; ++i)
					if (m_colors[i])
						++numActive;

				if (numActive < 2)
					m_isFlamed = true;
				else
					return false;
			}
			else
				m_isFlamed = false;
		}
		return true;
	}

	bool modifyColor(int lane, char modifier)
	{
		if (lane == 0)
			return m_open.modify(modifier);
		else
			return m_colors[lane - 1].modify(modifier);
	}

	void save_chart(const uint32_t position, std::fstream& outFile) const
	{
		Note<numColors, DrumType, DrumPad_Bass>::save_chart(position, outFile);
		if (m_isFlamed)
			outFile << "  " << position << " = M F\n";
	}

	constexpr static bool isDrums()
	{
		return true;
	}
};
