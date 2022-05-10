#pragma once
#include "Note_cht.hpp"
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
void Chord<numColors>::init_cht_single(TextTraversal& traversal)
{
	// Read note
	uint32_t lane;
	size_t count = traversal.extractUInt(lane);
	if (!count)
		throw EndofLineException();

	traversal.move(count);
	unsigned char color = lane & 127;
	uint32_t sustain = 0;
	if (lane & 128)
	{
		if (!(count = traversal.extractUInt(sustain)))
			throw EndofLineException();
		traversal.move(count);
	}

	if (color > numColors)
		throw InvalidNoteException(color);

	if (color == 0)
		m_special.init(sustain);
	else
		m_colors[color - 1].init(sustain);

	// Read modifiers
	uint32_t numMods;
	if (count = traversal.extractUInt(numMods))
	{
		traversal.move(count);
		for (uint32_t i = 0; i < numMods;++i)
		{
			switch (traversal.getChar())
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
			traversal.move(1);
		}
	}
}

template <size_t numColors>
void Chord<numColors>::init_cht_chord(TextTraversal& traversal)
{
	uint32_t colors;
	if (size_t count = traversal.extractUInt(colors))
	{
		traversal.move(count);
		int numAdded = 0;
		uint32_t lane;
		for (uint32_t i = 0; i < colors; ++i)
		{
			if (!(count = traversal.extractUInt(lane)))
				throw EndofLineException();

			traversal.move(count);
			unsigned char color = lane & 127;
			uint32_t sustain = 0;
			if (lane & 128)
			{
				if ((count = traversal.extractUInt(sustain)) == 0)
					throw EndofLineException();
				traversal.move(count);
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
void Chord<numColors>::modify_cht(TextTraversal& traversal)
{
	uint32_t numMods;
	if (size_t count = traversal.extractUInt(numMods))
	{
		traversal.move(count);
		for (uint32_t i = 0; i < numMods; ++i)
		{
			modify(traversal.getChar(), false);
			traversal.move(1);
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

