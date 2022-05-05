#pragma once
#include "Base Nodes/Sustainable.h"
#include "Note.h"

template <size_t numColors>
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
	bool checkModifiers(size_t lane, uint32_t sustain)
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
	bool init(size_t lane, uint32_t sustain = 0)
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

	// Pulls values from a V1 .chart file
	// Returns whether a valid value could be utilized
	void init_chartV1(int lane, uint32_t sustain)
	{
		if (lane < 5)
			m_colors[lane].init(sustain);
		else if (!checkModifiers(lane, sustain))
			throw InvalidNoteException(lane);
	}

	void init_cht_single(const char* str)
	{
		// Read note
		int lane, count;
		if (sscanf_s(str, " %i%n", &lane, &count) != 1)
			throw EndofLineException();

		str += count;
		unsigned char color = lane & 127;
		uint32_t sustain = 0;
		if (lane & 128)
		{
			if (sscanf_s(str, " %lu%n", &sustain, &count) != 1)
				throw EndofLineException();
			str += count;
		}

		if (color > numColors)
			throw InvalidNoteException(color);

		if (color == 0)
			m_special.init(sustain);
		else
			m_colors[color - 1].init(sustain);

		// Read modifiers
		int numMods;
		if (*str && sscanf_s(str, " %i%n", &numMods, &count) == 1)
		{
			str += count;
			char modifier;
			for (int i = 0;
				i < numMods && sscanf_s(str, " %c%n", &modifier, 1, &count) == 1;
				++i)
			{
				str += count;
				switch (modifier)
				{
				case 'F':
					m_isForced = ForceStatus::FORCED;
					break;
				case '<':
					m_isForced = ForceStatus::HOPO_ON;
					break;
				case '>':
					m_isForced = ForceStatus::HOPO_OFF;
					break;
				case 'T':
					m_isTap = true;
				}
			}
		}
	}

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
					numAdded = 1;
					static const Sustainable replacement[numColors];
					memcpy(m_colors, replacement, sizeof(Sustainable) * numColors);
				}
				else if (color <= numColors)
				{
					m_colors[color - 1].init(sustain);
					if (!m_special)
						++numAdded;
					else
						m_special = Sustainable();
				}
			}

			if (numAdded == 0)
				throw InvalidNoteException();
		}
		else
			throw EndofLineException();
	}

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

	void modify_cht(const char* str)
	{
		int numMods;
		int count;
		if (sscanf_s(str, " %i%n", &numMods, &count) == 1)
		{
			str += count;
			char modifier;
			for (int i = 0;
				i < numMods && sscanf_s(str, " %c%n", &modifier, 1, &count) == 1;
				++i)
			{
				str += count;
				switch (modifier)
				{
				case 'F':
					m_isForced = ForceStatus::FORCED;
					break;
				case '<':
					m_isForced = ForceStatus::HOPO_ON;
					break;
				case '>':
					m_isForced = ForceStatus::HOPO_OFF;
					break;
				case 'T':
					m_isTap = true;
				}
			}
		}
	}

	// write values to a V2 .chart file
	void save_cht(const uint32_t position, std::fstream& outFile) const
	{
		int numActive = Note<numColors, Sustainable, Sustainable>::write_notes_cht(position, outFile);
		int numMods = m_isForced != ForceStatus::UNFORCED ? 1 : 0;
		if (m_isTap)
			++numMods;

		if (numMods > 0)
		{
			if (numActive > 1)
				outFile << "\n\t\t" << position << " = M";
			outFile << ' ' << numMods;

			switch (m_isForced)
			{
			case ForceStatus::FORCED:
				outFile << " F";
				break;
			case ForceStatus::HOPO_ON:
				outFile << " <";
				break;
			case ForceStatus::HOPO_OFF:
				outFile << " >";
			}

			if (m_isTap)
				outFile << " T";
		}
		outFile << '\n';
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
};

template<>
void Chord<6>::init_chartV1(int lane, uint32_t sustain);
