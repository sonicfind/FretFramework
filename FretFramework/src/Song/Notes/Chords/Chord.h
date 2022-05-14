#pragma once
#include "Base Nodes/Sustainable.h"
#include "Note.h"

template <int numColors>
class Chord : public Note<numColors, Sustainable, Sustainable>
{
public:
	using Note<numColors, Sustainable, Sustainable>::m_colors;
	using Note<numColors, Sustainable, Sustainable>::m_special;
	enum class ForceStatus
	{
		UNFORCED,
		FORCED,
		HOPO_ON,
		HOPO_OFF
	} m_isForced = ForceStatus::UNFORCED;
	Toggleable m_isTap;

private:
	// Checks modifier value from a v1 .chart file
	bool checkModifiers(int lane, uint32_t sustain)
	{
		switch (lane)
		{
		case 5:
			m_isForced = ForceStatus::FORCED;
			break;
		case 6:
			m_isTap = true;
			break;
		case 7:
			m_special.init(sustain);
			break;
		default:
			return false;
		}
		return true;
	}

public:
	bool init(int lane, uint32_t sustain = 0)
	{
		if (lane > numColors)
			return false;

		if (lane == 0)
		{
			m_special.init(sustain);
			static const Sustainable replacement[numColors];
			memcpy(m_colors, replacement, sizeof(Sustainable) * numColors);
		}
		else
		{
			m_colors[lane - 1].init(sustain);
			m_special = Sustainable();
		}
		return true;
	}

	void init_chartV1(int lane, uint32_t sustain);
	void init_cht_single(TextTraversal& traversal);
	void init_cht_chord(TextTraversal& traversal);

	bool modify(char modifier, bool toggle = true)
	{
		switch (modifier)
		{
		case 'F':
			switch (m_isForced)
			{
			case ForceStatus::UNFORCED:
				m_isForced = ForceStatus::FORCED;
				break;
			default:
				m_isForced = ForceStatus::UNFORCED;
				break;
			}
			break;
		case 'T':
			if (toggle)
				m_isTap.toggle();
			else
				m_isTap = true;
			break;
		case '<':
			m_isForced = ForceStatus::HOPO_ON;
			break;
		case '>':
			m_isForced = ForceStatus::HOPO_OFF;
			break;
		default:
			return false;
		}
		return true;
	}

	void modify_cht(TextTraversal& traversal);
	// write values to a V2 .chart file
	void save_cht(const uint32_t position, std::fstream& outFile) const;

	uint32_t getLongestSustain() const
	{
		if (m_special)
			return m_special.getSustain();
		else
		{
			uint32_t sustain = 0;
			for (const auto& color : m_colors)
				if (color && color.getSustain() > sustain)
					sustain = color.getSustain();
			return sustain;
		}
	}
};

template<>
void Chord<6>::init_chartV1(int lane, uint32_t sustain);
