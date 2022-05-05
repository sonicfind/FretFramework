#pragma once
#include "Chord.h"

template <size_t numColors>
void Chord<numColors>::init_chartV1(int lane, uint32_t sustain)
{
	if (lane < 5)
		m_colors[lane].init(sustain);
	else if (!checkModifiers(lane, sustain))
		throw InvalidNoteException(lane);
}

template <size_t numColors>
void Chord<numColors>::init_cht_single(const char* str)
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

template <size_t numColors>
void Chord<numColors>::init_cht_chord(const char* str)
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

template <size_t numColors>
void Chord<numColors>::modify_cht(const char* str)
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
			modify(modifier, false);
		}
	}
}

// write values to a V2 .chart file
template <size_t numColors>
void Chord<numColors>::save_cht(const uint32_t position, std::fstream& outFile) const
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

