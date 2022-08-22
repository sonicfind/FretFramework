#pragma once
#include "InstrumentalNote.h"
#include "Base Nodes/Sustainable.h"

template <int numColors>
class GuitarNote : public InstrumentalNote<numColors, Sustainable, Sustainable>
{
public:
	using InstrumentalNote<numColors, Sustainable, Sustainable>::m_colors;
	using InstrumentalNote<numColors, Sustainable, Sustainable>::m_special;

	enum class ForceStatus
	{
		UNFORCED,
		FORCED,
		HOPO_ON,
		HOPO_OFF
	} m_isForced = ForceStatus::UNFORCED;
	Toggleable m_isTap;

private:
	unsigned char m_previousInsertedColorIndex = numColors + 1;
	// Checks modifier value from a v1 .chart file
	bool checkModifiers(unsigned char lane)
	{
		switch (lane)
		{
		case 5:
			m_isForced = ForceStatus::FORCED;
			break;
		case 6:
			m_isTap = true;
			break;
		default:
			return false;
		}
		return true;
	}

	static constexpr Sustainable replacement[numColors];

public:
	constexpr explicit GuitarNote() : InstrumentalNote<numColors, Sustainable, Sustainable>() {}
	constexpr void init(const unsigned char lane, const uint32_t sustain)
	{
		InstrumentalNote<numColors, Sustainable, Sustainable>::init(lane, sustain);

		if (m_previousInsertedColorIndex <= numColors)
		{
			if (m_previousInsertedColorIndex > 0)
			{
				if (lane == 0)
					memcpy(m_colors, replacement, sizeof(m_colors));
			}
			else if (lane > 0)
				m_special = Sustainable();
		}

		m_previousInsertedColorIndex = lane;
	}

	void init_chartV1(const unsigned char lane, const uint32_t sustain);

	using InstrumentalNote<numColors, Sustainable, Sustainable>::modify;
	void modify(char modifier, unsigned char lane = 0)
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
			m_isTap.toggle();
			break;
		case '<':
			m_isForced = ForceStatus::HOPO_ON;
			break;
		case '>':
			m_isForced = ForceStatus::HOPO_OFF;
			break;
		}
	}

	void modify_binary(char modifier, unsigned char lane = 0)
	{
		if (modifier & 1)
			m_isForced = ForceStatus::FORCED;
		else if (modifier & 2)
			m_isForced = ForceStatus::HOPO_ON;
		else if (modifier & 4)
			m_isForced = ForceStatus::HOPO_OFF;

		if (modifier & 8)
			m_isTap = true;
	}

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

protected:
	int write_modifiers_single(std::stringstream& buffer) const;
	int write_modifiers_chord(std::stringstream& buffer) const;
	void write_modifiers_single(char*& buffer) const;
	char write_modifiers_chord(char*& buffer) const;
};
